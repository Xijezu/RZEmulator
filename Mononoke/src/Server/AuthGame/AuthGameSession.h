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
#include "GameList.h"

template<class T> class WorldSocket;
class XPacket;

// Handle login commands
class AuthGameSession
{
public:
	typedef WorldSocket<AuthGameSession> GameSocket;
	explicit AuthGameSession(GameSocket *pSocket);
	~AuthGameSession();

	// Network handlers
	void OnClose();
	void ProcessIncoming(XPacket *);

	// Packet handlers
	void HandleGameLogin(XPacket *);
	void HandleClientLogin(XPacket *);
	void HandleClientLogout(XPacket *);
	void HandleClientKickFailed(XPacket *);

	int GetAccountId() const { return (m_pGame != nullptr ? m_pGame->nIDX : 0); }
	std::string GetAccountName() const { return (m_pGame != nullptr ? m_pGame->szName : "<null>"); }

private:
	GameSocket *m_pSocket;
	Game       *m_pGame;
	bool       m_bIsAuthed;
};

#endif // _GAMESOCKET_H