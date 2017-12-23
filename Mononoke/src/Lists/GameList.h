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

#include <ace/Singleton.h>
#include <ace/Null_Mutex.h>
#include <ace/INET_Addr.h>

// Storage object for a game
struct Game
{
	uint16_t server_idx;
	std::string server_name;
	std::string server_screenshot_url;
	bool is_adult_server;
	std::string server_ip;
	int32_t server_port;
};

/// Storage object for the list of realms on the server
class GameList
{
public:
    typedef std::map<int32_t, Game> GameMap;

    GameList() {}
    ~GameList() {}

    void AddGame(const Game NewRealm) { m_games[NewRealm.server_idx] = NewRealm; }
	void RemoveGame(int32_t key) { if(contains(key)) m_games.erase(key); }
	GameMap GetMap() const { return m_games; }

	GameMap::const_iterator begin() const { return m_games.begin(); }
	GameMap::const_iterator end() const { return m_games.end(); }
    uint32 size() const { return m_games.size(); }
	bool contains(int32_t idx) const { return m_games.count(idx) == 1 ? true : false; }

private:

	GameMap m_games;                                  ///< Internal map of realms
    uint32   m_UpdateInterval;
    time_t   m_NextUpdateTime;
};

#define sGameMapList ACE_Singleton<GameList, ACE_Null_Mutex>::instance()

#endif // _GAMELIST_H
