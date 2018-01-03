#include "NPC/NPC.h"
#include "XPacket.h"


NPC::~NPC()
{
}

void NPC::EnterPacket(XPacket &pEnterPct, NPC *pNPC)
{
    Unit::EnterPacket(pEnterPct, pNPC);
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) HIWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) LOWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
}