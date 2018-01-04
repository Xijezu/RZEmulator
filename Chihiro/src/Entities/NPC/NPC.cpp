#include "NPC/NPC.h"
#include "XPacket.h"
#include "MemPool.h"

// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

NPC::NPC(NPCTemplate* base) : Unit(true)
{
#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif
    _mainType = MT_NPC;
    _subType  = ST_NPC;
    _objType  = OBJ_MOVABLE;
    _valuesCount = UNIT_END;
    m_pBase = base;

    _InitValues();

    sMemoryPool->AllocMiscHandle(this);
    SetUInt32Value(UNIT_FIELD_UID, m_pBase->id);
    SetInt32Value(UNIT_FIELD_RACE, 0xac);

    SetHealth(100);
    SetMaxHealth(100);
    Relocate(m_pBase->x, m_pBase->y, m_pBase->z);
}

void NPC::EnterPacket(XPacket &pEnterPct, NPC *pNPC)
{
    Unit::EnterPacket(pEnterPct, pNPC);
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) HIWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
    pEnterPct << (uint16) 0;
    pEnterPct << (uint16) LOWORD(pNPC->GetInt32Value(UNIT_FIELD_UID));
}

void NPC::LinkQuest(QuestLink *quest_link_info)
{
    if(quest_link_info->bLF_Start)
        m_vQuestLink_Start.emplace_back(quest_link_info);
    if(quest_link_info->bLF_Progress)
        m_vQuestLink_Progress.emplace_back(quest_link_info);
    if(quest_link_info->bLF_End)
        m_vQuestLink_End.emplace_back(quest_link_info);
}

NPCStatus NPC::GetStatus() const
{
    return (NPCStatus)m_nStatus;
}

void NPC::SetStatus(NPCStatus status)
{
    m_nStatus = status;
}

int NPC::GetNPCID() const
{
    return m_pBase->id;
}
