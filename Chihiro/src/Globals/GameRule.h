//
// Created by xijezu on 17.12.17.
//

#ifndef PROJECT_GAMERULE_H
#define PROJECT_GAMERULE_H

#include "Common.h"

class GameRule {
public:
    static float GetItemLevelPenalty(int creature_level, int item_rank, int item_level);
    static int GetItemRecommendedLevel(int item_rank, int item_level);
    static int GetItemLevelLimit(int item_rank);
    static int GetItemRecommendModTable(int item_rank);
    static int GetRankLevel(int rank);
    static float GetItemDropRate();
    static float GetGoldDropRate();
    static float GetChaosDropRate();
    static float GetEXPRate();
    static float GetItemValue(float, int, int, int, int);
    static float GetPickableRange();
    static int GetChipLevelLimit(int idx);
    static int GetMaxLevel() { return 150; }
    static int GetIntValueByRandomInt(double fValue);
    static int64 GetIntValueByRandomInt64(double fValue);
    static float GetStaminaRatio(int level);
    static float GetStaminaBonus();
private:
    static int _chipLevelLimit[];
    static float _itemDropRate;
    static float _expRate;
    static float _GoldDropRate;
    static float _chaosDropRate;
    static int _modtable[];
    static float _staminaExpRate[];
};


#endif //PROJECT_GAMERULE_H
