/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __XSOCKET_H__
#define __XSOCKET_H__

#include "Common.h"
#include "MessageBuffer.h"
#include "Socket.h"
#include <chrono>
#include <functional>
#include "XRc4Cipher.h"
#include "MPSCQueue.h"
#include "XPacket.h"
#include <mutex>

class EncryptablePacket : public XPacket
{
    public:
        EncryptablePacket(XPacket const &packet, bool encrypt) : XPacket(packet), _encrypt(encrypt) {}

        bool NeedsEncryption() const { return _encrypt; }

    private:
        bool _encrypt;
};

class XSession
{
    public:
        virtual ReadDataHandlerResult ProcessIncoming(XPacket *) = 0;

        virtual bool IsEncrypted() const { return true; }

        virtual void OnClose() { }

        virtual ~XSession() = default;

};

constexpr int HEADER_SIZE = sizeof(TS_MESSAGE);

class XSocket : public Socket<XSocket>
{
        typedef Socket<XSocket> BaseSocket;

    public:
        explicit XSocket(boost::asio::ip::tcp::socket &&socket) : Socket(std::move(socket)), _session(nullptr), _sendBufferSize(4096)
        {
            _headerBuffer.Resize(HEADER_SIZE);
        }

        ~XSocket() = default;

        XSocket(XSocket const &right) = delete;
        XSocket &operator=(XSocket const &right) = delete;

        void Start() override
        {
            //AsyncReadWithCallback(&XSocket::InitializeHandler);
            AsyncRead();
        }

        bool Update() override
        {
            EncryptablePacket *queued;
            MessageBuffer     buffer(_sendBufferSize);
            while (_bufferQueue.Dequeue(queued))
            {
                auto packetSize = queued->size();
                queued->FinalizePacket();
                if (queued->NeedsEncryption())
                {
                    _encryption.Encode((char *)queued->contents(), (char *)queued->contents(), packetSize);
                }

                if (buffer.GetRemainingSpace() < packetSize)
                {
                    QueuePacket(std::move(buffer));
                    buffer.Resize(_sendBufferSize);
                }

                if (buffer.GetRemainingSpace() >= packetSize + HEADER_SIZE)
                    WritePacketToBuffer(*queued, buffer);
                else    // single packet larger than 4096 bytes
                {
                    MessageBuffer packetBuffer(packetSize + HEADER_SIZE);
                    WritePacketToBuffer(*queued, packetBuffer);
                    QueuePacket(std::move(packetBuffer));
                }

                delete queued;
            }

            if (buffer.GetActiveSize() > 0)
                QueuePacket(std::move(buffer));

            return BaseSocket::Update();

        }

        void SendPacket(XPacket const &packet)
        {
            if (!IsOpen())
                return;

            //if (sPacketLog->CanLogPacket())
            //sPacketLog->LogPacket(packet, SERVER_TO_CLIENT, GetRemoteIpAddress(), GetRemotePort(), GetConnectionType());

            _bufferQueue.Enqueue(new EncryptablePacket(packet, _session->IsEncrypted()));
        }

        void SetSession(XSession *session)
        {
            std::lock_guard<std::mutex> sessionGuard(_sessionLock);
            _session = session;
            if (_session->IsEncrypted())
            {
                _encryption.SetKey("}h79q~B%al;k'y $E");
                _decryption.SetKey("}h79q~B%al;k'y $E");
            }
            SetSendBufferSize(65536);
        }

        void SetSendBufferSize(std::size_t sendBufferSize) { _sendBufferSize = sendBufferSize; }
        XSession* GetSession() { return _session; }

    protected:
        void OnClose() override
        {
            {
                if(_session)
                    _session->OnClose();
                std::lock_guard<std::mutex> sessionGuard(_sessionLock);
                _session = nullptr;
            }
        }

        void ReadHandler() override
        {
            if (!IsOpen())
                return;

            MessageBuffer &packet = GetReadBuffer();
            while (packet.GetActiveSize() > 0)
            {
                if (_headerBuffer.GetRemainingSpace() > 0)
                {
                    // need to receive the header
                    std::size_t readHeaderSize = std::min(packet.GetActiveSize(), _headerBuffer.GetRemainingSpace());
                    _headerBuffer.Write(packet.GetReadPointer(), readHeaderSize);
                    packet.ReadCompleted(readHeaderSize);

                    if (_headerBuffer.GetRemainingSpace() > 0)
                    {
                        // Couldn't receive the whole header this time.
                                ASSERT(packet.GetActiveSize() == 0);
                        break;
                    }

                    // We just received nice new header
                    if (!ReadHeaderHandler())
                    {
                        CloseSocket();
                        return;
                    }
                }

                // We have full read header, now check the data payload
                if (_packetBuffer.GetRemainingSpace() > 0)
                {
                    // need more data in the payload
                    std::size_t readDataSize = std::min(packet.GetActiveSize(), _packetBuffer.GetRemainingSpace());
                    _packetBuffer.Write(packet.GetReadPointer(), readDataSize);
                    packet.ReadCompleted(readDataSize);

                    if (_packetBuffer.GetRemainingSpace() > 0)
                    {
                        // Couldn't receive the whole data this time.
                                ASSERT(packet.GetActiveSize() == 0);
                        break;
                    }
                }

                // just received fresh new payload
                ReadDataHandlerResult result = ReadDataHandler();
                _headerBuffer.Reset();
                if (result != ReadDataHandlerResult::Ok)
                {
                    if (result != ReadDataHandlerResult::WaitingForQuery)
                        CloseSocket();

                    return;
                }
            }

            AsyncRead();
        }

        bool ReadHeaderHandler()
        {
            if (_session->IsEncrypted())
            {
                _decryption.Decode((char *)_headerBuffer.GetReadPointer(), (char *)_headerBuffer.GetReadPointer(), sizeof(TS_MESSAGE));
            }
            auto header = reinterpret_cast<TS_MESSAGE *>(_headerBuffer.GetReadPointer());

            if (header->size > 4098)
            {
                NG_LOG_ERROR("network", "XSocket::ReadHeaderHandler(): client %s sent malformed packet (size: %u, cmd: %u)",
                             GetRemoteIpAddress().to_string().c_str(), header->size, header->id);
                return false;
            }

            _packetBuffer.Resize(header->size - HEADER_SIZE);
            return true;
        }

        ReadDataHandlerResult ReadDataHandler()
        {
            auto header = reinterpret_cast<TS_MESSAGE *>(_headerBuffer.GetReadPointer());
            if (_session->IsEncrypted())
            {
                _decryption.Decode((char *)_packetBuffer.GetReadPointer(), (char *)_packetBuffer.GetReadPointer(), _packetBuffer.GetBufferSize());
            }
            XPacket packet(header->id, std::move(_packetBuffer));

            std::unique_lock<std::mutex> sessionGuard(_sessionLock, std::defer_lock);

            return _session->ProcessIncoming(&packet);
        }

    private:
        void WritePacketToBuffer(EncryptablePacket const &packet, MessageBuffer &buffer)
        {
            // Reserve space for buffer
            if (packet.NeedsEncryption() && !packet.empty())
            {
                buffer.Write(packet.contents(), packet.size());
            }
            else if (!packet.empty())
                buffer.Write(packet.contents(), packet.size());
        }

        std::mutex _sessionLock;
        XSession   *_session;
        XRC4Cipher _encryption, _decryption;

        MPSCQueue<EncryptablePacket> _bufferQueue;
        MessageBuffer                _headerBuffer;
        MessageBuffer                _packetBuffer;
        std::size_t                  _sendBufferSize;

};

#endif // __XSOCKET_H__
