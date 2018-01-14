#ifndef PROJECT_SUMMON_H
#define PROJECT_SUMMON_H

#include "Common.h"
#include "Unit.h"

class Player;

class Summon : public Unit {
public:
    static Summon *AllocSummon(Player *, uint);
    explicit Summon(uint, uint);
    ~Summon();

    static void DB_InsertSummon(Player*,Summon*);
    static void DB_UpdateSummon(Player*,Summon*);
    static void EnterPacket(XPacket &, Summon *, Player* pPlayer);

    CreatureStat* GetBaseStat() const override;
    void OnAfterReadSummon();
    uint32_t GetCardHandle();
    int32_t GetSummonCode();
    Player* GetMaster() const
    { return m_pMaster; }

    void OnUpdate() override;

    bool TranslateWearPosition(ItemWearType& pos, Item* item, std::vector<int>& ItemList) override;

    int m_nSummonInfo{ };
    int m_nCardUID{ };
    int m_nTransform{ };
    Item* m_pItem{nullptr};
    void SetSummonInfo(int);
    bool DoEvolution();
    uint8_t m_cSlotIdx{};
protected:
    void processWalk(uint t);
    void onExpChange() override;
private:
    SummonResourceTemplate* m_tSummonBase{nullptr};
    Player *m_pMaster{nullptr};
};


#endif // PROJECT_SUMMON_H
