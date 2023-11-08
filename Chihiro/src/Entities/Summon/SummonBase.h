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

enum class SUMMON_DEFAULTS : int32_t {
    SUMMON_SPAWN_LENGTH = 70,
    SUMMON_SPAWN_MIN_LENGTH = 24,
    SUMMON_MAX_NON_ARTIFACT_ITEM_WEAR = 2,
    SUMMON_MAX_ACCESSORY_ITEM_WEAR = 3,
    SUMMON_MAX_ITEM_WEAR = 6,
};

enum class SUMMON_RATE : int32_t {
    RATE_BASIC = 0,
    RATE_COMMON = 1,
    RATE_UNCOMMON = 2,
    RATE_RARE = 3,
    RATE_UNIQUE = 4,
    RATE_EXTRA = 99,
};

enum class SUMMON_EVOLVE_TYPE : int32_t {
    EVOLVE_NORMAL = 1,
    EVOLVE_GROWTH = 2,
    EVOLVE_EVOLVE = 3,
};

struct SummonLevelBonus {
    int32_t summon_id;
    float_t strength;
    float_t vital;
    float_t dexterity;
    float_t agility;
    float_t intelligence;
    float_t mentality;
    float_t luck;
};

enum class SUMMON_RIDE_TYPE { CANT_RIDING = 0, RIDING_LENT = 1 };

struct SummonResourceTemplate {
    int32_t id;
    int32_t type;
    int32_t magic_type;
    int32_t rate;
    int32_t stat_id;
    float_t size;
    float_t scale;
    int32_t standard_walk_speed;
    int32_t standard_run_speed;
    int32_t walk_speed;
    int32_t run_speed;
    bool is_riding_only;
    float_t attack_range;
    int32_t material;
    int32_t weapon_type;
    int32_t form;
    int32_t evolve_target;
    int32_t card_id;
};