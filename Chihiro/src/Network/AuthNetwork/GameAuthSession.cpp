/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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

#include "GameAuthSession.h"

#include "AuthNetwork.h"
#include "Common.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

GameAuthSession::GameAuthSession(boost::asio::ip::tcp::socket &&socket)
    : XSocket(std::move(socket))
{
    m_nGameIDX = (uint16_t)sConfigMgr->GetIntDefault("GameServer.Index", 1);
    m_szGameName = sConfigMgr->GetStringDefault("GameServer.Name", "Testserver");
    m_szGameSSU = sConfigMgr->GetStringDefault("GameServer.SSU", "about:blank");
    m_bGameIsAdultServer = sConfigMgr->GetIntDefault("GameServer.AdultServer", 0) != 0;
    m_szGameIP = sConfigMgr->GetStringDefault("GameServer.IP", "127.0.0.1");
    m_nGamePort = sConfigMgr->GetIntDefault("GameServer.Port", 4514);
}

GameAuthSession::~GameAuthSession() {}

typedef struct AuthHandler {
    NGemity::Packets cmd;
    void (GameAuthSession::*handler)(XPacket *);
} AuthHandler;

constexpr AuthHandler authPacketHandler[] = {{NGemity::Packets::TS_AG_LOGIN_RESULT, &GameAuthSession::HandleGameLoginResult}, {NGemity::Packets::TS_AG_KICK_CLIENT, &GameAuthSession::HandleClientKick},
    {NGemity::Packets::TS_AG_CLIENT_LOGIN, &GameAuthSession::HandleClientLoginResult}, {NGemity::Packets::TS_CS_PING, &GameAuthSession::HandleNullPacket}};

constexpr int32_t authTableSize = (sizeof(authPacketHandler) / sizeof(AuthHandler));

ReadDataHandlerResult GameAuthSession::ProcessIncoming(XPacket *pGamePct)
{
    ASSERT(pGamePct);

    auto _cmd = pGamePct->GetPacketID();
    int32_t i = 0;

    for (i = 0; i < authTableSize; i++) {
        if ((uint16_t)authPacketHandler[i].cmd == _cmd) {
            (*this.*authPacketHandler[i].handler)(pGamePct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == authTableSize) {
        NG_LOG_DEBUG("server.network", "Got unknown packet '%d' from '%s'", pGamePct->GetPacketID(), GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Error;
    }
    return ReadDataHandlerResult::Ok;
}

void GameAuthSession::HandleClientLoginResult(XPacket *pRecvPct)
{
    auto szAccount = pRecvPct->ReadString(61);
    pRecvPct->rpos(0);
    if (m_queue.count(szAccount) == 1) {
        m_queue[szAccount]->ProcessIncoming(pRecvPct);
        m_queue.erase(szAccount);
    }
}

void GameAuthSession::HandleClientKick(XPacket *pRecvPct)
{
    auto szPlayer = pRecvPct->ReadString(61);
    auto player = Player::FindPlayer(szPlayer);
    if (player != nullptr) {
        player->GetSession().KickPlayer();
    }
}

void GameAuthSession::AccountToAuth(WorldSession *pSession, const std::string &szLoginName, uint64_t nOneTimeKey)
{
    m_queue[szLoginName] = pSession;
    TS_GA_CLIENT_LOGIN loginPct{};
    loginPct.account = szLoginName;
    loginPct.one_time_key = nOneTimeKey;
    SendPacket(loginPct);
}

int32_t GameAuthSession::GetAccountId() const
{
    return m_nGameIDX;
}

std::string GameAuthSession::GetAccountName()
{
    return m_szGameName;
}

void GameAuthSession::OnClose()
{
    NG_LOG_ERROR("server.network", "Authserver has closed connection!");
}

void GameAuthSession::HandleGameLoginResult(XPacket *pRecvPct)
{
    auto result = pRecvPct->read<uint16_t>();
    if (result != TS_RESULT_SUCCESS) {
        NG_LOG_ERROR("server.network", "Authserver refused login! Shutting down...");
        World::StopNow(ERROR_EXIT_CODE);
    }
}

void GameAuthSession::ClientLogoutToAuth(const std::string &szAccount)
{
    TS_GA_CLIENT_LOGOUT logoutPct{};
    logoutPct.account = szAccount;
    SendPacket(logoutPct);
}

void GameAuthSession::SendGameLogin()
{
    TS_GA_LOGIN loginPct{};
    loginPct.server_idx = m_nGameIDX;
    loginPct.server_name = m_szGameName;
    loginPct.server_screenshot_url = m_szGameSSU;
    loginPct.is_adult_server = static_cast<uint8_t>(m_bGameIsAdultServer ? 1 : 0);
    loginPct.server_ip = sConfigMgr->GetStringDefault("GameServer.ExternalIP", "127.0.0.1");
    loginPct.server_port = m_nGamePort;
    SendPacket(loginPct);
}

void GameAuthSession::HandleNullPacket(XPacket *) {}
