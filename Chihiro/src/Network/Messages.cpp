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

#include "Messages.h"
#include "ClientPackets.h"
#include "Skill.h"
#include "NPC.h"
#include "MemPool.h"
#include "RegionContainer.h"
#include "GroupManager.h"

void Messages::SendEXPMessage(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket resultPct(TS_SC_EXP_UPDATE);
    resultPct << (uint32_t)pUnit->GetHandle();
    resultPct << (int64)pUnit->GetEXP();
    resultPct << (uint32_t)pUnit->GetJP();
    pPlayer->SendPacket(resultPct);
}

void Messages::SendLevelMessage(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket resultPct(TS_SC_LEVEL_UPDATE);
    resultPct << (uint32_t)pUnit->GetHandle();
    resultPct << (uint32_t)pUnit->GetLevel();
    resultPct << pUnit->GetCurrentJLv();
    pPlayer->SendPacket(resultPct);
}

void Messages::SendHPMPMessage(Player *pPlayer, Unit *pUnit, int add_hp, float add_mp, bool display)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket statPct(TS_SC_HPMP);
    statPct << (uint32)pUnit->GetHandle();

    statPct << (int32)add_hp;
    statPct << (int32)pUnit->GetHealth();
    statPct << (int32)pUnit->GetMaxHealth();

    statPct << (int32)add_mp;
    statPct << (int32)pUnit->GetMana();
    statPct << (int32)pUnit->GetMaxMana();
    statPct << (uint8)(display ? 1 : 0);
    pPlayer->SendPacket(statPct);
}

void Messages::SendStatInfo(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket statPct(1000);
    statPct << (uint32_t)pUnit->GetHandle();
    pUnit->m_cStat.WriteToPacket(statPct);
    pUnit->m_Attribute.WriteToPacket(statPct);
    statPct << (uint8_t)0;
    pPlayer->SendPacket(statPct);

    statPct.Reset();
    statPct << (uint32_t)pUnit->GetHandle();
    pUnit->m_cStatByState.WriteToPacket(statPct);
    pUnit->m_AttributeByState.WriteToPacket(statPct);
    statPct << (uint8_t)1;
    pPlayer->SendPacket(statPct);
}

void Messages::SendAddSummonMessage(Player *pPlayer, Summon *pSummon)
{
    if (pPlayer == nullptr || pSummon == nullptr)
        return;

    XPacket summonPct(TS_SC_ADD_SUMMON_INFO);
    summonPct << (uint32_t)pSummon->GetCardHandle();
    summonPct << (uint32_t)pSummon->GetHandle();
    summonPct.fill(pSummon->GetName(), 19);
    summonPct << (int32_t)pSummon->GetSummonCode();
    summonPct << (int32_t)pSummon->GetLevel();
    summonPct << (int32_t)1000; // TODO: SP
    pPlayer->SendPacket(summonPct);

    SendStatInfo(pPlayer, pSummon);

    SendHPMPMessage(pPlayer, pSummon, pSummon->GetHealth(), pSummon->GetMana(), false);
    SendLevelMessage(pPlayer, pSummon);
    SendEXPMessage(pPlayer, pSummon);
    SendSkillList(pPlayer, pSummon, -1);
    // SendSPMessage(pPlayer, pSummon);
}

void Messages::SendCreatureEquipMessage(Player *pPlayer, bool bShowDialog)
{
    if (pPlayer == nullptr)
        return;

    XPacket summonPct(TS_EQUIP_SUMMON);
    summonPct << ((uint8_t)(bShowDialog ? 1 : 0));
    for (auto &i : pPlayer->m_aBindSummonCard)
    {
        if (i != nullptr)
        {
            summonPct << (uint32_t)i->m_nHandle;
        }
        else
        {
            summonPct << (uint32_t)0;
        }
    }
    pPlayer->SendPacket(summonPct);
}

void Messages::SendPropertyMessage(Player *pPlayer, Unit *pUnit, const std::string &szKey, int64 nValue)
{
    XPacket packet(TS_SC_PROPERTY);
    packet << (uint32_t)pUnit->GetHandle();
    packet << (uint8)1;
    packet.fill(szKey, 16);
    packet << (int64)nValue;
    packet.FinalizePacket();
    pPlayer->SendPacket(packet);
}

void Messages::SendPropertyMessage(Player *pPlayer, Unit *pUnit, const std::string &pszKey, const std::string &pszValue)
{
    XPacket packet(TS_SC_PROPERTY);
    packet << pUnit->GetHandle();
    packet << (uint8)0;
    packet.fill(pszKey, 16);
#if EPIC > 4
    packet << (uint64) 0;
#else
    packet << (uint32)0;
#endif
    packet << pszValue;
    packet << (uint8)0;
    pPlayer->SendPacket(packet);
}

