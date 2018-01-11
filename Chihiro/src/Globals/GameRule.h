//
// Created by xijezu on 17.12.17.
//

#ifndef PROJECT_GAMERULE_H
#define PROJECT_GAMERULE_H


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
    static float GetItemValue(float, int, int, int, int);
    static float GetPickableRange();
    static int GetChipLevelLimit(int idx);
    static int GetMaxLevel() { return 150; }
    static int GetIntValueByRandomInt(double fValue);
private:
    static int _chipLevelLimit[];
    static float _itemDropRate;
    static float _GoldDropRate;
    static float _chaosDropRate;
    static int _modtable[];
};


#endif //PROJECT_GAMERULE_H
