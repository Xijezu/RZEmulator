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
#include <atomic>

#include "Common.h"
#include "GameRule.h"
#include "Unit.h"

class Monster;
struct MonsterDeleteHandler;
struct DropGroup;
struct WayPointInfo;
class NPC;
struct NPCTemplate;

class GameContent {
public:
    struct MonsterRespawnBaseInfo {
        MonsterRespawnBaseInfo(uint32_t _id = 0, uint32_t _interval = 0, uint32_t _inc = 0, bool _is_wandering = true, int32_t _way_point_id = 0, uint32_t _prespawn_count = 0)
            : id(_id)
            , interval(_interval)
            , layer(0)
            , inc(_inc)
            , is_wandering(_is_wandering)
            , dungeon_id(0)
            , way_point_id(_way_point_id)
            , count(0)
            , prespawn_count(_prespawn_count)
        {
        }

        uint32_t id;
        uint32_t interval;
        uint8_t layer;
        uint32_t inc;
        bool is_wandering;
        int32_t dungeon_id;
        int32_t way_point_id;
        std::atomic_uint32_t count;
        uint32_t prespawn_count;
    };

    struct MonsterRespawnInfo : MonsterRespawnBaseInfo {
        MonsterRespawnInfo(uint32_t _id = 0, uint32_t _interval = 0, float_t _left = 0, float_t _top = 0, float_t _right = 0, float_t _bottom = 0, uint32_t _monster_id = 0, uint32_t _max_num = 0,
            uint32_t _inc = 0, bool _is_wandering = true, int32_t _way_point_id = 0, int32_t _dungeon_id = 0, uint32_t _prespawn_count = 0)
            : MonsterRespawnBaseInfo(_id, _interval, _inc, _is_wandering, _way_point_id,
                  (!_dungeon_id && !_way_point_id && (_max_num * GameRule::MONSTER_PRESPAWN_RATE >= 1.0f)) ? (_max_num * GameRule::MONSTER_PRESPAWN_RATE) : _max_num)
            , left(_left)
            , top(_top)
            , right(_right)
            , bottom(_bottom)
            , monster_id(_monster_id)
            , max_num(_max_num)
        {
        }

        MonsterRespawnInfo(const MonsterRespawnInfo &rh)
            : MonsterRespawnBaseInfo(rh.id, rh.interval, rh.inc, rh.is_wandering, rh.way_point_id, rh.prespawn_count)
        {
            interval = rh.interval;
            monster_id = rh.monster_id;
            max_num = rh.max_num;
            inc = rh.inc;
            left = rh.left;
            right = rh.right;
            top = rh.top;
            bottom = rh.bottom;
            layer = rh.layer;
            is_wandering = rh.is_wandering;
            dungeon_id = rh.dungeon_id;
            count = 0;
            prespawn_count = (!rh.dungeon_id && !rh.way_point_id && (rh.max_num * GameRule::MONSTER_PRESPAWN_RATE >= 1.0f)) ? (rh.max_num * GameRule::MONSTER_PRESPAWN_RATE) : rh.max_num;
            way_point_id = rh.way_point_id;
        }

        float_t left, top, right, bottom;
        uint32_t monster_id;
        uint32_t max_num;
    };

    struct RandomMonsterRespawnInfo : MonsterRespawnBaseInfo {
        RandomMonsterRespawnInfo(uint32_t _id = 0, uint32_t _interval = 0, uint32_t _random_area_id = 0, uint32_t _inc = 0, bool _is_wandering = true, int32_t _way_point_id = 0,
            uint32_t _prespawn_count = 0, bool _except_raid_siege = false)
            : MonsterRespawnBaseInfo(_id, _interval, _inc, _is_wandering, _way_point_id, _prespawn_count)
            , random_area_id(_random_area_id)
            , except_raid_siege(_except_raid_siege)
        {
        }

        uint32_t random_area_id;
        std::vector<std::pair<int32_t, int32_t>> monster_list;
        bool except_raid_siege;
    };

    struct RoamingCreatureRespawnInfo {
        RoamingCreatureRespawnInfo(const int32_t eCreatureType, const int32_t nCreatureID, const uint32_t nRespawnInterval, const int32_t nAngle, const float_t nDistance)
            : m_eCreatureType(eCreatureType)
            , m_nCreatureID(nCreatureID)
            , m_nRespawnInterval(nRespawnInterval)
            , m_nAngle(nAngle)
            , m_nDistance(nDistance)
        {
        }

        int32_t m_eCreatureType;
        int32_t m_nCreatureID;
        uint32_t m_nRespawnInterval;
        int32_t m_nAngle;
        float_t m_nDistance;
    };

    static Monster *RespawnMonster(float_t x, float_t y, uint8_t layer, int32_t id, bool is_wandering, int32_t way_point_id, MonsterDeleteHandler *pDeleteHandler, bool bNeedLock);
    static bool IsInRandomPoolMonster(int32_t group_id, int32_t monster_id);
    static bool LearnAllSkill(Unit *pPlayer);
    static bool SelectItemIDFromDropGroup(int32_t nDropGroupID, int32_t &nItemID, int64_t &nItemCount);
    static uint16_t IsLearnableSkill(Unit *, int, int, int32_t &);
    static int32_t GetLocationID(float_t x, float_t y);
    static bool IsBlocked(float_t x, float_t y);
    static bool CollisionToLine(float_t x1, float_t y1, float_t x2, float_t y2);
    static NPC *GetNewNPC(NPCTemplate *npc_info, uint8_t layer);
    static void AddNPCToWorld();
    static int64_t GetItemSellPrice(int64_t price, int32_t rank, int32_t lv, bool same_price_for_buying);

private:
    static uint16_t isLearnableSkill(Unit *pUnit, int32_t skill_id, int32_t skill_level, int32_t nJobID, int32_t unit_job_level);
    GameContent() = default;
    ~GameContent() = default;
};