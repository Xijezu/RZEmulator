#include "NPC/NPC.h"
#include "XPacket.h"

// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

NPC::NPC() : Unit(true)
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

void NPC::EnterPacket(XPacket &pEnterPct, NPC *pNPC)
{
    Unit::EnterPacket(pEnterPct, pNPC);
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) HIWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) LOWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
}