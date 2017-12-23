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

#include "Common.h"

#include "AuthGame/GameAuthPackets.h"
#include "AuthGame/AuthGamePackets.h"
#include "AuthGame/GameHandler.h"
#include "Lists/PlayerList.h"
#include <ace/Auto_Ptr.h>

// Constructor - set the default server name to <null>, also give it a socket
GameHandler::GameHandler(XSocket &socket) : _socket(socket)
{
    _game.server_name = "<null>";
}

// Accept the connection - function itself not used here because we're only interested in the game server data itself
void GameHandler::OnAccept()
{
    MX_LOG_TRACE("network", "'%s:%d' Accepting connection\n", _socket.getRemoteAddress().c_str(), _socket.getRemotePort());
}

void GameHandler::OnClose()
{
    if (_game.server_name != "<null>") {
        sGameMapList->RemoveGame(_game.server_idx);
        MX_LOG_INFO("server.authserver", "Gameserver <%s> [Idx: %d] disconnected.", _game.server_name.c_str(), _game.server_idx);
    }
}

enum eStatus {
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct AuthHandler {
    uint16_t cmd;
    uint8_t status;

    bool (GameHandler::*handler)(XPacket *);
} AuthHandler;


const AuthHandler packetHandler[] =
        {
                {TS_GA_LOGIN,              STATUS_CONNECTED, &GameHandler::HandleGameLogin},
                {TS_GA_CLIENT_LOGIN,       STATUS_AUTHED,    &GameHandler::HandleClientLogin},
                {TS_GA_CLIENT_LOGOUT,      STATUS_AUTHED,    &GameHandler::HandleClientLogout},
                {TS_GA_CLIENT_KICK_FAILED, STATUS_AUTHED,    &GameHandler::HandleClientKickFailed}
        };

const int tableSize = (sizeof(packetHandler) / sizeof(AuthHandler));

// Handler for incoming packets
void GameHandler::ProcessIncoming(XPacket *pGamePct)
{
    ACE_ASSERT(pGamePct);

    // Manage memory
    ACE_Auto_Ptr<XPacket> aptr(pGamePct);

    auto _cmd = pGamePct->GetPacketID();
    int i = 0;

    for (i = 0; i < tableSize; i++) {
        if ((uint16_t) packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (_isAuthed && packetHandler[i].status == STATUS_AUTHED))) {
            MX_LOG_DEBUG("network", "Got data for id %u recv length %u", (uint32) _cmd, (uint32) pGamePct->size());

            if (!(*this.*packetHandler[i].handler)(pGamePct)) {
                MX_LOG_DEBUG("network", "Packet handler failed for id %u recv length %u", (uint32) _cmd, (uint32) pGamePct->size());
                return;
            }
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == tableSize) {
        MX_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pGamePct->GetPacketID(), _socket.getRemoteAddress().c_str());
        return;
    }
    aptr.release();
}

bool GameHandler::HandleGameLogin(XPacket *pGamePct)
{
    GA_LOGIN *loginPct = ((GA_LOGIN *) pGamePct->contents());

    _game.is_adult_server = loginPct->is_adult_server == 1;
    _game.server_idx = loginPct->server_idx;
    _game.server_ip = std::string(loginPct->server_ip);
    _game.server_name = std::string(loginPct->server_name);
    _game.server_port = loginPct->server_port;
    _game.server_screenshot_url = std::string(loginPct->server_screenshot_url);

    AG_LOGIN_RESULT resultPct = {};
    resultPct.id = 20002;
    resultPct.size = sizeof(AG_LOGIN_RESULT);
    resultPct.result = TS_RESULT_UNKNOWN;

    if (!sGameMapList->contains(_game.server_idx)) {
        resultPct.result = TS_RESULT_SUCCESS;
        sGameMapList->AddGame(_game);
        _isAuthed = true;
        MX_LOG_INFO("server.authserver", "Gameserver <%s> [Idx: %d] at %s:%d registered.", _game.server_name.c_str(), _game.server_idx, _game.server_ip.c_str(), _game.server_port);
    } else {
        resultPct.result = TS_RESULT_ALREADY_EXIST;
        MX_LOG_WARN("server.authserver", "Gameserver %s [Idx: %d] at %s:%d already in list!", _game.server_name.c_str(), _game.server_idx, _game.server_ip.c_str(), _game.server_port);
    }

    _socket.SendPacket(resultPct, sizeof(resultPct));
    return true;
}

bool GameHandler::HandleClientLogin(XPacket *pGamePct)
{
    GA_CLIENT_LOGIN *loginPct = ((GA_CLIENT_LOGIN *) pGamePct->contents());

    AG_CLIENT_LOGIN resultPct = {};
    resultPct.result = 0x6;                                        // Temporary ACCESS_DENIED

    if (sPlayerMapList->contains(loginPct->account)) {
        Player p = sPlayerMapList->GetMap()[loginPct->account];

        strcpy(resultPct.account, p.login_name.c_str());            // Login name
        resultPct.nAccountID = p.account_id;                        // Account ID

        if (loginPct->one_time_key == p.one_time_key) {
            resultPct.result = 0;                                    // TS_RESULT_SUCCESS
        } else {
            resultPct.result = 0x6;                                // TS_RESULT_ACCESS_DENIED
            MX_LOG_WARN("network","Account %s tried to login with a wrong one time key. Expected %d but got %d", p.login_name, p.one_time_key, loginPct->one_time_key);
            sPlayerMapList->RemovePlayer(loginPct->account);
        }
    }
    resultPct.nPCBangUser = 0;                                        // PCBangUser
    resultPct.nEventCode = 0;                                        // EventCode
    resultPct.nAge = 0;                                            // Age
    resultPct.nContinuousPlayTime = 0;                                // Continuous Playtime
    resultPct.nContinuousLogoutTime = 0;                            // Continuous Logout time

    XPacket resPack(AGPACKETS::TS_AG_CLIENT_LOGIN);
    resPack.fill(resultPct.account, 61);
    resPack << (uint32_t) resultPct.nAccountID;
    resPack << (uint16_t) resultPct.result;
    resPack << resultPct.nPCBangUser;
    resPack << (uint32_t) resultPct.nEventCode;
    resPack << (uint32_t) resultPct.nAge;
    resPack << (uint32_t) resultPct.nContinuousPlayTime;
    resPack << (uint32_t) resultPct.nContinuousLogoutTime;

    _socket.SendPacket(resPack);
    return true;
}

bool GameHandler::HandleClientLogout(XPacket *pGamePct)
{
    GA_CLIENT_LOGOUT *packet = ((GA_CLIENT_LOGOUT *) pGamePct->contents());
    return true;
}

bool GameHandler::HandleClientKickFailed(XPacket *pGamePct)
{
    GA_CLIENT_KICK_FAILED *packet = ((GA_CLIENT_KICK_FAILED *) pGamePct->contents());
    return true;
}