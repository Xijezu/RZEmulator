//
// Created by xijezu on 17.12.17.
//

#include "GameRule.h"

int GameRule::_modtable[8] = {0, 3, 3, 2, 2, 3, 2, 2};
float GameRule::_itemDropRate = 1.0f;
float GameRule::_GoldDropRate = 1.0f;

float GameRule::GetItemValue(float item_current_value, int item_rank_value, int creature_level, int item_rank, int item_level)
{
    float ilp = GetItemLevelPenalty(creature_level, item_rank, item_level);
    //-,*,+
    float v8  = (item_current_value - item_rank_value) * ilp;
    return v8 + item_rank_value;
    //float f1 = item_current_value * item_rank_value;
    //return f1 * ilp;
}

float GameRule::GetItemLevelPenalty(int creature_level, int item_rank, int item_level)
{
    float result      = 1;
    int   recommended = GetItemRecommendedLevel(item_rank, item_level);
    int   limit       = GetItemLevelLimit(item_rank);

    if (item_level == 1 || creature_level < limit || creature_level >= recommended) {
        result = 1;
    } else {
        float n = (float) (recommended - limit);
        result = (float) (recommended - creature_level);
        result *= n;
    }
    return result;
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

void GameRule::SetItemDropRate(float _itemDropRate)
{
    GameRule::_itemDropRate = _itemDropRate;
}

float GameRule::GetGoldDropRate()
{
    return _GoldDropRate;
}

void GameRule::SetGoldDropRate(float goldDropRate)
{
    _GoldDropRate = goldDropRate;
}
