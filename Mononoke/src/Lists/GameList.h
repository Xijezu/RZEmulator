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
#include <map>

#include "Common.h"
#include "SharedMutex.h"
#include "TS_AC_SERVER_LIST.h"

class AuthGameSession;
struct Game : public TS_SERVER_INFO {
    AuthGameSession *m_pSession;
};

class GameList {
public:
    typedef std::map<uint16_t, std::shared_ptr<Game>> GameMap;
    ~GameList() = default;

    static GameList &Instance()
    {
        static GameList instance;
        return instance;
    }

    void AddGame(std::shared_ptr<Game> pNewGame)
    {
        // Adds a game to the list
        {
            NG_UNIQUE_GUARD writeGuard(*GetGuard());
            m_games[pNewGame->server_idx] = pNewGame;
        }
    }

    void RemoveGame(const uint16_t nIDX)
    {
        // Removes a game from the list
        {
            NG_UNIQUE_GUARD writeGuard(*GetGuard());
            if (m_games.count(nIDX) == 1)
                m_games.erase(nIDX);
        }
    }

    NG_SHARED_MUTEX *GetGuard() { return &i_lock; }

    // You need to use the mutex while working with the map!
    GameMap *GetMap() { return &m_games; }

    const Game *GetGame(const uint16_t nIDX)
    {
        if (m_games.count(nIDX) == 1)
            return m_games[nIDX].get();
        return nullptr;
    }

private:
    NG_SHARED_MUTEX i_lock{};
    GameMap m_games;

protected:
    GameList() = default;
};

#define sGameMapList GameList::Instance()