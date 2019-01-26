#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "ItemFields.h"

class Monster;

struct PARTY_DAMAGE
{
    PARTY_DAMAGE()
    {
        nDamage = 0;
        nLevel = 0;
    }

    PARTY_DAMAGE(int _damage, int _level) : nDamage(_damage), nLevel(_level) {}
    int nDamage;
    int nLevel;
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

    int id;
    int name_id;
    int monster_group;
    int location_id;
    int level;
    int grp;
    float size;
    float scale;
    int magic_type;
    int race;
    int visible_range;
    int chase_range;
    int flag[5];
    int monster_type;
    int stat_id;
    int fight_type;
    int weapon_type;
    int attack_motion_speed;
    int ability;
    int standard_walk_speed;
    int standard_run_speed;
    int walk_speed;
    int run_speed;
    float attack_range;
    int hp;
    int mp;
    int attacK_point;
    int magic_point;
    int defence;
    int magic_defence;
    int attack_speed;
    int magic_speed;
    int accuracy;
    int avoid;
    int magic_accuracy;
    int magic_avoid;
    int taming_id;
    float taming_percentage;
    float taming_exp_mod;
    int exp[2];
    int jp[2];
    int gold_drop_percentage;
    int gold_min[2];
    int gold_max[2];
    int chaos_drop_percentage;
    int chaos_min[2];
    int chaos_max[2];
    int drop_item_id[10];
    int drop_percentage[10];
    int drop_min_count[10];
    int drop_max_count[10];
    int drop_min_level[10];
    int drop_max_level[10];
    int skill_id[4];
    int skill_lv[4];
    float skill_probability[4];
    int local_flag;
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
    HateTag(uint _uid, uint _time, int _hate) : uid(_uid), nTime(_time), nHate(_hate), bIsActive(true), nBadAttackCount(0), nLastMaxHate(0)
    {
    }

    uint uid;
    uint nTime;
    int nHate;
    bool bIsActive;
    int nBadAttackCount;
    int nLastMaxHate;
};

struct HateModifierTag
{
    HateModifierTag(uint _uid, int _hate)
    {
        uid = _uid;
        nHate = _hate;
    }

    uint uid;
    int nHate;
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
    uint interval;
    float left;
    float top;
    float right;
    float bottom;
    uint8_t layer;
    uint monster_id;
    uint max_num;
    uint inc;
    uint id;
    bool is_wandering;
    int dungeon_id;
    int way_point_id;

    MonsterRespawnInfo() = default;

    MonsterRespawnInfo(uint _id, uint _interval, float _left, float _top, float _right, float _bottom, uint _monster_id, uint _max_num, uint _inc, bool _is_wandering, int _way_point_id)
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