void Messages::SendDialogMessage(Player *pPlayer, uint32_t npc_handle, int type, const std::string &szTitle, const std::string &szText, const std::string &szMenu)
{
    if (pPlayer == nullptr)
        return;

    XPacket dialogPct(TS_SC_DIALOG);
    dialogPct << type;
    dialogPct << npc_handle;
    dialogPct << (int16_t)szTitle.length();
    dialogPct << (int16_t)szText.length();
    dialogPct << (int16_t)szMenu.length();
    dialogPct.WriteString(szTitle);
    dialogPct.WriteString(szText);
    dialogPct.WriteString(szMenu);
    pPlayer->SendPacket(dialogPct);
}

void Messages::SendSkillList(Player *pPlayer, Unit *pUnit, int skill_id)
{
    XPacket skillPct(TS_SC_SKILL_LIST);
    skillPct << (uint32_t)pUnit->GetHandle();
    if (skill_id < 0)
    {
        skillPct << (uint16_t)pUnit->m_vSkillList.size();
        skillPct << (uint8_t)0; // reset | modification_type ?

        for (auto& t : pUnit->m_vSkillList)
        {
            if (t->m_nSkillUID < 0)
                continue;
            skillPct << (int32_t)t->m_nSkillID;
            skillPct << (int8_t)pUnit->GetBaseSkillLevel(t->m_nSkillID);
            skillPct << (int8_t)pUnit->GetCurrentSkillLevel(t->m_nSkillID);
            skillPct << (uint32_t)pUnit->GetTotalCoolTime(t->m_nSkillID);
            skillPct << (uint32_t)pUnit->GetRemainCoolTime(t->m_nSkillID);
        }
    }
    else
    {
        auto skill = pUnit->GetSkill(skill_id);
        if (skill == nullptr)
            return;
        skillPct << (ushort)1; // Size
        skillPct << (uint8_t)0; // reset | modification_type?
        skillPct << skill_id;
        skillPct << (int8_t)pUnit->GetBaseSkillLevel(skill_id);
        skillPct << (int8_t)pUnit->GetCurrentSkillLevel(skill_id);
        skillPct << (uint32_t)pUnit->GetTotalCoolTime(skill_id);
        skillPct << (uint32_t)pUnit->GetRemainCoolTime(skill_id);
    }
    pPlayer->SendPacket(skillPct);
}

void Messages::SendChatMessage(int nChatType, const std::string &szSenderName, Player *target, const std::string &szMsg)
{
    if (target == nullptr)
        return;

    if (szMsg.length() <= 30000)
    {
        XPacket chatPct(TS_SC_CHAT);
        chatPct.fill(szSenderName, 21);
        chatPct << (uint16_t)szMsg.length();
        chatPct << (uint8_t)nChatType;
        chatPct.fill(szMsg, szMsg.length() + 1);
        target->SendPacket(chatPct);
    }
}

void Messages::SendPartyChatMessage(int nChatType, const std::string &szSender, int nPartyID, const std::string &szMessage)
{
    sGroupManager.DoEachMemberTag(nPartyID, [=](PartyMemberTag &tag) {
        if (tag.bIsOnline && tag.pPlayer != nullptr)
        {
            Messages::SendChatMessage(nChatType, szSender, tag.pPlayer, szMessage);
        }
    });
}

void Messages::SendMarketInfo(Player *pPlayer, uint32_t npc_handle, const std::vector<MarketInfo> &pMarket)
{
    if (pPlayer == nullptr || pMarket.empty())
        return;

    XPacket marketPct(TS_SC_MARKET);
    pPlayer->SetLastContact("market", pMarket[0].name);

    marketPct << npc_handle;
    marketPct << (uint16_t)pMarket.size();
    for (const auto &info : pMarket)
    {
        marketPct << (int32_t)info.code;
#if EPIC >= 5
        marketPct << (int64_t) info.price_ratio;
        marketPct << (int32_t) 0;//info.huntaholic_ratio;
#else
	marketPct << (int64_t)info.price_ratio;
#endif // EPIC >= 4
    }

    pPlayer->SendPacket(marketPct);
}

void Messages::SendItemMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    XPacket inventoryPct(TS_SC_INVENTORY);
    inventoryPct << (uint16_t)1;
    fillItemInfo(inventoryPct, pItem);
    pPlayer->SendPacket(inventoryPct);
}

