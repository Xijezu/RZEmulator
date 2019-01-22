#include "XSocket.h"

XSocket::XSocket(boost::asio::ip::tcp::socket &&socket) : Socket(std::move(socket)), _sendBufferSize(4096)
{
    _headerBuffer.Resize(HEADER_SIZE);
}

void XSocket::Start()
{
    if (IsEncrypted())
    {
        _encryption.SetKey("}h79q~B%al;k'y $E");
        _decryption.SetKey("}h79q~B%al;k'y $E");
    }
    SetSendBufferSize(65536);
    //AsyncReadWithCallback(&XSocket::InitializeHandler);
    AsyncRead();
}

bool XSocket::Update()
{
    EncryptablePacket *queued;
    MessageBuffer buffer(_sendBufferSize);
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
        else // single packet larger than 4096 bytes
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

void XSocket::SendPacket(XPacket const &packet)
{
    if (!IsOpen())
        return;

    //if (sPacketLog->CanLogPacket())
    //sPacketLog->LogPacket(packet, SERVER_TO_CLIENT, GetRemoteIpAddress(), GetRemotePort(), GetConnectionType());

    _bufferQueue.Enqueue(new EncryptablePacket(packet, IsEncrypted()));
}

void XSocket::SetSendBufferSize(std::size_t sendBufferSize)
{
    _sendBufferSize = sendBufferSize;
}

void XSocket::ReadHandler()
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
            //if (result != ReadDataHandlerResult::WaitingForQuery)
            //    CloseSocket();

            return;
        }
    }

    AsyncRead();
}

bool XSocket::ReadHeaderHandler()
{
    if (IsEncrypted())
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

ReadDataHandlerResult XSocket::ReadDataHandler()
{
    auto header = reinterpret_cast<TS_MESSAGE *>(_headerBuffer.GetReadPointer());
    if (IsEncrypted())
    {
        _decryption.Decode((char *)_packetBuffer.GetReadPointer(), (char *)_packetBuffer.GetReadPointer(), _packetBuffer.GetBufferSize());
    }

    XPacket packet(header->id, std::move(_packetBuffer));
    return ProcessIncoming(&packet);
}

void XSocket::WritePacketToBuffer(EncryptablePacket const &packet, MessageBuffer &buffer)
{
    // Reserve space for buffer
    if (packet.NeedsEncryption() && !packet.empty())
    {
        buffer.Write(packet.contents(), packet.size());
    }
    else if (!packet.empty())
        buffer.Write(packet.contents(), packet.size());
}