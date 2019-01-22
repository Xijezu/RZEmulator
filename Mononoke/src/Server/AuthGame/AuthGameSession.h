#pragma once
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
#include "GameList.h"
#include "PlayerList.h"
#include "XSocket.h"

class XPacket;

// Handle login commands
class AuthGameSession : public XSocket
{
  public:
    explicit AuthGameSession(boost::asio::ip::tcp::socket &&socket);
    ~AuthGameSession();

    // Network handlers
    void OnClose() override;
    ReadDataHandlerResult ProcessIncoming(XPacket *) override;

    bool IsEncrypted() const override { return false; }

    /// \brief Handles game login (packet contains servername, ...)
    void HandleGameLogin(const TS_GA_LOGIN *);
    /// \brief Handles client login on server - checks one-time-key
    void HandleClientLogin(const TS_GA_CLIENT_LOGIN *);
    /// \brief Handles client logout on server - removes player from our list
    void HandleClientLogout(const TS_GA_CLIENT_LOGOUT *);
    /// \brief Handles client kick failed - not used yet
    void HandleClientKickFailed(const TS_GA_CLIENT_KICK_FAILED *);
    /// \brief Receives and sends a Ping packet from/to the Gameserver
    void HandlePingPacket(const TS_CS_PING *);
    /// \brief Kicks a player from the gameserver
    /// \param pPlayer The player to be kicked
    void KickPlayer(Player *pPlayer);

    /// \brief Gets the GameIDX - used in WorldSocket
    /// \return GameIDX
    int GetAccountId() const { return (m_pGame != nullptr ? m_pGame->server_idx : 0); }

    /// \brief Gets the server name - used in WorldSocket
    /// \return server name
    std::string GetAccountName() const { return (m_pGame != nullptr ? m_pGame->server_name : "<null>"); }

  private:
    Game *m_pGame;
    bool m_bIsAuthed;
};