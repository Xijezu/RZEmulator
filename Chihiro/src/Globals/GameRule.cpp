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

#include "GameRule.h"
#include "World.h"

int32_t GameRule::_modtable[8] = {0, 3, 3, 2, 2, 3, 2, 2};
int32_t GameRule::_chipLevelLimit[8] = {20, 50, 80, 100, 120, 150, 180, 200};
float GameRule::_staminaExpRate[300] = {0};
int32_t GameRule::nEnhanceFailType = 2;

float GameRule::GetItemValue(float item_current_value, int32_t item_rank_value, int32_t creature_level, int32_t item_rank, int32_t item_level)
{
    float ilp = GetItemLevelPenalty(creature_level, item_rank, item_level);
    //-,*,+
    float v8 = (item_current_value - item_rank_value) * ilp;
    return v8 + item_rank_value;
    //float f1 = item_current_value * item_rank_value;
    //return f1 * ilp;
}

/*
 * NO TOUCH, IS WORK! NO TOUCH IF NO BROKE
 * No seriously, I tried to reconstruct it from retails pseudocode
 * but for some odd reasons I always got odd results
 * finally I tried to reconstruct it more or less 1:1 and this is the result
 * I fricking hate IDAs pseudocode plugin...
*/
float GameRule::GetItemLevelPenalty(int32_t creature_level, int32_t item_rank, int32_t item_level)
{
    float result = 0;
    int32_t recommended = GetItemRecommendedLevel(item_rank, item_level);
    int32_t limit = GetItemLevelLimit(item_rank);

    if (item_level == 1 || creature_level < limit || creature_level >= recommended)
    {
        result = 10000;
    }
    else
    {
        int64_t B = 500ll;

        int64_t n = 10000ll * (recommended - limit);                // 0
        int64_t resulta = 10000ll * (recommended - creature_level); // 0
        resulta /= n;

        int64_t v6 = (item_level)*B;
        B = 10000 - v6;
        int64_t v8 = 10000 - (resulta * B);
        result = v8;
    }
    return result / 10000;
}

int32_t GameRule::GetItemRecommendModTable(int32_t item_rank)
{
    int32_t idx = item_rank;

    if (idx < 1)
        idx = 1;
    if (idx > 8)
        idx = 8;
    return _modtable[idx];
}

int32_t GameRule::GetRankLevel(int32_t rank)
{
    int32_t idx = rank;
    if (idx < 1)
        idx = 1;
    if (idx > 8)
        idx = 8;

    switch (idx)
    {
    case 1:
        return 0;
    case 2:
        return 20;
    case 3:
        return 50;
    case 4:
        return 80;
    case 5:
        return 100;
    case 6:
        return 120;
    case 7:
        return 150;
    case 8:
        return 180;
    default:
        return 0;
    }
}

int32_t GameRule::GetItemLevelLimit(int32_t item_rank)
{
    return GetRankLevel(item_rank);
}

int32_t GameRule::GetItemRecommendedLevel(int32_t item_rank, int32_t item_level)
{
    int32_t result;
    if (item_rank > 1)
    {
        result = (GetItemRecommendModTable(item_rank) * (item_level - 1)) + GetItemLevelLimit(item_rank);
    }
    else
    {
        result = 0;
    }
    return result;
}

float GameRule::GetPickableRange()
{
    return 20;
}

float GameRule::GetItemDropRate()
{
    return sWorld.getRate(RATES_ITEM_DROP);
}

float GameRule::GetGoldDropRate()
{
    return sWorld.getRate(RATES_GOLD_DROP);
}

float GameRule::GetEXPRate()
{
    return sWorld.getRate(RATES_EXP);
}

int32_t GameRule::GetChipLevelLimit(int32_t idx)
{
    return _chipLevelLimit[idx];
}

float GameRule::GetChaosDropRate()
{
    return sWorld.getRate(RATES_CHAOS_DROP);
}

int32_t GameRule::GetIntValueByRandomInt(double fValue)
{
    double result = fValue + 1;
    if (((uint32_t)rand32() % 100) / 100.0 + fValue >= fValue)
        result = fValue;
    return (int32_t)result;
}

int64_t GameRule::GetIntValueByRandomInt64(double fValue)
{
    double result = fValue + 1;
    if (((uint32_t)rand32() % 100) / 100.0 + fValue >= fValue)
        result = fValue;
    return (int64_t)result;
}

float GameRule::GetStaminaRatio(int32_t level)
{
    if (level < 1)
        level = 1;
    if (level > 300)
        level = 300;
    if (_staminaExpRate[level] == 0.0f)
    {
        double v2 = pow(level, 1.46) + (double)level * 2.4;
        _staminaExpRate[level] = (float)((pow(level, 2) * 0.1 + v2 + 2.0f) * 0.00055f);
    }
    return _staminaExpRate[level];
}

float GameRule::GetStaminaBonus()
{
    return sWorld.getRate(RATES_STAMINA_BONUS);
}

int32_t GameRule::GetLocalFlag()
{
    return sWorld.getIntConfig(CONFIG_LOCAL_FLAG);
}
