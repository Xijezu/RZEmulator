#ifndef PROJECT_SUMMON_H
#define PROJECT_SUMMON_H

#include "Common.h"
#include "Unit.h"

class Player;

class Summon : public Unit {
public:
    static Summon *AllocSummon(Player *, uint);
    explicit Summon(uint, uint);
    ~Summon() = default;

    void OnAfterReadSummon();
    uint32_t GetCardHandle();
    int32_t GetSummonCode();

    void applyJobLevelBonus() override
    {};

    uint16_t putonItem(ItemWearType, Item *) override
    {};

    uint16_t putoffItem(ItemWearType) override
    {};

    int m_nSummonInfo{ };
    int m_nCardUID{ };
    int m_nTransform{ };
    Item* m_pItem;
    void SetSummonInfo(int);
    uint8_t m_cSlotIdx{};
private:
    SummonResourceTemplate m_tSummonBase;
    Player *m_pMaster;
};


#endif // PROJECT_SUMMON_H
