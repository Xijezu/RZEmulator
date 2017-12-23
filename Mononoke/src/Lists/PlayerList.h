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

#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>
#include <ace/INET_Addr.h>

// Storage object for a game
struct Player
{
	uint32 account_id;
	std::string login_name;
	bool block;
	uint32 last_login_server_idx;
	bool isInGame;
	uint64 one_time_key;
};

/// Storage object for the list of players on the server
class PlayerList
{
public:
	typedef std::map<std::string, Player> PlayerMap;

	PlayerList() {}
	~PlayerList() {}

	void AddPlayer(const Player NewPlayer) { m_players[NewPlayer.login_name] = NewPlayer; }
	void RemovePlayer(std::string key) { if (contains(key)) m_players.erase(key); }
	void UpdatePlayer(const Player player)
	{ 
		if (contains(player.login_name)) 
			m_players[player.login_name] = player; 
	}
	PlayerMap GetMap() const { return m_players; }

	uint32 size() const { return m_players.size(); }
	bool contains(std::string idx) const { return m_players.count(idx) == 1 ? true : false; }
private:
	PlayerMap m_players;                                  ///< Internal map of players
};

#define sPlayerMapList ACE_Singleton<PlayerList, ACE_Null_Mutex>::instance()
#endif // !__PLAYERLIST_H_
