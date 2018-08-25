/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "DatabaseEnv.h"
#include "AuthClientSession.h"
#include "PlayerList.h"
#include "GameList.h"
#include "XPacket.h"
#include "Encryption/MD5.h"
#include "AuthGameSession.h"
#include "Util.h"

// Constructo - give it a socket
AuthClientSession::AuthClientSession(XSocket *socket) : _socket(socket), m_pPlayer(nullptr)
{
    _desCipther.Init("MERONG");
}

// Close patch file descriptor before leaving
AuthClientSession::~AuthClientSession()
{
}

void AuthClientSession::OnClose()
{
    if (m_pPlayer == nullptr)
        return;
    auto g = sPlayerMapList.GetPlayer(m_pPlayer->szLoginName);
    if (g != nullptr && g->nAccountID == m_pPlayer->nAccountID && !g->bIsInGame)
    {
        sPlayerMapList.RemovePlayer(g->szLoginName);
        delete m_pPlayer;
    }
    _socket->DeleteSession();
}

enum eStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct AuthHandler
{
    int                                                 cmd;
    eStatus                                             status;
    std::function<void(AuthClientSession *, XPacket *)> handler;
} AuthHandler;

template<typename T>
AuthHandler declareHandler(eStatus status, void (AuthClientSession::*handler)(const T *packet))
{
    AuthHandler handlerData{ };
    handlerData.cmd     = T::getId(EPIC_4_1_1);
    handlerData.status  = status;
    handlerData.handler = [handler](AuthClientSession *instance, XPacket *packet) -> void {
        T                       deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };

    return handlerData;
}

const AuthHandler packetHandler[] = {
        declareHandler(STATUS_CONNECTED, &AuthClientSession::HandleVersion),
        declareHandler(STATUS_CONNECTED, &AuthClientSession::HandleLoginPacket),
        declareHandler(STATUS_AUTHED, &AuthClientSession::HandleServerList),
        declareHandler(STATUS_AUTHED, &AuthClientSession::HandleSelectServer),
};

constexpr int tableSize = sizeof(packetHandler) / sizeof(AuthHandler);

/// Handler for incoming packets
ReadDataHandlerResult AuthClientSession::ProcessIncoming(XPacket *pRecvPct)
{
            ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();

    int i = 0;
    for (i = 0; i < tableSize; i++)
    {
        if ((uint16_t)packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (_isAuthed && packetHandler[i].status == STATUS_AUTHED)))
        {
            //(*this.*packetHandler[i].handler)(pRecvPct);
            packetHandler[i].handler(this, pRecvPct);
            break;
        }
    }
    // Report unknown packets in the error log
    if (i == tableSize)
    {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), _socket->GetRemoteIpAddress().to_v4().to_string().c_str());
        return ReadDataHandlerResult::Error;
    }
    return ReadDataHandlerResult::Ok;
}

void AuthClientSession::HandleLoginPacket(const TS_CA_ACCOUNT *pRecvPct)
{
    std::string szPassword((char *)(pRecvPct->passwordDes.password));
    _desCipther.Decrypt(&szPassword[0], (int)szPassword.length());
    szPassword.erase(std::remove(szPassword.begin(), szPassword.end(), '\0'), szPassword.end());
    szPassword.insert(0, "2011"); // @todo: md5 key
    szPassword = md5(szPassword);

    // SQL part
    PreparedStatement *stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT);
    stmt->setString(0, pRecvPct->account);
    stmt->setString(1, szPassword);
    if (PreparedQueryResult dbResult = LoginDatabase.Query(stmt))
    {
        m_pPlayer = new Player{ };
        m_pPlayer->nAccountID     = (*dbResult)[0].GetUInt32();
        m_pPlayer->szLoginName    = (*dbResult)[1].GetString();
        m_pPlayer->nLastServerIDX = (*dbResult)[2].GetUInt32();
        m_pPlayer->bIsBlocked     = (*dbResult)[3].GetBool();
        m_pPlayer->nPermission    = (*dbResult)[4].GetInt32();
        m_pPlayer->bIsInGame      = false;

        std::transform(m_pPlayer->szLoginName.begin(), m_pPlayer->szLoginName.end(), m_pPlayer->szLoginName.begin(), ::tolower);

        if (m_pPlayer->bIsBlocked)
        {
            SendResultMsg(pRecvPct->id, TS_RESULT_ACCESS_DENIED, 0);
            return;
        }

        auto pOldPlayer = sPlayerMapList.GetPlayer(m_pPlayer->szLoginName);
        if (pOldPlayer != nullptr)
        {
            if (pOldPlayer->bIsInGame)
            {
                auto game = sGameMapList.GetGame((uint)pOldPlayer->nGameIDX);
                if (game != nullptr && game->m_pSession != nullptr)
                    game->m_pSession->KickPlayer(pOldPlayer);
            }
            SendResultMsg(pRecvPct->id, TS_RESULT_ALREADY_EXIST, 0);
            sPlayerMapList.RemovePlayer(pOldPlayer->szLoginName);
            delete pOldPlayer;
        }

        _isAuthed = true;
        sPlayerMapList.AddPlayer(m_pPlayer);
        SendResultMsg(pRecvPct->id, TS_RESULT_SUCCESS, 1);
        return;
    }
    SendResultMsg(pRecvPct->id, TS_RESULT_NOT_EXIST, 0);
}

void AuthClientSession::HandleVersion(const TS_CA_VERSION *pRecvPct)
{
    NG_LOG_TRACE("network", "[Version] Client version is %s", pRecvPct->szVersion.c_str());
}

void AuthClientSession::HandleServerList(const TS_CA_SERVER_LIST *pRecvPct)
{
    NG_SHARED_GUARD readGuard(*sGameMapList.GetGuard());
    auto            map = sGameMapList.GetMap();
    XPacket         packet(NGemity::Packets::TS_AC_SERVER_LIST);
    packet << (uint16)0;
    packet << (uint16)map->size();
    for (auto &x : *map)
    {
        packet << (uint16)x.second->nIDX;
        packet.fill(x.second->szName, 21);
        packet << (uint8)(x.second->bIsAdultServer ? 1 : 0);
        packet.fill(x.second->szSSU, 256);
        packet.fill(x.second->szIP, 16);
        packet << (int32)x.second->nPort;
        packet << (uint16)0;
    }
    _socket->SendPacket(packet);
}

void AuthClientSession::HandleSelectServer(const TS_CA_SELECT_SERVER *pRecvPct)
{
    m_pPlayer->nGameIDX    = pRecvPct->server_idx;
    m_pPlayer->nOneTimeKey = ((uint64)rand32()) * rand32() * rand32() * rand32();
    m_pPlayer->bIsInGame   = true;
    bool    bExist = sGameMapList.GetGame((uint)m_pPlayer->nGameIDX) != 0;

    TS_AC_SELECT_SERVER resultPct{ };
    resultPct.result       = (bExist ? TS_RESULT_SUCCESS : TS_RESULT_NOT_EXIST);
    resultPct.one_time_key = (bExist ? m_pPlayer->nOneTimeKey : 0);
    resultPct.pending_time = 0;
    _socket->SendPacket(resultPct);
}

void AuthClientSession::SendResultMsg(uint16 pctID, uint16 result, uint value)
{
    TS_AC_RESULT resultMsg{ };
    resultMsg.request_msg_id = pctID;
    resultMsg.result         = result;
    resultMsg.login_flag     = value;
    _socket->SendPacket(resultMsg);
}