void Messages::fillItemInfo(XPacket &packet, Item *item)
{
    if (item == nullptr || item->m_pItemBase == nullptr)
        return;

    packet << (uint32_t)item->m_nHandle;
    packet << (int32_t)item->m_Instance.Code;
    packet << (int64)item->m_Instance.UID;
    packet << (int64) item->m_Instance.nCount;

    packet << (uint32_t)item->m_Instance.nCurrentEndurance;
    packet << (uint8_t)item->m_Instance.nEnhance;
    packet << (uint8_t)item->m_Instance.nLevel;
    packet << (uint32_t)item->m_Instance.Flag;

    int socket[4]{0};
    std::copy(std::begin(item->m_Instance.Socket), std::end(item->m_Instance.Socket), std::begin(socket));

    if (item->m_pItemBase->group == GROUP_SUMMONCARD)
    {
        if (item->m_pSummon != nullptr)
        {
            int slot     = 1;
            int tl       = item->m_pSummon->m_nTransform;
            while (slot < tl)
            {
                socket[slot] = item->m_pSummon->GetPrevJobLv(slot - 1);
                ++slot;
            }
            socket[slot] = item->m_pSummon->GetLevel();
        }
    }

    packet << (int32_t)socket[0];
    packet << (int32_t)socket[1];
    packet << (int32_t)socket[2];
    packet << (int32_t)socket[3];
    // Prior to Epic 6 we have to use 2 dummy socket slots.
    // Can you imagine how much time I wasted on this?
#if EPIC < 6
    packet << (int32_t)0;
    packet << (int32_t)0;
#endif // EPIC < 6
    packet << (int32_t)item->m_Instance.tExpire;

    if (item->IsInStorage())
        packet << (int16_t)-2;
    else
        packet << (int16_t)item->m_Instance.nWearInfo;
    packet << (uint32_t)(item->m_Instance.nOwnSummonUID > 0 ? item->m_Instance.OwnSummonHandle : 0);
#if EPIC >= 4
    packet << (int32_t) item->m_Instance.nIdx;
#endif // EPIC >= 5
}

void Messages::SendTimeSynch(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    XPacket result(TS_TIMESYNC);
    result << (uint32_t)sWorld.GetArTime();
    pPlayer->SendPacket(result);
}

void Messages::SendGameTime(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    XPacket gtPct(TS_SC_GAME_TIME);
    gtPct << (uint32)sWorld.GetArTime();
    gtPct << (int64)time(nullptr);
    pPlayer->SendPacket(gtPct);
}

void Messages::SendItemList(Player *pPlayer, bool bIsStorage)
{
    Item *item{nullptr};
    if (pPlayer->GetItemCount() > 0)
    {
        int64 count = bIsStorage ? pPlayer->GetStorageItemCount() : pPlayer->GetItemCount();
        int64 idx   = 0;
        if (count != 0)
        {
            do
            {
                XPacket packet(TS_SC_INVENTORY);
                auto    lcnt   = idx;
                int64   mcount = 200;
                if (count - idx <= 200)
                    mcount = count - idx;

                packet << (uint16_t)mcount;

                auto ltotal = idx + mcount;
                if (idx < ltotal)
                {
                    do
                    {
                        if (bIsStorage)
                            item = pPlayer->GetStorageItem((uint)lcnt);
                        else
                            item = pPlayer->GetItem((uint)lcnt);
                        fillItemInfo(packet, item);
                        ++lcnt;
                    } while (lcnt < ltotal);
                }
                pPlayer->SendPacket(packet);
                idx += 200;
            } while (idx < count);
        }
    }
}

void Messages::SendResult(Player *pPlayer, uint16 nMsg, uint16 nResult, uint32 nValue)
{
    if (pPlayer == nullptr)
        return;

    XPacket packet(CSPACKETS::TS_SC_RESULT);
    packet << nMsg;
    packet << nResult;
    packet << nValue;
    pPlayer->SendPacket(packet);
}

void Messages::SendResult(WorldSession *worldSession, uint16 nMsg, uint16 nResult, uint32 nValue)
{
    if (worldSession == nullptr)
        return;

    XPacket packet(CSPACKETS::TS_SC_RESULT);
    packet << nMsg;
    packet << nResult;
    packet << nValue;
    worldSession->GetSocket()->SendPacket(packet);
}

void Messages::SendDropResult(Player * pPlayer, uint itemHandle, bool bIsSuccess)
{
	XPacket packet(CSPACKETS::TS_SC_DROP_RESULT);
	packet << itemHandle;
	packet << bIsSuccess;
	pPlayer->SendPacket(packet);
}

void Messages::sendEnterMessage(Player *pPlayer, WorldObject *pObj, bool/* bAbsolute*/)
{
    if (pObj == nullptr || pPlayer == nullptr)
        return;

    if (pObj->IsMonster())
    {
        pObj->As<Monster>()->m_bNearClient = true;
    }

    pObj->SendEnterMsg(pPlayer);

    if (pObj->GetObjType() != 0 && pObj->bIsMoving && pObj->IsInWorld())
        SendMoveMessage(pPlayer, dynamic_cast<Unit *>(pObj));
}

