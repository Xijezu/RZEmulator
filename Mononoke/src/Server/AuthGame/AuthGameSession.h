#pragma once
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
#include "GameList.h"
#include "PlayerList.h"
#include "XSocket.h"

class XPacket;

// Handle login commands
class AuthGameSession : public XSession
{
    public:
        explicit AuthGameSession(XSocket *pSocket);
        ~AuthGameSession();

        // Network handlers
        void OnClose() override;
        ReadDataHandlerResult ProcessIncoming(XPacket *) override;

        bool IsEncrypted() const override
        {
            return true;
        }

        /// \brief Handles game login (packet contains servername, ...)
        void HandleGameLogin(XPacket *);
        /// \brief Handles client login on server - checks one-time-key
        void HandleClientLogin(XPacket *);
        /// \brief Handles client logout on server - removes player from our list
        void HandleClientLogout(XPacket *);
        /// \brief Handles client kick failed - not used yet
        void HandleClientKickFailed(XPacket *);
        /// \brief Receives and sends a Ping packet from/to the Gameserver
        void HandlePingPacket(XPacket *);
        /// \brief Kicks a player from the gameserver
        /// \param pPlayer The player to be kicked
        void KickPlayer(Player *pPlayer);

        /// \brief Gets the GameIDX - used in WorldSocket
        /// \return GameIDX
        int GetAccountId() const { return (m_pGame != nullptr ? m_pGame->nIDX : 0); }

        /// \brief Gets the server name - used in WorldSocket
        /// \return server name
        std::string GetAccountName() const { return (m_pGame != nullptr ? m_pGame->szName : "<null>"); }

    private:
        XSocket *m_pSocket;
        Game    *m_pGame;
        bool    m_bIsAuthed;
};