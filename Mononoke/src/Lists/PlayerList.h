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
#include "Common.h"
#include "SharedMutex.h"
#include <map>

class AuthClientSession;
// Storage object for a player
struct Player
{
    Player() : nAccountID(0), szLoginName(), bIsBlocked(false), nLastServerIDX(0),
               bIsInGame(false), bKickNextLogin(false), nOneTimeKey(0), nGameIDX(-1),
               nPermission(0)
    {
    }

    uint32_t nAccountID;
    std::string szLoginName;
    bool bIsBlocked;
    uint32_t nLastServerIDX;
    bool bIsInGame;
    bool bKickNextLogin;
    uint64_t nOneTimeKey;
    int nGameIDX;
    int nPermission;
};

/// Storage object for the list of players on the server
class PlayerList
{
  public:
    typedef std::map<std::string, Player *> PlayerMap;
    ~PlayerList() = default;

    static PlayerList &Instance()
    {
        static PlayerList instance;
        return instance;
    }

    void AddPlayer(Player *pNewPlayer)
    {
        // Adds a player to the list
        {
            NG_UNIQUE_GUARD writeGuard(*GetGuard());
            m_players[pNewPlayer->szLoginName] = pNewPlayer;
        }
    }

    void RemovePlayer(const std::string &szAccount)
    {
        // Removes a player from the list
        {
            NG_UNIQUE_GUARD writeGuard(*GetGuard());
            if (m_players.count(szAccount) == 1)
                m_players.erase(szAccount);
        }
    }

    NG_SHARED_MUTEX *GetGuard()
    {
        return &i_lock;
    }

    // You need to use the mutex while working with the map!
    PlayerMap *GetMap()
    {
        return &m_players;
    }

    Player *GetPlayer(const std::string &szAccount)
    {
        if (m_players.count(szAccount) == 1)
            return m_players[szAccount];
        return nullptr;
    }

  private:
    NG_SHARED_MUTEX i_lock;
    PlayerMap m_players; ///< Internal map of players
  protected:
    PlayerList() = default;
};

#define sPlayerMapList PlayerList::Instance()