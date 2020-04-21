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

#include "ItemFields.h"

constexpr int32_t MAX_DROP_GROUP = 10;

struct DropGroup
{
    int32_t uid;
    int32_t drop_item_id[MAX_DROP_GROUP];
    float drop_percentage[MAX_DROP_GROUP];
};

struct LevelResourceTemplate
{
    int32_t level;
    int64_t normal_exp;
    int32_t jlv[4];
};

struct ItemTemplate
{
    int32_t id;
    int32_t name_id;
    int32_t type;
    int32_t group;
    int32_t iclass;
    int32_t wear_type;
    int32_t set_id;
    int32_t set_part_flag;
    int32_t rank;
    int32_t level;
    int32_t enhance;
    int32_t socket;
    int32_t status_flag;
    int32_t limit_deva;
    int32_t limit_asura;
    int32_t limit_gaia;
    int32_t limit_fighter;
    int32_t limit_hunter;
    int32_t limit_magician;
    int32_t limit_summoner;
    int32_t nLimit;
    int32_t use_min_level;
    int32_t use_max_level;
    int32_t target_min_level;
    int32_t target_max_level;
    float range;
    float weight;
    int32_t price;
    int32_t endurance;
    int32_t material;
    int32_t summon_id;
    int8_t flaglist[19];
    int32_t available_period;
    int16_t decrease_type;
    float throw_range;
    int8_t distribute_type;
    int16_t base_type[4];
    float base_var[4][2];
    int16_t opt_type[4];
    float opt_var[4][2];
    int16_t enhance_id[2];
    float _enhance[2][4];
    int32_t skill_id;
    int32_t state_id;
    int32_t state_level;
    int32_t state_time;
    int32_t state_type;
    int32_t cool_time;
    int16_t cool_time_group;
    std::string script_text;

    void SetCombinedFlags()
    {
        if (limit_asura != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_ASURA);
        if (limit_gaia != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_GAIA);
        if (limit_deva != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_DEVA);

        if (limit_hunter != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_HUNTER);
        if (limit_fighter != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_FIGHTER);
        if (limit_magician != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_MAGICIAN);
        if (limit_summoner != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_SUMMONER);
    }
};

struct JobLevelBonusTemplate
{
    int32_t job_id;
    float strength[4];
    float vital[4];
    float dexterity[4];
    float agility[4];
    float intelligence[4];
    float mentality[4];
    float luck[4];
};

struct JobResourceTemplate
{
    int32_t id;
    int32_t stat_id;
    int32_t job_class;
    int32_t job_depth;
    int32_t up_lv;
    int32_t up_jlv;
    int32_t available_job[4];
};

struct MarketInfo
{
    int32_t sort_id;
    std::string name;
    uint32_t code;
    float price_ratio;
    float huntaholic_ratio;
};