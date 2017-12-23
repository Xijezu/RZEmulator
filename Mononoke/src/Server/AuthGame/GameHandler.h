/*
  *  Copyright (C) 2016-2016 Xijezu <http://xijezu.com>
  *  Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
  *  Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  *  Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
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

#ifndef _GAMESOCKET_H
#define _GAMESOCKET_H

#include "Common.h"
#include "Server/XSocket.h"
#include "Lists/GameList.h"

// Handle login commands
class GameHandler: public XSocket::Session
{
public:
    explicit GameHandler(XSocket& socket);
    virtual ~GameHandler() = default;

	// Network handlers
    virtual void OnAccept();
    virtual void OnClose();

    // Leave empty for game server session, this one doesn't use encryption
	virtual void Decrypt(void*, size_t, bool/* =false */) { };
	virtual void Encrypt(void*, size_t, bool/* =false */) { };

	virtual void ProcessIncoming(XPacket*);

    // Packet handlers
    bool HandleGameLogin(XPacket*);
	bool HandleClientLogin(XPacket*);
	bool HandleClientLogout(XPacket*);
	bool HandleClientKickFailed(XPacket*);

private:
	XSocket& _socket;
	Game _game{};
	bool _isAuthed{false};
};

#endif // _GAMESOCKET_H