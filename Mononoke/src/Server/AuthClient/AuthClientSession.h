#pragma once
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
#include <Lists/PlayerList.h>
#include "Common.h"
#include "Encryption/XRc4Cipher.h"
#include "XDes.h"
#include "XSocket.h"

struct Player;

// Handle the player network
class AuthClientSession : public XSocket
{
  public:
    explicit AuthClientSession(boost::asio::ip::tcp::socket &&socket);
    ~AuthClientSession();

    void OnClose() override;
    ReadDataHandlerResult ProcessIncoming(XPacket *) override;

    /// \brief Handler for the Login packet - checks accountname and password
    /// \param packet received packet
    void HandleLoginPacket(const TS_CA_ACCOUNT *);
    /// \brief Handler for the Version packet, not used yet
    void HandleVersion(const TS_CA_VERSION *);
    /// \brief Handler for the server list packet - sends the client the currently connected gameserver(s)
    void HandleServerList(const TS_CA_SERVER_LIST *);
    /// \brief Handler for the select server packet - generates one time key for gameserver login
    void HandleSelectServer(const TS_CA_SELECT_SERVER *);

    /// \brief Sends a result message to the client
    /// \param pctID Response to received pctID
    /// \param result TS_RESULT code
    /// \param value More informations
    void SendResultMsg(uint16_t pctID, uint16_t result, uint32_t value);

    /// \brief Gets the current players AccountID, used in WorldSocket
    /// \return account id
    int GetAccountId() const { return m_pPlayer != nullptr ? m_pPlayer->nAccountID : 0; }

    /// \brief Gets the current players account name, used in WorldSocket
    /// \return account name
    std::string GetAccountName() const { return m_pPlayer != nullptr ? m_pPlayer->szLoginName : "<null>"; }

  private:
    XDes _desCipther{};

    Player *m_pPlayer{nullptr};
    bool _isAuthed{false};
};