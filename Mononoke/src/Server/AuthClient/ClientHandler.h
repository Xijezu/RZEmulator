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

#ifndef _CLIENTHANDLER_H
#define _CLIENTHANDLER_H

#include "Common.h"
#include "XSocket.h"
#include "Encryption/XRc4Cipher.h"
#include "Encryption/XDes.h"
#include "PlayerList.h"

class ClientHandler: public XSocket::Session
{
public:
    explicit ClientHandler(XSocket& socket);
    virtual ~ClientHandler();

	virtual void OnAccept();
	virtual void OnClose();
	virtual void Decrypt(void*, size_t, bool/* =false */);
	virtual void Encrypt(void*, size_t, bool/* =false */);
	virtual void ProcessIncoming(XPacket*);

    bool HandleLoginPacket(XPacket *packet);
	bool HandleVersion(XPacket *);
	bool HandleServerList(XPacket *);
	bool HandleSelectServer(XPacket *);
	// Used for ping packet
	bool HandleNULL(XPacket*) { return true; }

private:
	XSocket&	_socket;
	XRC4Cipher	_rc4encode;
	XRC4Cipher	_rc4decode;
	XDes		_descipher;
	Player		_player;
	bool		_bIsAuthed{false};
};

#endif