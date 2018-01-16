//
// Created by xijezu on 17.12.17.
//
#include "GameRule.h"
#include "Common.h"
#include "Util.h"

int GameRule::_modtable[8] = {0, 3, 3, 2, 2, 3, 2, 2};
int GameRule::_chipLevelLimit[8] = {0, 20, 50, 80, 100, 120, 150, 180};
float GameRule::_staminaExpRate[300] = { 0 };
float GameRule::_itemDropRate = 1.0f;
float GameRule::_GoldDropRate = 1.0f;
float GameRule::_chaosDropRate = 1.0f;
float GameRule::_expRate = 1.0f;

float GameRule::GetItemValue(float item_current_value, int item_rank_value, int creature_level, int item_rank, int item_level)
{
    float ilp = GetItemLevelPenalty(creature_level, item_rank, item_level);
    //-,*,+
    float v8  = (item_current_value - item_rank_value) * ilp;
    return v8 + item_rank_value;
    //float f1 = item_current_value * item_rank_value;
    //return f1 * ilp;
}

// NO TOUCH, IS WORK! NO TOUCH IF NO BROKE
float GameRule::GetItemLevelPenalty(int creature_level, int item_rank, int item_level)
{
    float result      = 0;
    int   recommended = GetItemRecommendedLevel(item_rank, item_level);
    int   limit       = GetItemLevelLimit(item_rank);

    if (item_level == 1 || creature_level < limit || creature_level >= recommended) {
        result = 10000;
    } else {
        int64 B = 500ll;

        int64 n       = 10000ll * (recommended - limit);                                        // 0
        int64 resulta = 10000ll * (recommended - creature_level);                        // 0
        resulta /= n;

        int64 v6 = (item_level) * B;
        B = 10000 - v6;
        int64 v8 = 10000 - (resulta * B);
        result = v8;
    }
    return result / 10000;
}

int GameRule::GetItemRecommendModTable(int item_rank)
{
    int idx = item_rank;

    if (idx < 1)
        idx = 1;
    if (idx > 8)
        idx = 8;
    return _modtable[idx];
}

int GameRule::GetRankLevel(int rank)
{
    int idx = rank;
    if (idx < 1)
        idx = 1;
    if (idx > 8)
        idx = 8;

    switch (idx) {
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

    return 0;
}

int GameRule::GetItemLevelLimit(int item_rank)
{
    return GetRankLevel(item_rank);
}

int GameRule::GetItemRecommendedLevel(int item_rank, int item_level)
{
    int result;
    if (item_rank > 1) {
        result = (GetItemRecommendModTable(item_rank) * (item_level - 1)) + GetItemLevelLimit(item_rank);
    } else {
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
    return _itemDropRate;
}

float GameRule::GetGoldDropRate()
{
    return _GoldDropRate;
}

float GameRule::GetEXPRate()
{
    return _expRate;
}

int GameRule::GetChipLevelLimit(int idx)
{
    return _chipLevelLimit[idx];
}

float GameRule::GetChaosDropRate()
{
    return _chaosDropRate;
}

int GameRule::GetIntValueByRandomInt(double fValue)
{
    double result = fValue +1;
    if(((uint)rand32() % 100) / 100.0 + fValue >= fValue)
        result = fValue;
    return (int)result;
}

int64 GameRule::GetIntValueByRandomInt64(double fValue)
{
    double result = fValue + 1;
    if (((uint)rand32() % 100) / 100.0 + fValue >= fValue)
        result = fValue;
    return (int64)result;
}

float GameRule::GetStaminaRatio(int level)
{
    if(level < 1)
        level = 1;
    if(level > 300)
        level = 300;
    if(_staminaExpRate[level] == 0.0f)
    {
        double v2 = pow(level, 1.46) + (double)level * 2.4;
        _staminaExpRate[level] = (float)((pow(level, 2) * 0.1 + v2 + 2.0f) * 0.00055f);
    }
    return _staminaExpRate[level];
}

float GameRule::GetStaminaBonus()
{
    return 1.0f;
}
