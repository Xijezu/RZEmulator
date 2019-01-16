/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "LunaSession.h"
#include "AES.h"

template <class TS_SERIALIZABLE_PACKET>
void SendPacket(TS_SERIALIZABLE_PACKET const &packet, XSocket *Socket)
{
    XPacket output;
    MessageSerializerBuffer serializer(&output);
    packet.serialize(&serializer);
    Socket->SendPacket(*serializer.getFinalizedPacket());
}

LunaSession::LunaSession(XSocket *socket) : m_pSocket(socket)
{
    m_pCipher = std::make_unique<RsaCipher>();
}

LunaSession::~LunaSession()
{
}

enum eStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct
{
    int cmd;
    eStatus status;
    std::function<void(LunaSession *, XPacket *)> handler;
} LunaHandler;

template <typename T>
LunaHandler declareHandler(eStatus status, void (LunaSession::*handler)(const T *packet))
{
    LunaHandler handlerData{};
    handlerData.handler = [handler](LunaSession *instance, XPacket *packet) -> void {
        T deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };
    return handlerData;
}

const LunaHandler LunaPacketHandler[] =
    {
        {declareHandler(STATUS_CONNECTED, &LunaSession::onResultHandler)},
        {declareHandler(STATUS_CONNECTED, &LunaSession::onPacketServerList)},
        {declareHandler(STATUS_CONNECTED, &LunaSession::onAuthResult)},
        {declareHandler(STATUS_CONNECTED, &LunaSession::onRsaKey)}};

constexpr int LunaTableSize = (sizeof(LunaPacketHandler) / sizeof(LunaHandler));

ReadDataHandlerResult LunaSession::ProcessIncoming(XPacket *pRecvPct)
{
    ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int i = 0;

    for (i = 0; i < LunaTableSize; i++)
    {
        if ((uint16_t)LunaPacketHandler[i].cmd == _cmd)
        {
            LunaPacketHandler[i].handler(this, pRecvPct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == LunaTableSize)
    {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), m_pSocket->GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Ok;
    }
    return ReadDataHandlerResult::Ok;
}

int LunaSession::GetAccountId() const
{
    return 0;
}

std::string LunaSession::GetAccountName()
{
    return "";
}

void LunaSession::OnClose()
{
    NG_LOG_ERROR("network", "Authserver has closed connection!");
}

void LunaSession::onResultHandler(const TS_SC_RESULT *resultPct)
{
    NG_LOG_INFO("network", "Received result:");
    NG_LOG_INFO("network", "Result: %d", resultPct->result);
    NG_LOG_INFO("network", "Value: %d", (resultPct->value ^ 0xADADADAD));
}

void LunaSession::onRsaKey(const TS_AC_AES_KEY_IV *pRecv)
{
    TS_CA_ACCOUNT accountMsg{};
    std::vector<uint8_t> encryptedPassword{};

    if (!m_pCipher->privateDecrypt(pRecv->data.data(), pRecv->data.size(), aesKey) || aesKey.size() != 32)
    {
        NG_LOG_INFO("network", "onPacketAuthPasswordKey: invalid decrypted data size: %d", static_cast<int32_t>(aesKey.size()));
        m_pSocket->CloseSocket();
        return;
    }

    AesPasswordCipher cipher{};
    cipher.init(aesKey.data());

    if (!cipher.encrypt((const uint8_t *)m_szPassword.c_str(), m_szPassword.size(), encryptedPassword))
    {
        NG_LOG_WARN("network", "onPacketAuthPasswordKey: could not encrypt password !");
        m_pSocket->CloseSocket();
        return;
    }

    memcpy(accountMsg.passwordAes.password, encryptedPassword.data(), encryptedPassword.size());
    m_szPassword.clear();

    accountMsg.account = m_szUsername;
    accountMsg.passwordAes.password_size = encryptedPassword.size();

    SendPacket(accountMsg, m_pSocket);
}

void LunaSession::onPacketServerList(const TS_AC_SERVER_LIST *pRecv)
{
    for (const auto &server : pRecv->servers)
    {
        NG_LOG_INFO("network", "Name: %s - IP: %s, Port: %d", server.server_name.c_str(), server.server_ip.c_str(), server.server_port);
    }
    m_pSocket->CloseSocket();
}

void LunaSession::onAuthResult(const TS_AC_RESULT *pRecv)
{
    NG_LOG_INFO("network", "Received Auth Result: %s", pRecv->result == 0 ? "Success" : "Error");
    if (pRecv->result == 0)
    {
        TS_CA_SERVER_LIST list{};
        SendPacket(list, m_pSocket);
    }
    else
    {
        m_pSocket->CloseSocket();
    }
}

void LunaSession::InitConnection(const std::string &szUsername, const std::string &szPassword)
{
    m_szUsername = szUsername;
    m_szPassword = szPassword;

    TS_CA_VERSION versionPkt{};
    versionPkt.szVersion = "200701120";
    SendPacket(versionPkt, m_pSocket);

    if (!m_pCipher->isInitialized())
    {
        if (!m_pCipher->generateKey())
            NG_LOG_ERROR("network", "Failed to generate RSA Key.");
    }

    if (m_pCipher->isInitialized())
    {
        TS_CA_RSA_PUBLIC_KEY keyMsg{};

        m_pCipher->getPemPublicKey(keyMsg.key);

        SendPacket(keyMsg, m_pSocket);
    }
    else
    {
        NG_LOG_ERROR("network", "No RSA key to send, aborting...");
        m_pSocket->CloseSocket();
    }
}