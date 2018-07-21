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
#include "Monster.h"

struct RespawnInfo : public MonsterRespawnInfo
{
    explicit RespawnInfo(MonsterRespawnInfo info) : MonsterRespawnInfo(info)
    {
        count              = 0;
        prespawn_count     = info.max_num / 2;
        if (prespawn_count < 1 || dungeon_id != 0 || way_point_id != 0)
            prespawn_count = info.max_num;
    }

    uint count;
    uint prespawn_count;
};

class RespawnObject : public MonsterDeleteHandler
{
    public:
        explicit RespawnObject(MonsterRespawnInfo rh);
        ~RespawnObject() = default;

        void onMonsterDelete(Monster *mob) override;
        void Update(uint diff);

    private:
        RespawnInfo       info;
        uint              m_nMaxRespawnNum;
        std::vector<uint> m_vRespawnedMonster;
        uint              lastDeadTime;
};