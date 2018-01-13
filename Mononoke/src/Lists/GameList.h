/*
  *  Copyright (C) 2016-2016 Xijezu <http://xijezu.com/>
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

#ifndef _GAMELIST_H
#define _GAMELIST_H

#include "Common.h"
#include "SharedMutex.h"
class AuthGameSession;
// Storage object for a game
struct Game
{
	Game() : nIDX(0), szName(), szSSU(), bIsAdultServer(false), szIP(), nPort(0), m_pSession(nullptr) { }
	uint16 nIDX;
	std::string szName;
	std::string szSSU;
	bool bIsAdultServer;
	std::string szIP;
	int nPort;
	AuthGameSession *m_pSession;
};

/// Storage object for the list of realms on the server
class GameList
{
public:
	typedef std::map<uint, Game*> GameMap;

	GameList() = default;
	~GameList() = default;

	void AddGame(Game *pNewGame)
	{
		// Adds a game to the list
		{
			MX_UNIQUE_GUARD writeGuard(*GetGuard());
			m_games[pNewGame->nIDX] = pNewGame;
		}
	}

	void RemoveGame(const uint nIDX)
	{
		// Removes a game from the list
		{
			MX_UNIQUE_GUARD writeGuard(*GetGuard());
			if (m_games.count(nIDX) == 1)
				m_games.erase(nIDX);
		}
	}

	MX_SHARED_MUTEX *GetGuard()
	{
		return &i_lock;
	}

	// You need to use the mutex while working with the map!
	GameMap* GetMap()
	{
		return &m_games;
	}

	Game *GetGame(const uint nIDX)
	{
		if (m_games.count(nIDX) == 1)
			return m_games[nIDX];
		return nullptr;
	}

private:
	MX_SHARED_MUTEX i_lock{};
	GameMap         m_games;
};

#define sGameMapList ACE_Singleton<GameList, ACE_Null_Mutex>::instance()

#endif // _GAMELIST_H
