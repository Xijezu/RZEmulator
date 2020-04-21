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

        uint32_t                  idx{ };
        uint8_t               location_type{ };
        uint8_t               weather_ratio[7][4]{ };
        uint8_t               current_weather{ };
        uint32_t                  weather_change_time{ };
        uint32_t                  last_changed_time{ };
        int32_t                   shovelable_item{ };
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

        WorldLocation *AddToLocation(uint32_t idx, Player *player);
        bool RemoveFromLocation(Player *player);
        void SendWeatherInfo(uint32_t idx, Player *player);
        int32_t GetShovelableItem(uint32_t idx);
        uint32_t GetShovelableMonster(uint32_t idx);
        void RegisterWorldLocation(uint32_t idx, uint8_t location_type, uint32_t time_id, uint32_t weather_id, uint8_t ratio, uint32_t weather_change_time, int32_t shovelable_item);
        void RegisterMonsterLocation(uint32_t idx, uint32_t monster_id);

    private:
        NG_SHARED_MUTEX                             i_lock;
        std::vector<WorldLocation>                  m_vWorldLocation{ };
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_hsMonsterID{ };

    protected:
        WorldLocationManager() = default;
};
#define sWorldLocationMgr WorldLocationManager::Instance()