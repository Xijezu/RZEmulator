/*
  *  Copyright (C) 2017-2017 Xijezu <http://xijezu.com/>
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

#include "GameList.h"
#include "PlayerList.h"
#include "XPacket.h"

#include "AuthGame/AuthGameSession.h"

// Constructor - set the default server name to <null>, also give it a socket
AuthGameSession::AuthGameSession(XSocket *pSocket) : m_pSocket(pSocket), m_pGame(new Game{ }), m_bIsAuthed(false)
{
    m_pGame->nIDX   = 255;
    m_pGame->szName = "<null>";
}

AuthGameSession::~AuthGameSession()
{
    if (m_pGame)
    {
        if (m_bIsAuthed)
        {
            auto g = sGameMapList.GetGame(m_pGame->nIDX);
            if (g != nullptr)
            {
                sGameMapList.RemoveGame(g->nIDX);

            }
        }
        delete m_pGame;
        m_pGame = nullptr;
    }
    m_pSocket->DeleteSession();
}

void AuthGameSession::OnClose()
{
    if (m_pGame == nullptr)
        return;
    auto g = sGameMapList.GetGame(m_pGame->nIDX);
    if (g != nullptr && g->szName == m_pGame->szName)
    {
        {
            NG_UNIQUE_GUARD writeGuard(*sPlayerMapList.GetGuard());
            auto            map = sPlayerMapList.GetMap();
            for (auto       &player : *map)
            {
                if (player.second->nGameIDX == g->nIDX)
                {
                    map->erase(player.second->szLoginName);
                    delete player.second;
                }
            }
        }
        sGameMapList.RemoveGame(g->nIDX);
        NG_LOG_INFO("gameserver", "Gameserver <%s> [Idx: %d] has disconnected.", m_pGame->szName.c_str(), m_pGame->nIDX);
    }

}

enum eStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct GameHandler
{
    int                                               cmd;
    eStatus                                           status;
    std::function<void(AuthGameSession *, XPacket *)> handler;
} GameHandler;

template<typename T>
GameHandler declareHandler(eStatus status, void (AuthGameSession::*handler)(const T *packet))
{
    GameHandler handlerData{ };
    handlerData.cmd     = T::getId(EPIC_4_1_1);
    handlerData.status  = status;
    handlerData.handler = [handler](AuthGameSession *instance, XPacket *packet) -> void {
        T                       deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };

    return handlerData;
}

const GameHandler packetHandler[] =
                              {
                                      declareHandler(STATUS_CONNECTED, &AuthGameSession::HandleGameLogin),
                                      declareHandler(STATUS_AUTHED, &AuthGameSession::HandleClientLogin),
                                      declareHandler(STATUS_AUTHED, &AuthGameSession::HandleClientLogout),
                                      declareHandler(STATUS_AUTHED, &AuthGameSession::HandleClientKickFailed),
                                      declareHandler(STATUS_CONNECTED, &AuthGameSession::HandlePingPacket)
                              };

constexpr int tableSize = (sizeof(packetHandler) / sizeof(GameHandler));

// Handler for incoming packets
ReadDataHandlerResult AuthGameSession::ProcessIncoming(XPacket *pGamePct)
{
            ASSERT(pGamePct);

    auto _cmd = pGamePct->GetPacketID();
    int  i    = 0;

    for (i = 0; i < tableSize; i++)
    {
        if ((uint16_t)packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (m_bIsAuthed && packetHandler[i].status == STATUS_AUTHED)))
        {
            packetHandler[i].handler(this, pGamePct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == tableSize)
    {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pGamePct->GetPacketID(), m_pSocket->GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Error;
    }
    return ReadDataHandlerResult::Ok;
}

void AuthGameSession::HandleGameLogin(const TS_GA_LOGIN *pGamePct)
{
    m_pGame->nIDX           = pGamePct->server_idx;
    m_pGame->szName         = pGamePct->server_name;
    m_pGame->szSSU          = pGamePct->server_screenshot_url;
    m_pGame->bIsAdultServer = pGamePct->is_adult_server != 0;
    m_pGame->szIP           = pGamePct->server_ip;
    m_pGame->nPort          = pGamePct->server_port;
    m_pGame->m_pSession     = this;

    auto               pGame = sGameMapList.GetGame(m_pGame->nIDX);
    TS_AG_LOGIN_RESULT resultPct;

    if (pGame == nullptr)
    {
        m_bIsAuthed = true;
        sGameMapList.AddGame(m_pGame);
        NG_LOG_INFO("server.authserver", "Gameserver <%s> [Idx: %d] at %s:%d registered.", m_pGame->szName.c_str(), m_pGame->nIDX, m_pGame->szIP.c_str(), m_pGame->nPort);
        resultPct.result = TS_RESULT_SUCCESS;
        m_pSocket->SendPacket(resultPct);
    }
    else
    {
        m_bIsAuthed = false;
        NG_LOG_INFO("server.authserver", "Gameserver <%s> [Idx: %d] at %s:%d already in list!", m_pGame->szName.c_str(), m_pGame->nIDX, m_pGame->szIP.c_str(), m_pGame->nPort);
        resultPct.result = TS_RESULT_ACCESS_DENIED;
        m_pSocket->SendPacket(resultPct);
        m_pSocket->CloseSocket();
    }
}

void AuthGameSession::HandleClientLogin(const TS_GA_CLIENT_LOGIN *pGamePct)
{
    auto   p      = sPlayerMapList.GetPlayer(pGamePct->account);
    uint16 result = TS_RESULT_ACCESS_DENIED;

    if (p != nullptr)
    {
        if (pGamePct->one_time_key == p->nOneTimeKey)
        {
            p->bIsInGame = true;
            result = TS_RESULT_SUCCESS;
        }
        else
        {
            NG_LOG_ERROR("network", "AuthGameSession::HandleClientLogin: Client [%d:%s] tried to login with wrong key!!!", p->nAccountID, p->szLoginName.c_str());
        }
    }

    TS_AG_CLIENT_LOGIN resultPct{ };
    resultPct.account    = (p != nullptr ? p->szLoginName : "");
    resultPct.nAccountID = (p != nullptr ? p->nAccountID : 0);
    resultPct.result     = result;
    resultPct.permission = (p != nullptr ? p->nPermission : 0);
    /*
    resultPct << (uint8)0;  // PC Bang Mode
    resultPct << (uint32)0; // Age
    resultPct << (uint32)0; // Event Code
    resultPct << (uint32)0; // Continuous Playtime
    resultPct << (uint32)0; // Continuous Logouttime
    */

    m_pSocket->SendPacket(resultPct);
}

void AuthGameSession::HandleClientLogout(const TS_GA_CLIENT_LOGOUT *pGamePct)
{
    auto p = sPlayerMapList.GetPlayer(pGamePct->account);
    if (p != nullptr)
    {
        sPlayerMapList.RemovePlayer(pGamePct->account);
        delete p;
    }
}

void AuthGameSession::HandleClientKickFailed(const TS_GA_CLIENT_KICK_FAILED *pGamePct)
{
    auto p = sPlayerMapList.GetPlayer(pGamePct->account);
    if (p != nullptr)
    {
        sPlayerMapList.RemovePlayer(pGamePct->account);
        delete p;
    }
}

void AuthGameSession::KickPlayer(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    TS_AG_KICK_CLIENT kickPct{ };
    kickPct.account = pPlayer->szLoginName;
    m_pSocket->SendPacket(kickPct);
}

void AuthGameSession::HandlePingPacket(const TS_CS_PING *)
{
    TS_CS_PING pingPct{ };
    m_pSocket->SendPacket(pingPct);
}
