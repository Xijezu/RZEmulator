#pragma once
/*
 * Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include <chrono>
#include <functional>
#include <mutex>

#include "Common.h"
#include "MPSCQueue.h"
#include "MessageBuffer.h"
#include "Packets/MessageSerializerBuffer.h"
#include "Socket.h"
#include "XPacket.h"
#include "XRc4Cipher.h"

class EncryptablePacket : public XPacket {
public:
    EncryptablePacket(XPacket const &packet, bool encrypt)
        : XPacket(packet)
        , _encrypt(encrypt)
    {
    }
    bool NeedsEncryption() const { return _encrypt; }

private:
    bool _encrypt;
};

constexpr int HEADER_SIZE = sizeof(TS_MESSAGE);
class XSocket : public Socket<XSocket> {
    typedef Socket<XSocket> BaseSocket;

public:
    explicit XSocket(boost::asio::ip::tcp::socket &&socket);
    XSocket(XSocket const &right) = delete;
    XSocket &operator=(XSocket const &right) = delete;

    // Overrides
    virtual ReadDataHandlerResult ProcessIncoming(XPacket *) { return ReadDataHandlerResult::Error; };
    virtual bool IsEncrypted() const { return true; }
    virtual ~XSocket() = default;

    void Start() override;
    bool Update() override;

    void SendPacket(XPacket const &packet);

    template<class TS_SERIALIZABLE_PACKET>
    void SendPacket(TS_SERIALIZABLE_PACKET const &packet)
    {
        if (!IsOpen())
            return;

        XPacket output;
        MessageSerializerBuffer serializer(&output);
        packet.serialize(&serializer);
        SendPacket(*serializer.getFinalizedPacket());
    }

    void SetSendBufferSize(std::size_t sendBufferSize);

protected:
    void ReadHandler() override;
    bool ReadHeaderHandler();
    ReadDataHandlerResult ReadDataHandler();

    virtual void InitSocket() {}

private:
    void WritePacketToBuffer(EncryptablePacket const &packet, MessageBuffer &buffer);

    XRC4Cipher _encryption, _decryption;

    MPSCQueue<EncryptablePacket> _bufferQueue;
    MessageBuffer _headerBuffer;
    MessageBuffer _packetBuffer;
    std::size_t _sendBufferSize;
};
