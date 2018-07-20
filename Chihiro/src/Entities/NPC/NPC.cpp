/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "NPC/NPC.h"
#include "XPacket.h"
#include "MemPool.h"
#include "ObjectMgr.h"
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

    sMemoryPool.AllocMiscHandle(this);
    SetUInt32Value(UNIT_FIELD_UID, m_pBase->id);
    SetInt32Value(UNIT_FIELD_RACE, 0xac);

    CalculateStat();

    SetHealth(GetMaxHealth());
    SetMana(GetMaxMana());
    Relocate(m_pBase->x, m_pBase->y, m_pBase->z);
}

void NPC::EnterPacket(XPacket &pEnterPct, NPC *pNPC, Player* pPlayer)
{
    Unit::EnterPacket(pEnterPct, pNPC, pPlayer);
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

NPC_STATUS NPC::GetStatus() const
{
    return (NPC_STATUS)m_nStatus;
}

void NPC::SetStatus(NPC_STATUS status)
{
    m_nStatus = status;
}

int NPC::GetNPCID() const
{
    return m_pBase->id;
}

bool NPC::HasStartableQuest(Player *player)
{
    bool bHasProgressRandom{false};
    bool isstart{false};

    for (auto &ql : m_vQuestLink_Start)
    {
        auto b = sObjectMgr.GetQuestBase(ql->code);
        if (b == nullptr)
            continue;
        auto qt = b->nType;
        if (qt == QuestType::QUEST_RANDOM_KILL_INDIVIDUAL || qt == QuestType::QUEST_RANDOM_COLLECT)
        {
            if (player->IsInProgressQuest(ql->code) || player->IsFinishableQuest(ql->code))
            {
                bHasProgressRandom = true;
            } else
            {
                if (player->IsStartableQuest(ql->code, true))
                    isstart = true;
            }
        } else
        {
            if (player->IsStartableQuest(ql->code, true))
                return true;
        }
    }
    return isstart && !bHasProgressRandom;

}

bool NPC::HasFinishableQuest(Player *player)
{
    for(auto& ql : m_vQuestLink_End) {
        if(player->IsFinishableQuest(ql->code))
            return true;
    }
    return false;
}

bool NPC::HasInProgressQuest(Player *player)
{
    for(auto& ql : m_vQuestLink_Progress) {
        if(player->IsInProgressQuest(ql->code))
            return true;
    }
    return false;
}

void NPC::DoEachStartableQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn)
{
    for(auto& ql : m_vQuestLink_Start) {
        if(pPlayer->IsStartableQuest(ql->code, false)) {
            fn(pPlayer, ql);
        }
    }
}

void NPC::DoEachInProgressQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn)
{
    for(auto& ql : m_vQuestLink_Progress) {
        if(pPlayer->IsInProgressQuest(ql->code)) {
            fn(pPlayer, ql);
        }
    }
}

void NPC::DoEachFinishableQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn)
{
    for(auto& ql : m_vQuestLink_End) {
        if(pPlayer->IsFinishableQuest(ql->code)) {
            fn(pPlayer, ql);
        }
    }
}

int NPC::GetQuestTextID(int code, int progress) const
{
    std::vector<QuestLink*> wpl{};
    if(progress == 2)
        wpl = m_vQuestLink_End;
    else if(progress == 1)
        wpl = m_vQuestLink_Progress;
    else
        wpl = m_vQuestLink_Start;

    for(auto& ql : wpl) {
        if(ql->code == code) {
            if(progress == 0)
                return ql->nStartTextID;
            if(progress == 1)
                return ql->nInProgressTextID;
            if(progress == 2)
                return ql->nEndTextID;
        }
    }
    return 0;
}

int NPC::GetProgressFromTextID(int code, int textId) const
{
    for(auto& ql : m_vQuestLink_Start) {
        if(ql->code == code) {
            if(textId == ql->nStartTextID)
                return 0;
            if(textId == ql->nInProgressTextID)
                return 1;
            if(textId == ql->nEndTextID)
                return 2;
        }
    }
    return -1;
}
