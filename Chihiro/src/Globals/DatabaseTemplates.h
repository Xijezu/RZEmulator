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

#include "ItemTemplate.hpp"

constexpr int32_t MAX_DROP_GROUP = 10;

struct DropGroup {
    int32_t uid;
    int32_t drop_item_id[MAX_DROP_GROUP];
    float_t drop_percentage[MAX_DROP_GROUP];
};

struct LevelResourceTemplate {
    int32_t level;
    int64_t normal_exp;
    int32_t jlv[4];
};

struct JobLevelBonusTemplate {
    int32_t job_id;
    float_t strength[4];
    float_t vital[4];
    float_t dexterity[4];
    float_t agility[4];
    float_t intelligence[4];
    float_t mentality[4];
    float_t luck[4];
};

struct JobResourceTemplate {
    int32_t id;
    int32_t stat_id;
    int32_t job_class;
    int32_t job_depth;
    int32_t up_lv;
    int32_t up_jlv;
    int32_t available_job[4];
};

struct MarketInfo {
    int32_t sort_id;
    std::string name;
    uint32_t code;
    float_t price_ratio;
    float_t huntaholic_ratio;
};