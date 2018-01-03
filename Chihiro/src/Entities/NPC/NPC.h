#ifndef _NPC_H_
#define _NPC_H_

#include "Entities/Unit/Unit.h"
#include "Network/GameNetwork/WorldSession.h"

class NPC : public Unit {
public:
    // we can disable this warning for this since it only
    // causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

    NPC() : Unit(true)
    {
#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif
        _mainType = MT_NPC;
        _subType  = ST_NPC;
        _objType  = OBJ_MOVABLE;
        _valuesCount = UNIT_END;

        _InitValues();
        SetInt32Value(UNIT_FIELD_RACE, 0xac);
        SetHealth(100);
        SetMaxHealth(100);
    }

    ~NPC();

    void applyJobLevelBonus() override
    {};

    uint16_t putonItem(ItemWearType, Item *) override
    {};

    uint16_t putoffItem(ItemWearType) override
    {};

    static void EnterPacket(XPacket& pEnterPct, NPC* pNPC);

    NPCTemplate m_pBase;
private:
};

#endif // _NPC_H_