void Messages::SendMoveMessage(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    if (pUnit->ends.size() < 2000)
    {
        XPacket movePct(TS_SC_MOVE);
        movePct << (uint32_t)pUnit->lastStepTime;
        movePct << (uint32)pUnit->GetHandle();
        movePct << (uint8_t)pUnit->GetLayer();
        movePct << (uint8_t)pUnit->speed;
        movePct << (uint16)pUnit->ends.size();
        for (auto &pos : pUnit->ends)
        {
            movePct << pos.end.GetPositionX();
            movePct << pos.end.GetPositionY();
        }
        pPlayer->SendPacket(movePct);
    }
}

void Messages::SendWearInfo(Player *pPlayer, Unit *pUnit)
{
    XPacket packet(TS_SC_WEAR_INFO);
    packet << pUnit->GetHandle();
    for (int  i = 0; i < MAX_ITEM_WEAR; i++)
    {
        int wear_info = (pUnit->m_anWear[i] != nullptr ? pUnit->m_anWear[i]->m_Instance.Code : 0);
        if (i == 2 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 2);
        if (i == 4 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 3);
        if (i == 5 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 4);
        packet << wear_info;
    }
    for (auto &i : pUnit->m_anWear)
    {
        packet << (i != nullptr ? i->m_Instance.nEnhance : 0);
    }
    for (auto &i : pUnit->m_anWear)
    {
        packet << (i != nullptr ? i->m_Instance.nLevel : 0);
    }
    packet.FinalizePacket();
    pPlayer->SendPacket(packet);
}

