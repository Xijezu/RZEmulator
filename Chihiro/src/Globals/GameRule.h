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

class GameRule
{
public:
    static float GetItemLevelPenalty(int32_t creature_level, int32_t item_rank, int32_t item_level);
    static int32_t GetItemRecommendedLevel(int32_t item_rank, int32_t item_level);
    static int32_t GetItemLevelLimit(int32_t item_rank);
    static int32_t GetItemRecommendModTable(int32_t item_rank);
    static int32_t GetRankLevel(int32_t rank);
    static int32_t GetLocalFlag();
    static float GetItemDropRate();
    static float GetGoldDropRate();
    static float GetChaosDropRate();
    static float GetEXPRate();
    static float GetItemValue(float, int, int, int, int32_t);
    static float GetPickableRange();
    static int32_t GetChipLevelLimit(int32_t idx);
    static int32_t GetBaseMoveSpeed();

    static int32_t GetMaxLevel() { return 150; }

    static int32_t GetIntValueByRandomInt(double fValue);
    static int64_t GetIntValueByRandomInt64(double fValue);
    static float GetStaminaRatio(int32_t level);
    static float GetStaminaBonus();
    static int32_t nEnhanceFailType;
    constexpr static float DEFAULT_UNIT_SIZE{12.0f};
    constexpr static int32_t SKILL_CAST_COST{0};
    constexpr static int32_t ATTACK_RANGE_UNIT{100};
    constexpr static float MAX_ATTACK_RANGE{0.84f};
    constexpr static float REFLECT_RANGE{4.0f * DEFAULT_UNIT_SIZE};
    constexpr static int32_t MAX_STATE_LEVEL{65535};
    constexpr static float UNMOUNT_PENALTY{0.05f};
    constexpr static float fPVPDamageRateForPlayer{0.2f};
    constexpr static float fPVPDamageRateForSummon{0.13f};
    constexpr static float MONSTER_PRESPAWN_RATE{0.5f};
    constexpr static float MONSTER_TRACKING_RANGE_BY_TIME{30 * DEFAULT_UNIT_SIZE};
    constexpr static int32_t MONSTER_FIND_ATTACK_POS_RATIO{5};
    constexpr static int32_t MAX_FIRST_ATTACK_BONUS_TIME{6000};
    constexpr static float SUMMON_FOLLOWING_LIMIT_RANGE{150 * DEFAULT_UNIT_SIZE};
    constexpr static float SUMMON_FOLLOWING_SECOND_SPEED_UP_RATE{1.5f};
    constexpr static float SUMMON_FOLLOWING_SECOND_SPEED_UP_RANGE{40 * DEFAULT_UNIT_SIZE};
    constexpr static float SUMMON_FOLLOWING_FIRST_SPEED_UP_RANGE{3 * DEFAULT_UNIT_SIZE};
    constexpr static float SUMMON_FOLLOWING_FIRST_SPEED_UP_RATE{1.2f};
    constexpr static float VISIBLE_RANGE{525.0f};
    constexpr static int32_t SPEED_UNIT{30};
    constexpr static int32_t DEFAULT_RUSH_SPEED{140};
    constexpr static int32_t DEFAULT_KNOCK_BACK_SPEED{140};

private:
    static int32_t _chipLevelLimit[];
    static int32_t _modtable[];
    static float _staminaExpRate[];
};