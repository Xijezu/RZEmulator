/*
  *  Copyright (C) 2016-2016 Xijezu <http://xijezu.com>
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

#ifndef _AUTHCLIENTSESSION_H_
#define _AUTHCLIENTSESSION_H_

#include <Lists/PlayerList.h>
#include "Common.h"
#include "Log.h"
#include "WorldSocket.h"
#include "Encryption/XRc4Cipher.h"
#include "XDes.h"

struct Player;

// Handle the player network
class AuthClientSession {
public:
    typedef WorldSocket<AuthClientSession> AuthSocket;
    explicit AuthClientSession(AuthSocket *socket);
    virtual ~AuthClientSession();

    // Accept & Close handler
    //void OnAccept() override;
    void OnClose();
    void ProcessIncoming(XPacket *);

    void HandleLoginPacket(XPacket *packet);
    void HandleVersion(XPacket *);
    void HandleServerList(XPacket *);
    void HandleSelectServer(XPacket *);
    void HandleNullPacket(XPacket *){ }
    void SendResultMsg(uint16 pctID, uint16 result, uint value);

    int GetAccountId() const { return m_pPlayer != nullptr ? m_pPlayer->nAccountID : 0; }
    std::string GetAccountName() const { return m_pPlayer != nullptr ? m_pPlayer->szLoginName : "<null>"; }
    AuthSocket *GetSocket() const { return _socket != nullptr ? _socket : nullptr; }
private:
    AuthSocket *_socket{nullptr};
    XRC4Cipher _rc4encode{ };
    XRC4Cipher _rc4decode{ };
    XDes _desCipther{};

    Player*     m_pPlayer{nullptr};
    bool        _isAuthed{false};
};

#endif // _AUTHCLIENTSESSION_H_