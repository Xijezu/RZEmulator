#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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

class Player;

enum WL_Time : int
{
    Dawn    = 0,
    Daytime = 1,
    Evening = 2,
    Night   = 3,
    WLT_Max = 4,
};

enum WL_Weather : int
{
    Clear     = 0,
    LightRain = 1,
    Rain      = 2,
    HeavyRain = 3,
    LightSnow = 4,
    Snow      = 5,
    HeavySnow = 6,
    WLW_Max   = 7
};

enum WL_Type : int
{
    WT_Etc            = 0,
    Town              = 1,
    WL_Field          = 2,
    NonPkField        = 3,
    Dungeon           = 4,
    BattleField       = 5,
    EventMap          = 7,
    HuntaholicLobby   = 8,
    HuntaholicDungeon = 9,
    FleaMarket        = 10,
};

enum WL_SpecLocId : int
{
    Abyss           = 110900,
    SecRoute1       = 130100,
    SecRoute2       = 130101,
    SecRouteAuction = 130107,
};

class WorldLocation
{
    public:
        WorldLocation() = default;
        WorldLocation(const WorldLocation &src);
        ~WorldLocation() = default;

        uint                  idx{ };
        uint8_t               location_type{ };
        uint8_t               weather_ratio[7][4]{ };
        uint8_t               current_weather{ };
        uint                  weather_change_time{ };
        uint                  last_changed_time{ };
        int                   shovelable_item{ };
        std::vector<Player *> m_vIncludeClient{ };
};

class WorldLocationManager
{
    public:
        static WorldLocationManager &Instance()
        {
            static WorldLocationManager instance;
            return instance;
        }

        ~WorldLocationManager() = default;

        WorldLocation *AddToLocation(uint idx, Player *player);
        bool RemoveFromLocation(Player *player);
        void SendWeatherInfo(uint idx, Player *player);
        int GetShovelableItem(uint idx);
        uint GetShovelableMonster(uint idx);
        void RegisterWorldLocation(uint idx, uint8_t location_type, uint time_id, uint weather_id, uint8_t ratio, uint weather_change_time, int shovelable_item);
        void RegisterMonsterLocation(uint idx, uint monster_id);

    private:
        NG_SHARED_MUTEX                             i_lock;
        std::vector<WorldLocation>                  m_vWorldLocation{ };
        std::unordered_map<uint, std::vector<uint>> m_hsMonsterID{ };

    protected:
        WorldLocationManager() = default;
};
#define sWorldLocationMgr WorldLocationManager::Instance()