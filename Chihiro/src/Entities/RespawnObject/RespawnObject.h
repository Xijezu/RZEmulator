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
#include "GameRule.h"
#include "Monster.h"

struct RespawnInfo : public MonsterRespawnInfo {
    explicit RespawnInfo(MonsterRespawnInfo info)
        : MonsterRespawnInfo(info)
        , count(0)
    {
        prespawn_count = (!info.dungeon_id && !info.way_point_id && (info.max_num * GameRule::MONSTER_PRESPAWN_RATE >= 1.0f)) ? (info.max_num * GameRule::MONSTER_PRESPAWN_RATE) : info.max_num;
        way_point_id = info.way_point_id;
    }

    uint32_t count;
    uint32_t prespawn_count;
};

class RespawnObject : public MonsterDeleteHandler {
public:
    explicit RespawnObject(MonsterRespawnInfo rh);
    ~RespawnObject() = default;

    // Deleting the copy & assignment operators
    // Better safe than sorry
    RespawnObject(const RespawnObject &) = delete;
    RespawnObject &operator=(const RespawnObject &) = delete;

    void onMonsterDelete(Monster *mob) override;
    void Update(uint32_t diff);

private:
    RespawnInfo info;
    uint32_t m_nMaxRespawnNum;
    std::vector<uint32_t> m_vRespawnedMonster;
    uint32_t lastDeadTime;
};