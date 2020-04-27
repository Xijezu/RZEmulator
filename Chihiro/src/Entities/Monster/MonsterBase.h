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
#include "ItemTemplate.hpp"

class Monster;

struct PARTY_DAMAGE
{
    PARTY_DAMAGE()
    {
        nDamage = 0;
        nLevel = 0;
    }

    PARTY_DAMAGE(int32_t _damage, int32_t _level)
        : nDamage(_damage)
        , nLevel(_level)
    {
    }
    int32_t nDamage;
    int32_t nLevel;
};

struct MonsterBase
{
    enum
    {
        FIGHT_TYPE_NORMAL = 0,
        FIGHT_TYPE_ENVIRONMENT = 1,
        FIGHT_TYPE_DUNGEON_CONNECTOR = 2,
        FIGHT_TYPE_BOSS = 3,
        FIGHT_TYPE_AGENT = 4,
        FIGHT_TYPE_AUTO_TRAP = 5,
        FIGHT_TYPE_TRAP = 6,
        FIGHT_TYPE_NOT_MOVABLE = 7,
    };

    enum
    {
        MONSTER_TYPE_NORMAL = 0,
        MONSTER_TYPE_WARRIOR = 1,
        MONSTER_TYPE_MAGICAN = 2,
        MONSTER_TYPE_SUMMONEE = 3,
        MONSTER_TYPE_RESERVED_1 = 4,
        MONSTER_TYPE_RESERVED_2 = 5,
        MONSTER_TYPE_RESERVED_3 = 6,
        MONSTER_TYPE_RESERVED_4 = 7,
        MONSTER_TYPE_RESERVED_5 = 8,
        MONSTER_TYPE_EVENT_GUARDIAN = 9,
        MONSTER_TYPE_ENHANCE = 11,
        MONSTER_TYPE_FIELD_SUB_BOSS = 12,
        MONSTER_TYPE_HUNTAHOLIC_1LV = 15,
        MONSTER_TYPE_HUNTAHOLIC_2LV = 16,
        MONSTER_TYPE_HUNTAHOLIC_3LV = 17,
        MONSTER_TYPE_HUNTAHOLIC_BOSS = 18,
        MONSTER_TYPE_DUNGEON = 21,
        MONSTER_TYPE_DUNGEON_SUB_BOSS = 22,
        MONSTER_TYPE_C_BOSS = 31,
        MONSTER_TYPE_B_BOSS = 32,
    };

    enum
    {
        MONSTER_RACE_DUNGEON_CORE = 20000,
        MONSTER_RACE_DUNGEON_CONNECTOR = 20001,
    };

    int32_t id;
    int32_t name_id;
    int32_t monster_group;
    int32_t location_id;
    int32_t level;
    int32_t grp;
    float size;
    float scale;
    int32_t magic_type;
    int32_t race;
    int32_t visible_range;
    int32_t chase_range;
    int32_t flag[5];
    int32_t monster_type;
    int32_t stat_id;
    int32_t fight_type;
    int32_t weapon_type;
    int32_t attack_motion_speed;
    int32_t ability;
    int32_t standard_walk_speed;
    int32_t standard_run_speed;
    int32_t walk_speed;
    int32_t run_speed;
    float attack_range;
    int32_t hp;
    int32_t mp;
    int32_t attacK_point;
    int32_t magic_point;
    int32_t defence;
    int32_t magic_defence;
    int32_t attack_speed;
    int32_t magic_speed;
    int32_t accuracy;
    int32_t avoid;
    int32_t magic_accuracy;
    int32_t magic_avoid;
    int32_t taming_id;
    float taming_percentage;
    float taming_exp_mod;
    int32_t exp[2];
    int32_t jp[2];
    int32_t gold_drop_percentage;
    int32_t gold_min[2];
    int32_t gold_max[2];
    int32_t chaos_drop_percentage;
    int32_t chaos_min[2];
    int32_t chaos_max[2];
    int32_t drop_item_id[10];
    int32_t drop_percentage[10];
    int32_t drop_min_count[10];
    int32_t drop_max_count[10];
    int32_t drop_min_level[10];
    int32_t drop_max_level[10];
    int32_t skill_id[4];
    int32_t skill_lv[4];
    float skill_probability[4];
    int32_t local_flag;
};

struct MonsterDeleteHandler
{
    virtual void onMonsterDelete(Monster *mob) = 0;
};

struct takePriority
{
    ItemPickupOrder PickupOrder{};
};

struct HateTag
{
    HateTag(uint32_t _uid, uint32_t _time, int32_t _hate)
        : uid(_uid)
        , nTime(_time)
        , nHate(_hate)
        , bIsActive(true)
        , nBadAttackCount(0)
        , nLastMaxHate(0)
    {
    }

    uint32_t uid;
    uint32_t nTime;
    int32_t nHate;
    bool bIsActive;
    int32_t nBadAttackCount;
    int32_t nLastMaxHate;
};

struct HateModifierTag
{
    HateModifierTag(uint32_t _uid, int32_t _hate)
    {
        uid = _uid;
        nHate = _hate;
    }

    uint32_t uid;
    int32_t nHate;
};

enum ATTACK_TYPE_FLAG : uint16_t
{
    AT_FIRST_ATTACK = 0x1,
    AT_GROUP_FIRST_ATTACK = 0x2,
    AT_RESPONSE_CASTING = 0x4,
    AT_RESPONSE_RACE = 0x8,
    AT_RESPONSE_BATTLE = 0x10
};

enum MONSTER_GENERATE_CODE : int
{
    MGC_NONE = 0,
    MGC_BY_RESPAWN = 1,
    MGC_BY_SCRIPT = 2,
    MGC_BY_SHOVELING = 3,
};

enum MONSTER_STATUS : int
{
    STATUS_NORMAL = 0,
    STATUS_TRACKING = 1,
    STATUS_FIND_ATTACK_POS = 2,
    STATUS_ATTACK = 3,
    STATUS_DEAD = 4,
};

struct MonsterRespawnInfo
{
    uint32_t interval;
    float left;
    float top;
    float right;
    float bottom;
    uint8_t layer;
    uint32_t monster_id;
    uint32_t max_num;
    uint32_t inc;
    uint32_t id;
    bool is_wandering;
    int32_t dungeon_id;
    int32_t way_point_id;

    MonsterRespawnInfo() = default;

    MonsterRespawnInfo(
        uint32_t _id, uint32_t _interval, float _left, float _top, float _right, float _bottom, uint32_t _monster_id, uint32_t _max_num, uint32_t _inc, bool _is_wandering, int32_t _way_point_id)
    {
        id = _id;
        interval = _interval;
        left = _left;
        top = _top;
        right = _right;
        bottom = _bottom;
        layer = 0;
        dungeon_id = 0;
        monster_id = _monster_id;
        max_num = _max_num;
        inc = _inc;
        is_wandering = _is_wandering;
        way_point_id = _way_point_id;
    }
};