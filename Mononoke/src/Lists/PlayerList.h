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
#ifndef __PLAYERLIST_H_
#define __PLAYERLIST_H_
#include "Common.h"
#include "SharedMutex.h"

class AuthClientSession;
// Storage object for a player
struct Player
{
	Player() : nAccountID(0), szLoginName(), bIsInGame(false), bIsBlocked(false),
			   nLastServerIDX(0), bKickNextLogin(false), nOneTimeKey(0), nGameIDX(-1)
	{
	}
	uint32 nAccountID;
	std::string szLoginName;
	bool bIsBlocked;
	uint32 nLastServerIDX;
	bool bIsInGame;
	bool bKickNextLogin;
	uint64 nOneTimeKey;
	int nGameIDX;
};

/// Storage object for the list of players on the server
class PlayerList
{
public:
	typedef std::map<std::string, Player*> PlayerMap;

	PlayerList() = default;
	~PlayerList() = default;

	void AddPlayer(Player* pNewPlayer)
	{
		// Adds a player to the list
		{
			MX_UNIQUE_GUARD writeGuard(*GetGuard());
			m_players[pNewPlayer->szLoginName] = pNewPlayer;
		}
	}

	void RemovePlayer(const std::string& szAccount)
	{
		// Removes a player from the list
		{
			MX_UNIQUE_GUARD writeGuard(*GetGuard());
			if (m_players.count(szAccount) == 1)
				m_players.erase(szAccount);
		}
	}

	MX_SHARED_MUTEX *GetGuard()
	{
		return &i_lock;
	}

	// You need to use the mutex while working with the map!
	PlayerMap* GetMap()
	{
		return &m_players;
	}

	Player* GetPlayer(const std::string& szAccount)
	{
		if(m_players.count(szAccount) == 1)
			return m_players[szAccount];
		return nullptr;
	}
private:
	MX_SHARED_MUTEX i_lock;
	PlayerMap m_players;                                  ///< Internal map of players
};

#define sPlayerMapList ACE_Singleton<PlayerList, ACE_Null_Mutex>::instance()
#endif // !__PLAYERLIST_H_