void Messages::BroadcastHPMPMessage(Unit *pUnit, int add_hp, float add_mp, bool need_to_display)
{
    XPacket hpmpPct(TS_SC_HPMP);
    hpmpPct << pUnit->GetHandle();
    hpmpPct << add_hp;
    hpmpPct << (int)pUnit->GetHealth();
    hpmpPct << (int)pUnit->GetMaxHealth();
    hpmpPct << (int)add_mp;
    hpmpPct << (int)pUnit->GetMana();
    hpmpPct << (int)pUnit->GetMaxMana();
    hpmpPct << (uint8_t)(need_to_display ? 1 : 0);
    sWorld.Broadcast((uint)(pUnit->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(pUnit->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pUnit->GetLayer(), hpmpPct);
}

void Messages::BroadcastLevelMsg(Unit *pUnit)
{
    XPacket levelPct(TS_SC_LEVEL_UPDATE);
    levelPct << (uint32)pUnit->GetHandle();
    levelPct << (int)pUnit->GetLevel();
    levelPct << (int)pUnit->GetCurrentJLv();
    sWorld.Broadcast((uint)(pUnit->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(pUnit->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pUnit->GetLayer(), levelPct);
}

void Messages::GetEncodedInt(XPacket &packet, uint32 nDecoded)
{
    packet << (int16)0;
    packet << (int16)HIWORD(nDecoded);
    packet << (int16)0;
    packet << (int16)LOWORD(nDecoded);
}

void Messages::SendWarpMessage(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;
    XPacket packet(CSPACKETS::TS_SC_WARP);
    packet << pPlayer->GetPositionX();
    packet << pPlayer->GetPositionY();
    packet << pPlayer->GetPositionZ();
    packet << (uint8_t)pPlayer->GetLayer();
    pPlayer->SendPacket(packet);
}

void Messages::SendItemCountMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    XPacket itemPct(TS_SC_UPDATE_ITEM_COUNT);
    itemPct << pItem->m_nHandle;
    itemPct << (uint64)pItem->m_Instance.nCount;
    pPlayer->SendPacket(itemPct);
}

void Messages::SendItemDestroyMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    XPacket itemPct(TS_SC_DESTROY_ITEM);
    itemPct << pItem->m_nHandle;

    pPlayer->SendPacket(itemPct);
}

void Messages::SendSkillCastFailMessage(Player *pPlayer, uint caster, uint target, uint16 skill_id, uint8 skill_level, Position pos, int error_code)
{
    if (pPlayer == nullptr)
        return;

    XPacket skillPct(TS_SC_SKILL);
    skillPct << skill_id;
    skillPct << skill_level;
    skillPct << caster;
    skillPct << target;
    skillPct << pos.GetPositionX();
    skillPct << pos.GetPositionY();
    skillPct << pos.GetPositionZ();
    skillPct << pos.GetLayer();
    skillPct << (uint8)1; // Type: Casting
    skillPct << (uint16)0; // costHP
    skillPct << (uint16)0; // costMP
    skillPct << (uint)0;  // Target HP
    skillPct << (uint16)0; // Target MP
    skillPct << (uint)0; // filler
    skillPct << (uint16)error_code;
    skillPct.fill("", 3);
    pPlayer->SendPacket(skillPct);
}

void Messages::SendCantAttackMessage(Player *pPlayer, uint handle, uint target, int reason)
{
    if (pPlayer == nullptr)
        return;

    XPacket atkPct(TS_SC_CANT_ATTACK);
    atkPct << handle;
    atkPct << target;
    atkPct << reason;
    pPlayer->SendPacket(atkPct);
}

uint Messages::GetStatusCode(WorldObject *pObj, Player *pClient)
{
    uint v2{0};

    switch (pObj->GetSubType())
    {
        case ST_NPC:
        {
            auto npc = dynamic_cast<NPC *>(pObj);
            if (npc->HasFinishableQuest(pClient))
                v2 |= 0x400;
            else if (npc->HasStartableQuest(pClient))
                v2 |= 0x100;
            else if (npc->HasInProgressQuest(pClient))
                v2 |= 0x200;
        }
            break;
        case ST_Mob:
        {
            auto monster = dynamic_cast<Monster *>(pObj);
            if (monster->GetStatus() == 4)
                v2 |= 0x100;
        }
            break;
        case ST_Player:
        {
            auto player = dynamic_cast<Player *>(pObj);
            if (player->IsSitdown())
                v2 |= 0x100;
            if (player->GetPermission() >= 100)
                v2 |= 0x4000;
        }
            break;
        default:
            break;
    }

    return v2;
}

void Messages::SendQuestInformation(Player *pPlayer, int code, int text, int ttype)
{
    //auto npc = dynamic_cast<NPC*>(sMemoryPool.getPtrFromId(pPlayer->GetLastContactLong("npc")));
    std::string strButton{ };
    std::string strTrigger{ };
    int         i    = 0;
    int         type = 3;

    auto npc = sMemoryPool.GetObjectInWorld<NPC>(pPlayer->GetLastContactLong("npc"));
    if (npc != nullptr)
    {
        int progress = 0;
        if (text != 0)
        {
            progress = npc->GetProgressFromTextID(code, text);
            if (progress == 1)
                type = 7;
            if (progress == 2)
                type = 8;
        }
        else
        {
            if (pPlayer->IsStartableQuest(code, false))
                progress = 0;
            else
                progress = pPlayer->IsFinishableQuest(code) ? 2 : 1;
        }
        Quest *q     = pPlayer->FindQuest(code);
        int   textID = text;
        if (textID == 0)
            textID = npc->GetQuestTextID(code, progress);
        if (npc == nullptr)
        {
            if (q->m_QuestBase->nEndType != 1 || progress != 2)
            {
                type     = 7;
                progress = 1;
            }
            else
            {
                QuestLink *l = sObjectMgr.GetQuestLink(code, q->m_Instance.nStartID);
                if (l != nullptr && l->nEndTextID != 0)
                    textID = l->nEndTextID;
                type       = 8;
            }
        }

        // /run function get_quest_progress() return 0 end
#if EPIC >= 5
        pPlayer->SetDialogTitle("Guide Arocel", type);
#else
        pPlayer->SetDialogTitle("Guide Arocel", 0);
#endif
        pPlayer->SetDialogText(string_format("QUEST|%u|%u", code, textID));

        auto rQuestBase = sObjectMgr.GetQuestBase(code);

        if (progress != 0)
        {
            if (pPlayer->IsFinishableQuest(code))
            {
                for (i = 0; i < MAX_OPTIONAL_REWARD; i++)
                {
                    if (rQuestBase->OptionalReward[i].nItemCode == 0)
                        break;

                    strTrigger = string_format("end_quest( %u, %u )", code, i);
                    pPlayer->AddDialogMenu("NULL", strTrigger);
                }
                if (i != 0)
                {
                    strButton  = "REWARD";
                    strTrigger = std::to_string(rQuestBase->nCode);
                }
                else
                {
#if EPIC <= 4
                    ///- Hack for epic 4, use proper workaround instead
                    pPlayer->AddDialogMenu("Confirm", string_format("end_quest(%u, -1)", code));
                    return;
#else
                    strTrigger = string_format("end_quest( %u, -1 )", code);
                    pPlayer->AddDialogMenu("NULL", strTrigger);
                    strButton  = "REWARD";
                    strTrigger = std::to_string(rQuestBase->nCode);
#endif
                }
            }
            else
            {
                strButton  = "OK";
                strTrigger = "";
            }
        }
        else
        {
            // /run function get_quest_progress() return 0 end
            strTrigger = string_format("start_quest( %u, %u )", code, textID);
            pPlayer->AddDialogMenu("START", strTrigger);
            strTrigger = "";
            strButton  = "REJECT";
        }
        pPlayer->AddDialogMenu(strButton, strTrigger);
    }
}

void Messages::SendQuestList(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    XPacket questPct(TS_SC_QUEST_LIST);
    questPct << (uint16)pPlayer->GetActiveQuestCount();

    /* FUNCTOR BEGIN*/
    auto functor = [&questPct](Quest *pQuest) -> void {
        questPct << pQuest->m_Instance.Code;
        questPct << pQuest->m_Instance.nStartID;
        if (Quest::IsRandomQuest(pQuest->m_Instance.Code))
        {
            questPct << pQuest->GetRandomKey(0);
            questPct << pQuest->GetRandomValue(0);
            questPct << pQuest->GetRandomKey(1);
            questPct << pQuest->GetRandomValue(1);
            questPct << pQuest->GetRandomKey(2);
            questPct << pQuest->GetRandomValue(2);
        }
        else
        {
            for (int i = 0; i < MAX_VALUE_NUMBER / 2; ++i)
            {
                questPct << pQuest->m_QuestBase->nValue[i];
            }
        }

        for (const auto &nStatu : pQuest->m_Instance.nStatus)
        {
            questPct << nStatu;
        }
    };
    /* FUNCTOR END*/
    pPlayer->DoEachActiveQuest(functor);
    pPlayer->SendPacket(questPct);
}

void Messages::BroadcastStatusMessage(WorldObject *obj)
{
    if (obj == nullptr)
        return;

    auto functor = [&obj](RegionType &list) -> void {
        for (auto &pObject : list)
        {
            XPacket statusMsg(TS_SC_STATUS_CHANGE);
            statusMsg << obj->GetHandle();
            statusMsg << Messages::GetStatusCode(obj, pObject->As<Player>());
            pObject->As<Player>()->SendPacket(statusMsg);
        }
    };

    sRegion.DoEachVisibleRegion((uint)(obj->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                (uint)(obj->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                obj->GetLayer(),
                                functor,
                                (uint8_t)RegionVisitor::ClientVisitor);
}

void Messages::BroadcastStateMessage(Unit *pUnit, State &pState, bool bIsCancel)
{
    XPacket statePct(TS_SC_STATE);
    statePct << pUnit->GetHandle();
    statePct << (uint16)pState.m_nUID;
    statePct << (int)pState.m_nCode;

    if (bIsCancel)
    {
        statePct << (uint16)0;
        statePct << (uint32)0;
        statePct << (uint32)0;
    }
    else
    {
        statePct << (uint16)pState.GetLevel();
        uint t{ };
        if (!pState.m_bAura)
        {
            t     = pState.m_nEndTime[0];
            if (t <= pState.m_nEndTime[1])
                t = pState.m_nEndTime[1];
            statePct << t;
        }
        else
        {
            statePct << -1;
        }

        t     = pState.m_nStartTime[1];
        if (pState.m_nStartTime[0] > t)
            t = pState.m_nStartTime[0];
        statePct << t;
    }

    statePct << pState.m_nStateValue;
    statePct.fill(pState.m_szStateValue, 32);

    sWorld.Broadcast((uint)(pUnit->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                     (uint)(pUnit->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pUnit->GetLayer(), statePct);
}

void Messages::BroadcastTamingMessage(Player *pPlayer, Monster *pMonster, int mode)
{
    if (pPlayer == nullptr || pMonster == nullptr)
        return;

    XPacket tamePct(TS_SC_TAMING_INFO);
    tamePct << (uint8)mode;
    tamePct << pPlayer->GetHandle();
    tamePct << pMonster->GetHandle();

    sWorld.Broadcast((uint)(pMonster->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                    (uint)(pMonster->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pMonster->GetLayer(), tamePct);

    std::string chatMsg{ };
    switch (mode)
    {
        case 0:
            chatMsg = string_format("TAMING_START|%s|", pMonster->GetNameAsString().c_str());
            break;

        case 1:
        case 3:
            chatMsg = string_format("TAMING_FAILED|%s|", pMonster->GetNameAsString().c_str());
            break;
        case 2:
            chatMsg = string_format("TAMING_SUCCESS|%s|", pMonster->GetNameAsString().c_str());
            break;
        default:
            return;
    }

    SendChatMessage(100, "@SYSTEM", pPlayer, chatMsg);
}

void Messages::SendGlobalChatMessage(int chatType, const std::string &szSenderName, const std::string &szString, uint len)
{
    XPacket chatPct(TS_SC_CHAT);
    chatPct.fill(szSenderName, 21);
    chatPct << (int16)szString.length();
    chatPct << (uint8)chatType;
    chatPct.fill(szString, szString.length() + 1);

    Player::DoEachPlayer([=](Player *pPlayer) {
        pPlayer->SendPacket(chatPct);
    });
    auto    sender = Player::FindPlayer(szSenderName);
    if (sender != nullptr)
        Messages::SendResult(sender, TS_CS_CHAT_REQUEST, TS_RESULT_SUCCESS, 0);
}

void Messages::SendLocalChatMessage(int nChatType, uint handle, const std::string &szMessage, uint len)
{
    auto p = sMemoryPool.GetObjectInWorld<Player>(handle);
    if (p != nullptr)
    {
        XPacket result(TS_SC_CHAT_LOCAL);
        result << (uint32)handle;
        result << (uint8)szMessage.length();
        result << (uint8)nChatType;
        result.WriteString(szMessage);
        result << (uint8)0;
        sWorld.Broadcast((uint)(p->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                         (uint)(p->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), p->GetLayer(), result);
        Messages::SendResult(p, TS_CS_CHAT_REQUEST, TS_RESULT_SUCCESS, 0);
    }
}

void Messages::SendQuestMessage(int nChatType, Player *pTarget, const std::string &szString)
{
    SendChatMessage(nChatType, "@QUEST", pTarget, szString);
}

void Messages::SendNPCStatusInVisibleRange(Player *pPlayer)
{
    auto functor = [&pPlayer](RegionType &regionType) -> void {
        for (const auto &obj : regionType)
        {
            if (obj != nullptr && obj->IsNPC())
            {
                XPacket statusMsg(TS_SC_STATUS_CHANGE);
                statusMsg << obj->GetHandle();
                statusMsg << Messages::GetStatusCode(obj, pPlayer);
                pPlayer->SendPacket(statusMsg);
            }
        }
    };

    sRegion.DoEachVisibleRegion((uint)(pPlayer->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                (uint)(pPlayer->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                pPlayer->GetLayer(),
                                functor,
                                (uint8_t)RegionVisitor::MovableVisitor);
}

void Messages::SendQuestStatus(Player *pPlayer, Quest *pQuest)
{
    XPacket questPct(TS_SC_QUEST_STATUS);
    questPct << pQuest->m_Instance.Code;
    for (int nStatu : pQuest->m_Instance.nStatus)
    {
        questPct << nStatu;
    }
    pPlayer->SendPacket(questPct);
}

void Messages::SendItemCoolTimeInfo(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    uint    ct = sWorld.GetArTime();
    XPacket coolTimePct(TS_SC_ITEM_COOL_TIME);

    for (unsigned int coolTime : pPlayer->m_nItemCooltime)
    {
        int cool_time = coolTime - ct;
        if (cool_time < 0)
            cool_time = 0;
        coolTimePct << cool_time;
    }
    pPlayer->SendPacket(coolTimePct);
}

void Messages::SendMixResult(Player *pPlayer, std::vector<uint> *pHandles)
{
    if (pPlayer == nullptr)
        return;

    XPacket mixPct(TS_SC_MIX_RESULT);
    mixPct << (uint)(pHandles != nullptr ? pHandles->size() : 0);
    if (pHandles != nullptr && !pHandles->empty())
    {
        for (unsigned int pHandle : *pHandles)
        {
            mixPct << pHandle;
        }
    }

    pPlayer->SendPacket(mixPct);
}

void Messages::SendItemWearInfoMessage(Player *pPlayer, Unit *pTarget, Item *pItem)
{
    XPacket packet(TS_SC_ITEM_WEAR_INFO);
    packet << (uint32_t)pItem->GetHandle();
    packet << (int16_t)pItem->m_Instance.nWearInfo;
    packet << (uint32_t)(pTarget != nullptr ? pTarget->GetHandle() : 0);
    packet << (int32_t)pItem->m_Instance.nEnhance;
    pPlayer->SendPacket(packet);
}

void Messages::ShowSoulStoneRepairWindow(Player *pPlayer)
{
    XPacket soulPct(TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW);
    pPlayer->SetLastContact("RepairSoulStone", 1);
    pPlayer->SendPacket(soulPct);
}

void Messages::ShowSoulStoneCraftWindow(Player *pPlayer)
{
    XPacket soulPct(TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW);
    pPlayer->SetLastContact("SoulStoneCraft", 1);
    pPlayer->SendPacket(soulPct);
}

void Messages::SendPartyInfo(Player *pPlayer)
{
    if (pPlayer == nullptr || pPlayer->GetPartyID() == 0)
        return;

    auto leader     = sGroupManager.GetLeaderName(pPlayer->GetPartyID());
    auto name       = sGroupManager.GetPartyName(pPlayer->GetPartyID());
    int  min_lvl    = sGroupManager.GetMinLevel(pPlayer->GetPartyID());
    int  max_lvl    = sGroupManager.GetMaxLevel(pPlayer->GetPartyID());
    int  share_mode = sGroupManager.GetShareMode(pPlayer->GetPartyID());

    std::string msg = string_format("PINFO|%s|%s|%d|%d|%d|", name.c_str(), leader.c_str(), share_mode, min_lvl, max_lvl);

    struct PInfo
    {
        uint handle;
        int  hp;
        int  mp;
        int  x;
        int  y;
        int  race;
        int  isOnline;
    };

    sGroupManager.DoEachMemberTag(pPlayer->GetPartyID(), [&msg](PartyMemberTag &tag) {
        PInfo info{ };
        auto  player = Player::FindPlayer(tag.strName);
        if (player != nullptr)
        {
            info.handle   = player->GetHandle();
            info.hp       = (int)GetPct((float)player->GetHealth(), player->GetMaxHealth());
            info.mp       = (int)GetPct((float)player->GetMana(), player->GetMaxMana());
            info.x        = (int)player->GetPositionX();
            info.y        = (int)player->GetPositionY();
            info.race     = player->GetRace();
            info.isOnline = 2;
        }
        msg.append(string_format("%d|%s|%d|%d|%d|%d|%d|%d|%d|", info.handle, tag.strName.c_str(), info.race, tag.nJobID, info.hp, info.mp, info.x, info.y, info.isOnline));
    });
    SendChatMessage(100, "@PARTY", pPlayer, msg);
}

void Messages::SendRegionAckMessage(Player *pPlayer, uint rx, uint ry)
{
    if (pPlayer == nullptr)
        return;

    XPacket ackPct(TS_SC_REGION_ACK);
    ackPct << rx;
    ackPct << ry;
    pPlayer->SendPacket(ackPct);
}

void Messages::SendOpenStorageMessage(Player *pPlayer)
{
    XPacket storagePct(TS_SC_OPEN_STORAGE);
    // Some dirty hacks unknown to mankind to fill
    // this packet with various, godlike and important infos
    // jk, packet is empty
    pPlayer->SendPacket(storagePct);
}

void Messages::SendSkillCardInfo(Player *pPlayer, Item *pItem)
{
    XPacket scPct(TS_SC_SKILLCARD_INFO);
    scPct << pItem->GetHandle();
    scPct << pItem->m_hBindedTarget;
    pPlayer->SendPacket(scPct);
}

void Messages::SendToggleInfo(Unit *pUnit, int skill_id, bool status)
{
    if (pUnit == nullptr)
        return;

    auto player = pUnit->As<Player>();
    if (player == nullptr && pUnit->IsSummon())
        player = pUnit->As<Summon>()->GetMaster();

    if (player == nullptr)
        return;

    XPacket auraPct(TS_SC_AURA);
    auraPct << pUnit->GetHandle();
    auraPct << (uint16)skill_id;
    auraPct << (uint8)(status != 0 ? 1 : 0);
    player->SendPacket(auraPct);
}

void Messages::SendRemoveSummonMessage(Player *pPlayer, Summon *pSummon)
{
    if (pSummon == nullptr || pPlayer == nullptr)
        return;

    XPacket removePct(TS_SC_REMOVE_SUMMON_INFO);
    removePct << pSummon->m_pItem->GetHandle();
    pPlayer->SendPacket(removePct);
}

void Messages::BroadcastPartyMemberInfo(Player *pClient)
{
    if (pClient == nullptr || pClient->GetPartyID() == 0)
        return;

    int  partyID = pClient->GetPartyID();
    auto hp      = (int)GetPct((float)pClient->GetHealth(), pClient->GetMaxHealth());
    auto mp      = (int)GetPct((float)pClient->GetMana(), pClient->GetMaxMana());

    auto buf = string_format("MINFO|%d|%s|%d|%d|%d|%d|%d|%d|%d|",
                             pClient->GetHandle(), pClient->GetName(), pClient->GetRace(), pClient->GetCurrentJob(), hp, mp, pClient->GetPositionX(), pClient->GetPositionY(), 2);

    SendPartyChatMessage(100, "@PARTY", partyID, buf);
}

void Messages::BroadcastPartyLoginStatus(int nPartyID, bool bIsOnline, const std::string &szName)
{
    auto partyName = sGroupManager.GetPartyName(nPartyID);
    auto szMsg     = bIsOnline ? string_format("LOGIN|%s|%s|", partyName.c_str(), szName.c_str()) : string_format("LOGOUT|%s|", szName.c_str());
    SendPartyChatMessage(100, "@PARTY", nPartyID, szMsg);
}

void Messages::SendTradeCancelMessage(Player *pClient)
{
    Player *tradeTarget = pClient->GetTradeTarget();
    if (tradeTarget == nullptr)
        return;

    XPacket tradePct(TS_TRADE);
    tradePct << (uint32)tradeTarget->GetHandle();
    tradePct << (uint8)TM_CANCEL_TRADE;
    pClient->SendPacket(tradePct);
}

void Messages::SendTradeItemInfo(int32 nTradeMode, Item *pItem, int32 nCount, Player *pPlayer, Player *pTarget)
{
    XPacket tradePct(TS_TRADE);
    tradePct << (uint32)pPlayer->GetHandle();
    tradePct << (uint8)nTradeMode;
    fillItemInfo(tradePct, pItem);

    // Change the count of the item to the trade count
    tradePct.wpos(28);
#if EPIC >= 5
    tradePct << (int64)nCount;
#else
    tradePct << (int32)nCount;
#endif
    pPlayer->SendPacket(tradePct);
    pTarget->SendPacket(tradePct);
}
