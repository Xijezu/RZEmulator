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
    static float GetItemValue(float, int, int, int, int);
private:
    static int _modtable[];
};


#endif //PROJECT_GAMERULE_H
