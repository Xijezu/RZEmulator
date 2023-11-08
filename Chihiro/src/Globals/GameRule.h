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

class GameRule {
public:
    static float_t GetItemLevelPenalty(int32_t creature_level, int32_t item_rank, int32_t item_level);
    static int32_t GetItemRecommendedLevel(int32_t item_rank, int32_t item_level);
    static int32_t GetItemLevelLimit(int32_t item_rank);
    static int32_t GetItemRecommendModTable(int32_t item_rank);
    static int32_t GetRankLevel(int32_t rank);
    static int32_t GetLocalFlag();
    static float_t GetItemDropRate();
    static float_t GetGoldDropRate();
    static float_t GetChaosDropRate();
    static float_t GetEXPRate();
    static float_t GetItemValue(float, int, int, int, int32_t);
    static float_t GetPickableRange();
    static int32_t GetChipLevelLimit(int32_t idx);
    static int32_t GetBaseMoveSpeed();

    static int32_t GetMaxLevel() { return 150; }

    static int32_t GetIntValueByRandomInt(double fValue);
    static int64_t GetIntValueByRandomInt64(double fValue);
    static float_t GetStaminaRatio(int32_t level);
    static float_t GetStaminaBonus();
    static int32_t nEnhanceFailType;
    constexpr static float_t DEFAULT_UNIT_SIZE{12.0f};
    constexpr static int32_t SKILL_CAST_COST{0};
    constexpr static int32_t ATTACK_RANGE_UNIT{100};
    constexpr static float_t MAX_ATTACK_RANGE{0.84f};
    constexpr static float_t REFLECT_RANGE{4.0f * DEFAULT_UNIT_SIZE};
    constexpr static int32_t MAX_STATE_LEVEL{65535};
    constexpr static float_t UNMOUNT_PENALTY{0.05f};
    constexpr static float_t fPVPDamageRateForPlayer{0.2f};
    constexpr static float_t fPVPDamageRateForSummon{0.13f};
    constexpr static float_t MONSTER_PRESPAWN_RATE{0.5f};
    constexpr static float_t MONSTER_TRACKING_RANGE_BY_TIME{30 * DEFAULT_UNIT_SIZE};
    constexpr static int32_t MONSTER_FIND_ATTACK_POS_RATIO{5};
    constexpr static int32_t MAX_FIRST_ATTACK_BONUS_TIME{6000};
    constexpr static float_t SUMMON_FOLLOWING_LIMIT_RANGE{150 * DEFAULT_UNIT_SIZE};
    constexpr static float_t SUMMON_FOLLOWING_SECOND_SPEED_UP_RATE{1.5f};
    constexpr static float_t SUMMON_FOLLOWING_SECOND_SPEED_UP_RANGE{40 * DEFAULT_UNIT_SIZE};
    constexpr static float_t SUMMON_FOLLOWING_FIRST_SPEED_UP_RANGE{3 * DEFAULT_UNIT_SIZE};
    constexpr static float_t SUMMON_FOLLOWING_FIRST_SPEED_UP_RATE{1.2f};
    constexpr static float_t VISIBLE_RANGE{525.0f};
    constexpr static int32_t SPEED_UNIT{30};
    constexpr static uint8_t DEFAULT_RUSH_SPEED{140};
    constexpr static uint8_t DEFAULT_KNOCK_BACK_SPEED{140};
    constexpr static int32_t MORAL_LIMIT{100};
    constexpr static int32_t CRIME_LIMIT{100};
    constexpr static int32_t GM_PERMISSION{100};

    constexpr static int32_t RACE_DEVA = 91;
    constexpr static int32_t RACE_ASURA = 92;
    constexpr static int32_t RACE_ANT = 11;
    constexpr static int32_t RACE_CHICKEN = 12;
    constexpr static int32_t RACE_STONE_TURTLE = 13;
    constexpr static int32_t RACE_SIREN = 32;
    constexpr static int32_t RACE_WHITE_DRAGON = 73;
    constexpr static int32_t RACE_GOBLIN_SKELECTON = 81;
    constexpr static int32_t JOB_HUNTER = 0;

    constexpr static int32_t CREATURE_NONE = -1;
    constexpr static int32_t CREATURE_ETC = 0;
    constexpr static int32_t CREATURE_BEAST = 1;
    constexpr static int32_t CREATURE_SEMIHUMAN = 2;
    constexpr static int32_t CREATURE_ELEMENTAL = 3;
    constexpr static int32_t CREATURE_ANGEL = 4;
    constexpr static int32_t CREATURE_DEVIL = 5;
    constexpr static int32_t CREATURE_MECHA = 6;
    constexpr static int32_t CREATURE_DRAGON = 7;
    constexpr static int32_t CREATURE_UNDEAD = 8;
    constexpr static int32_t CREATURE_HUMAN = 9;
    constexpr static int32_t CREATURE_ALL = 99;

private:
    static int32_t _chipLevelLimit[];
    static int32_t _modtable[];
    static float_t _staminaExpRate[];
};