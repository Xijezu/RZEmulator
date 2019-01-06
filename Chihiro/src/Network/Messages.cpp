/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "World.h"
#include "WorldSession.h"
#include "ObjectMgr.h"

void Messages::SendEXPMessage(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    TS_SC_EXP_UPDATE packet;
    packet.handle = pUnit->GetHandle();
    packet.exp = static_cast<uint64_t>(pUnit->GetEXP());
    packet.jp = static_cast<uint64_t>(pUnit->GetJP());
    pPlayer->SendPacket(packet);
}

void Messages::SendLevelMessage(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    TS_SC_LEVEL_UPDATE resultPct;
    resultPct.handle = pUnit->GetHandle();
    resultPct.level = pUnit->GetLevel();
    resultPct.job_level = pUnit->GetCurrentJLv();
    pPlayer->SendPacket(resultPct);
}

void Messages::SendHPMPMessage(Player *pPlayer, Unit *pUnit, int add_hp, float add_mp, bool display)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    TS_SC_HPMP statPct{};
    statPct.handle = pUnit->GetHandle();

    statPct.add_hp = add_hp;
    statPct.hp = pUnit->GetHealth();
    statPct.max_hp = pUnit->GetMaxHealth();

    statPct.add_mp = static_cast<int32_t>(add_mp);
    statPct.mp = pUnit->GetMana();
    statPct.max_mp = pUnit->GetMaxMana();

    statPct.need_to_display = static_cast<uint8_t>(display ? 1 : 0);
    ;

    pPlayer->SendPacket(statPct);
}

void Messages::SendStatInfo(Player *pPlayer, Unit *pUnit)
{
    if (pPlayer == nullptr || pUnit == nullptr)
        return;

    {
        TS_SC_STAT_INFO statPct{};
        statPct.handle = pUnit->GetHandle();
        pUnit->m_cStat.WriteToPacket(statPct);
        pUnit->m_Attribute.WriteToPacket(statPct);
        statPct.type = 0;
        pPlayer->SendPacket(statPct);
    }

    {
        TS_SC_STAT_INFO statPct{};
        statPct.handle = pUnit->GetHandle();
        pUnit->m_cStatByState.WriteToPacket(statPct);
        pUnit->m_AttributeByState.WriteToPacket(statPct);
        statPct.type = 1;
        pPlayer->SendPacket(statPct);
    }
}

void Messages::SendAddSummonMessage(Player *pPlayer, Summon *pSummon)
{
    if (pPlayer == nullptr || pSummon == nullptr)
        return;

    TS_SC_ADD_SUMMON_INFO summonPct{};
    summonPct.card_handle = pSummon->GetCardHandle();
    summonPct.summon_handle = pSummon->GetHandle();
    summonPct.name = pSummon->GetNameAsString();
    summonPct.code = pSummon->GetSummonCode();
    summonPct.level = pSummon->GetLevel();
    summonPct.sp = 1000; // @todo
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

    TS_EQUIP_SUMMON summonPct{};
    summonPct.open_dialog = static_cast<uint8_t>(bShowDialog ? 1 : 0);
    for (int i = 0; i < 6; i++)
    {
        if (pPlayer->m_aBindSummonCard[i] != nullptr)
            summonPct.card_handle[i] = pPlayer->m_aBindSummonCard[i]->m_nHandle;
        else
            summonPct.card_handle[i] = 0;
    }
    pPlayer->SendPacket(summonPct);
}

void Messages::SendPropertyMessage(Player *pPlayer, Unit *pUnit, const std::string &szKey, int64 nValue)
{
    TS_SC_PROPERTY propertyPct{};
    propertyPct.handle = pUnit->GetHandle();
    propertyPct.is_number = 1;
    propertyPct.name = szKey;
    propertyPct.value = nValue;
    pPlayer->SendPacket(propertyPct);
}

void Messages::SendPropertyMessage(Player *pPlayer, Unit *pUnit, const std::string &pszKey, const std::string &pszValue)
{
    TS_SC_PROPERTY propertyPct{};
    propertyPct.handle = pUnit->GetHandle();
    propertyPct.is_number = 0;
    propertyPct.name = pszKey;
    propertyPct.string_value = pszValue;
    pPlayer->SendPacket(propertyPct);
}

void Messages::SendDialogMessage(Player *pPlayer, uint32_t npc_handle, int type, const std::string &szTitle, const std::string &szText, const std::string &szMenu)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_DIALOG dialogPct{};
    dialogPct.type = type;
    dialogPct.npc_handle = npc_handle;
    dialogPct.title = szTitle;
    dialogPct.text = szText;
    dialogPct.menu = szMenu;
    pPlayer->SendPacket(dialogPct);
}

void Messages::SendSkillList(Player *pPlayer, Unit *pUnit, int skill_id)
{
    TS_SC_SKILL_LIST skillPct{};
    skillPct.target = pUnit->GetHandle();
    skillPct.modification_type = 0; // reset | modification_type ?
    if (skill_id < 0)
    {

        for (const auto &t : pUnit->m_vSkillList)
        {
            if (t->m_nSkillUID < 0)
                continue;
            TS_SKILL_INFO skill_info{};
            skill_info.skill_id = t->m_nSkillID;
            skill_info.base_skill_level = static_cast<decltype(skill_info.base_skill_level)>(pUnit->GetBaseSkillLevel(t->m_nSkillID));
            skill_info.current_skill_level = static_cast<decltype(skill_info.current_skill_level)>(pUnit->GetCurrentSkillLevel(t->m_nSkillID));
            skill_info.total_cool_time = static_cast<decltype(skill_info.total_cool_time)>(pUnit->GetTotalCoolTime(t->m_nSkillID));
            skill_info.remain_cool_time = pUnit->GetRemainCoolTime(t->m_nSkillID);
            skillPct.skills.emplace_back(skill_info);
        }
    }
    else
    {
        auto skill = pUnit->GetSkill(skill_id);
        if (skill == nullptr)
            return;
        TS_SKILL_INFO skill_info{};
        skill_info.skill_id = skill_id;
        skill_info.base_skill_level = static_cast<decltype(skill_info.base_skill_level)>(pUnit->GetBaseSkillLevel(skill_id));
        skill_info.current_skill_level = static_cast<decltype(skill_info.current_skill_level)>(pUnit->GetCurrentSkillLevel(skill_id));
        skill_info.total_cool_time = static_cast<decltype(skill_info.total_cool_time)>(pUnit->GetTotalCoolTime(skill_id));
        skill_info.remain_cool_time = pUnit->GetRemainCoolTime(skill_id);
        skillPct.skills.emplace_back(skill_info);
    }
    pPlayer->SendPacket(skillPct);
}

void Messages::SendChatMessage(int nChatType, const std::string &szSenderName, Player *target, const std::string &szMsg)
{
    if (target == nullptr)
        return;

    if (szMsg.length() <= 30000)
    {
        TS_SC_CHAT chatPct{};
        chatPct.szSender = szSenderName;
        chatPct.type = static_cast<uint8_t>(nChatType);
        chatPct.message = szMsg;
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

    TS_SC_MARKET marketPct{};
    pPlayer->SetLastContact("market", pMarket[0].name);

    marketPct.npc_handle = npc_handle;
    for (const auto &info : pMarket)
    {
        TS_MARKET_ITEM_INFO item_info{};
        item_info.code = info.code;
        item_info.arena_point = 0; // @ Epic 9
        item_info.huntaholic_point = static_cast<decltype(item_info.huntaholic_point)>(info.huntaholic_ratio);
        item_info.price = static_cast<decltype(item_info.price)>(info.price_ratio);
        marketPct.items.emplace_back(item_info);
    }

    pPlayer->SendPacket(marketPct);
}

void Messages::SendItemMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    TS_SC_INVENTORY inventoryPct{};
    auto info = fillItemInfo(pItem);
    if (info.has_value())
        inventoryPct.items.emplace_back(info.value());
    pPlayer->SendPacket(inventoryPct);
}

std::optional<TS_ITEM_INFO> Messages::fillItemInfo(Item *item)
{
    if (item == nullptr || item->m_pItemBase == nullptr)
        return {};

    TS_ITEM_INFO itemInfo{};
    itemInfo.base_info.handle = item->m_nHandle;
    itemInfo.base_info.code = item->m_Instance.Code;
    itemInfo.base_info.uid = item->m_Instance.UID;
    itemInfo.base_info.count = item->m_Instance.nCount;

    itemInfo.base_info.endurance = static_cast<uint32_t>(item->m_Instance.nCurrentEndurance);
    itemInfo.base_info.enhance = static_cast<uint8_t>(item->m_Instance.nEnhance);
    itemInfo.base_info.level = static_cast<uint8_t>(item->m_Instance.nLevel);
    itemInfo.base_info.flag = static_cast<uint32_t>(item->m_Instance.Flag);

    std::copy(std::begin(item->m_Instance.Socket), std::end(item->m_Instance.Socket), std::begin(itemInfo.base_info.socket));

    if (item->m_pItemBase->group == GROUP_SUMMONCARD)
    {
        if (item->m_pSummon != nullptr)
        {
            int slot = 1;
            int tl = item->m_pSummon->m_nTransform;
            while (slot < tl)
            {
                itemInfo.base_info.socket[slot] = item->m_pSummon->GetPrevJobLv(slot - 1);
                ++slot;
            }
            itemInfo.base_info.socket[slot] = item->m_pSummon->GetLevel();
        }
    }

    itemInfo.base_info.remain_time = static_cast<int32_t>(item->m_Instance.tExpire);

    if (item->IsInStorage())
        itemInfo.wear_position = -2;
    else
        itemInfo.wear_position = item->m_Instance.nWearInfo;

    itemInfo.own_summon_handle = item->m_Instance.nOwnSummonUID > 0 ? item->m_Instance.OwnSummonHandle : 0;
    itemInfo.index = item->m_Instance.nIdx;

    return itemInfo;
}

void Messages::SendTimeSynch(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    TS_TIMESYNC timesync{};
    timesync.time = sWorld.GetArTime();
    pPlayer->SendPacket(timesync);
}

void Messages::SendGameTime(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_GAME_TIME gtPct{};
    gtPct.game_time = static_cast<uint64_t>(time(nullptr));
    gtPct.t = sWorld.GetArTime();
    pPlayer->SendPacket(gtPct);
}

void Messages::SendItemList(Player *pPlayer, bool bIsStorage)
{
    Item *item{nullptr};
    if (pPlayer->GetItemCount() > 0)
    {
        int64 count = bIsStorage ? pPlayer->GetStorageItemCount() : pPlayer->GetItemCount();
        int64 idx = 0;
        if (count != 0)
        {
            do
            {
                TS_SC_INVENTORY inventoryPct{};
                auto lcnt = idx;
                int64 mcount = 200;
                if (count - idx <= 200)
                    mcount = count - idx;

                auto ltotal = idx + mcount;
                if (idx < ltotal)
                {
                    do
                    {
                        if (bIsStorage)
                            item = pPlayer->GetStorageItem((uint)lcnt);
                        else
                            item = pPlayer->GetItem((uint)lcnt);
                        auto info = fillItemInfo(item);
                        if (info.has_value())
                            inventoryPct.items.emplace_back(info.value());
                        ++lcnt;
                    } while (lcnt < ltotal);
                }
                pPlayer->SendPacket(inventoryPct);
                idx += 200;
            } while (idx < count);
        }
    }
}

void Messages::SendResult(Player *pPlayer, uint16 nMsg, uint16 nResult, uint32 nValue)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_RESULT resultPct{};
    resultPct.request_msg_id = nMsg;
    resultPct.result = nResult;
    resultPct.value = nValue;
    pPlayer->SendPacket(resultPct);
}

void Messages::SendResult(WorldSession *worldSession, uint16 nMsg, uint16 nResult, uint32 nValue)
{
    if (worldSession == nullptr)
        return;

    TS_SC_RESULT resultPct{};
    resultPct.request_msg_id = nMsg;
    resultPct.result = nResult;
    resultPct.value = nValue;
    worldSession->GetSocket()->SendPacket(resultPct);
}

void Messages::SendDropResult(Player *pPlayer, uint itemHandle, bool bIsSuccess)
{
    TS_SC_DROP_RESULT dropPct{};
    dropPct.item_handle = itemHandle;
    dropPct.isAccepted = static_cast<uint8_t>(bIsSuccess ? 1 : 0);
    pPlayer->SendPacket(dropPct);
}

void Messages::sendEnterMessage(Player *pPlayer, WorldObject *pObj, bool /* bAbsolute*/)
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
        TS_SC_MOVE movePct{};
        movePct.start_time = pUnit->lastStepTime;
        movePct.handle = pUnit->GetHandle();
        movePct.tlayer = pUnit->GetLayer();
        movePct.speed = pUnit->speed;
        for (const auto &pos : pUnit->ends)
        {
            MOVE_INFO move_info{};
            move_info.tx = pos.end.GetPositionX();
            move_info.ty = pos.end.GetPositionY();
            movePct.move_infos.emplace_back(move_info);
        }
        pPlayer->SendPacket(movePct);
    }
}

void Messages::SendWearInfo(Player *pPlayer, Unit *pUnit)
{
    TS_SC_WEAR_INFO wearPct{};
    wearPct.handle = pUnit->GetHandle();
    for (int i = 0; i < MAX_ITEM_WEAR; i++)
    {
        int wear_info = (pUnit->m_anWear[i] != nullptr ? pUnit->m_anWear[i]->m_Instance.Code : 0);
        if (i == 2 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 2);
        if (i == 4 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 3);
        if (i == 5 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 4);
        wearPct.item_code[i] = static_cast<uint32_t>(wear_info);
        wearPct.item_enhance[i] = pUnit->m_anWear[i] != nullptr ? pUnit->m_anWear[i]->m_Instance.nEnhance : 0;
        wearPct.item_level[i] = pUnit->m_anWear[i] != nullptr ? pUnit->m_anWear[i]->m_Instance.nLevel : 0;
    }
    pPlayer->SendPacket(wearPct);
}

void Messages::BroadcastHPMPMessage(Unit *pUnit, int add_hp, float add_mp, bool need_to_display)
{
    TS_SC_HPMP hpmpPct{};
    hpmpPct.handle = pUnit->GetHandle();
    hpmpPct.add_hp = add_hp;
    hpmpPct.hp = pUnit->GetHealth();
    hpmpPct.max_hp = pUnit->GetMaxHealth();
    hpmpPct.add_mp = add_mp;
    hpmpPct.mp = pUnit->GetMana();
    hpmpPct.max_mp = pUnit->GetMaxMana();
    hpmpPct.need_to_display = static_cast<uint8_t>(need_to_display ? 1 : 0);

    sWorld.Broadcast((uint)(pUnit->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(pUnit->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pUnit->GetLayer(), hpmpPct);
}

void Messages::BroadcastLevelMsg(Unit *pUnit)
{
    TS_SC_LEVEL_UPDATE levelPct{};
    levelPct.handle = pUnit->GetHandle();
    levelPct.level = pUnit->GetLevel();
    levelPct.job_level = pUnit->GetCurrentJLv();
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
    TS_SC_WARP warpPct{};
    warpPct.x = pPlayer->GetPositionX();
    warpPct.y = pPlayer->GetPositionY();
    warpPct.z = pPlayer->GetPositionZ();
    warpPct.layer = pPlayer->GetLayer();
    pPlayer->SendPacket(warpPct);
}

void Messages::SendItemCountMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    TS_SC_UPDATE_ITEM_COUNT itemPct{};
    itemPct.item_handle = pItem->GetHandle();
    itemPct.count = pItem->m_Instance.nCount;
    pPlayer->SendPacket(itemPct);
}

void Messages::SendItemDestroyMessage(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr)
        return;

    TS_SC_DESTROY_ITEM itemPct{};
    itemPct.item_handle = pItem->GetHandle();
    pPlayer->SendPacket(itemPct);
}

void Messages::SendSkillCastFailMessage(Player *pPlayer, uint caster, uint target, uint16 skill_id, uint8 skill_level, Position pos, int error_code)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_SKILL skillPct{};
    skillPct.skill_id = skill_id;
    skillPct.skill_level = skill_level;
    skillPct.caster = caster;
    skillPct.target = target;
    skillPct.x = pos.GetPositionX();
    skillPct.y = pos.GetPositionY();
    skillPct.z = pos.GetPositionZ();
    skillPct.layer = pos.GetLayer();
    skillPct.type = ST_Casting;
    skillPct.casting.nErrorCode = static_cast<uint16_t>(error_code);

    pPlayer->SendPacket(skillPct);
}

void Messages::SendCantAttackMessage(Player *pPlayer, uint handle, uint target, int reason)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_CANT_ATTACK atkPct{};
    atkPct.attacker_handle = handle;
    atkPct.target_handle = target;
    atkPct.reason = reason;
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
        if (player->IsSitDown())
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
    std::string strButton{};
    std::string strTrigger{};
    int i = 0;
    int type = 3;

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
        Quest *q = pPlayer->FindQuest(code);
        int textID = text;
        if (textID == 0)
            textID = npc->GetQuestTextID(code, progress);
        if (npc == nullptr)
        {
            if (q->m_QuestBase->nEndType != 1 || progress != 2)
            {
                type = 7;
                progress = 1;
            }
            else
            {
                QuestLink *l = sObjectMgr.GetQuestLink(code, q->m_Instance.nStartID);
                if (l != nullptr && l->nEndTextID != 0)
                    textID = l->nEndTextID;
                type = 8;
            }
        }

        // /run function get_quest_progress() return 0 end
#if EPIC >= EPIC_5_1
        pPlayer->SetDialogTitle("Guide Arocel", type);
#else
        pPlayer->SetDialogTitle("Guide Arocel", 0);
#endif
        pPlayer->SetDialogText(NGemity::StringFormat("QUEST|{}|{}", code, textID));

        auto rQuestBase = sObjectMgr.GetQuestBase(code);

        if (progress != 0)
        {
            if (pPlayer->IsFinishableQuest(code))
            {
                for (i = 0; i < MAX_OPTIONAL_REWARD; i++)
                {
                    if (rQuestBase->OptionalReward[i].nItemCode == 0)
                        break;

                    strTrigger = NGemity::StringFormat("end_quest( {}, {} )", code, i);
                    pPlayer->AddDialogMenu("NULL", strTrigger);
                }
                if (i != 0)
                {
                    strButton = "REWARD";
                    strTrigger = std::to_string(rQuestBase->nCode);
                }
                else
                {
#if EPIC <= EPIC_4_1_1
                    ///- Hack for epic 4, use proper workaround instead
                    pPlayer->AddDialogMenu("Confirm", NGemity::StringFormat("end_quest({}, -1)", code));
                    return;
#else
                    strTrigger = NGemity::StringFormat("end_quest( {}, -1 )", code);
                    pPlayer->AddDialogMenu("NULL", strTrigger);
                    strButton = "REWARD";
                    strTrigger = std::to_string(rQuestBase->nCode);
#endif
                }
            }
            else
            {
                strButton = "OK";
                strTrigger = "";
            }
        }
        else
        {
            // /run function get_quest_progress() return 0 end
            strTrigger = NGemity::StringFormat("start_quest( {}, {} )", code, textID);
            pPlayer->AddDialogMenu("START", strTrigger);
            strTrigger = "";
            strButton = "REJECT";
        }
        pPlayer->AddDialogMenu(strButton, strTrigger);
    }
}

void Messages::SendQuestList(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_QUEST_LIST questPct{};

    /* FUNCTOR BEGIN*/
    auto functor = [&questPct](Quest *pQuest) -> void {
        TS_QUEST_INFO info{};
        info.code = static_cast<uint32_t>(pQuest->m_Instance.Code);
        info.startID = static_cast<uint32_t>(pQuest->m_Instance.nStartID);

        if (Quest::IsRandomQuest(pQuest->m_Instance.Code))
        {
            for (int i = 0, j = 0; i < MAX_VALUE_NUMBER / 4; i++)
            {
                info.value[j++] = static_cast<uint32_t>(pQuest->GetRandomKey(i));
                info.value[j++] = static_cast<uint32_t>(pQuest->GetRandomValue(i));
            }
        }
        else
        {
            for (int i = 0; i < MAX_VALUE_NUMBER / 2; ++i)
            {
                info.value[i] = static_cast<uint32_t>(pQuest->m_QuestBase->nValue[i]);
            }
        }

        for (int i = 0; i < MAX_QUEST_STATUS; i++)
        {
            info.status[i] = static_cast<uint32_t>(pQuest->m_Instance.nStatus[i]);
        }
        questPct.activeQuests.emplace_back(info);
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
            TS_SC_STATUS_CHANGE statusMsg{};
            statusMsg.handle = obj->GetHandle();
            statusMsg.status = Messages::GetStatusCode(obj, pObject->As<Player>());
            pObject->As<Player>()->SendPacket(statusMsg);
        }
    };

    sRegion.DoEachVisibleRegion((uint)(obj->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                (uint)(obj->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                obj->GetLayer(),
                                functor,
                                (uint8_t)RegionVisitor::ClientVisitor);
}

void Messages::BroadcastStateMessage(Unit *pUnit, State *pState, bool bIsCancel)
{
    TS_SC_STATE statePct{};
    statePct.handle = pUnit->GetHandle();
    statePct.state_handle = pState->GetUID();
    statePct.state_code = pState->GetCode();

    if (!bIsCancel)
    {
        statePct.state_level = pState->GetLevel();

        if (pState->IsAura())
        {
            statePct.end_time = -1;
        }
        else
        {
            statePct.end_time = pState->GetEndTime();
        }
        statePct.start_time = pState->GetStartTime();
    }

    statePct.state_value = pState->m_nStateValue;
    statePct.state_string_value = pState->m_szStateValue;

    sWorld.Broadcast(pUnit->GetRX(), pUnit->GetRY(), pUnit->GetLayer(), statePct);
}

void Messages::BroadcastTamingMessage(Player *pPlayer, Monster *pMonster, int mode)
{
    if (pPlayer == nullptr || pMonster == nullptr)
        return;

    TS_SC_TAMING_INFO tamePct{};
    tamePct.mode = mode;
    tamePct.tamer_handle = pPlayer->GetHandle();
    tamePct.target_handle = pMonster->GetHandle();
    sWorld.Broadcast((uint)(pMonster->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                     (uint)(pMonster->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pMonster->GetLayer(), tamePct);

    std::string chatMsg{};
    switch (mode)
    {
    case 0:
        chatMsg = NGemity::StringFormat("TAMING_START|{}|", pMonster->GetNameAsString());
        break;

    case 1:
    case 3:
        chatMsg = NGemity::StringFormat("TAMING_FAILED|{}|", pMonster->GetNameAsString());
        break;
    case 2:
        chatMsg = NGemity::StringFormat("TAMING_SUCCESS|{}|", pMonster->GetNameAsString());
        break;
    default:
        return;
    }

    SendChatMessage(100, "@SYSTEM", pPlayer, chatMsg);
}

void Messages::SendGlobalChatMessage(int chatType, const std::string &szSenderName, const std::string &szString, uint len)
{
    TS_SC_CHAT chatPct{};
    chatPct.szSender = szSenderName;
    chatPct.type = chatType;
    chatPct.message = szString;

    Player::DoEachPlayer([=](Player *pPlayer) {
        pPlayer->SendPacket(chatPct);
    });
    auto sender = Player::FindPlayer(szSenderName);
    if (sender != nullptr)
        Messages::SendResult(sender, NGemity::Packets::TS_CS_CHAT_REQUEST, TS_RESULT_SUCCESS, 0);
}

void Messages::SendLocalChatMessage(int nChatType, uint handle, const std::string &szMessage, uint len)
{
    auto p = sMemoryPool.GetObjectInWorld<Player>(handle);
    if (p != nullptr)
    {
        TS_SC_CHAT_LOCAL chatPct{};
        chatPct.handle = handle;
        chatPct.message = szMessage;
        chatPct.type = static_cast<uint8_t>(nChatType);
        ;
        sWorld.Broadcast((uint)(p->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                         (uint)(p->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), p->GetLayer(), chatPct);
        Messages::SendResult(p, NGemity::Packets::TS_CS_CHAT_REQUEST, TS_RESULT_SUCCESS, 0);
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
                TS_SC_STATUS_CHANGE statusMsg{};
                statusMsg.handle = obj->GetHandle();
                statusMsg.status = Messages::GetStatusCode(obj, pPlayer);
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
    TS_SC_QUEST_STATUS questPct{};
    questPct.code = pQuest->m_Instance.Code;
    for (int i = 0; i < MAX_QUEST_STATUS; i++)
    {
        questPct.status[i] = pQuest->m_Instance.nStatus[i];
    }
    pPlayer->SendPacket(questPct);
}

void Messages::SendItemCoolTimeInfo(Player *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    uint ct = sWorld.GetArTime();
    TS_SC_ITEM_COOL_TIME coolTimePct{};

    for (int i = 0; i < MAX_ITEM_COOLTIME_GROUP; i++)
    {
        int cool_time = pPlayer->m_nItemCooltime[i] - ct;
        if (cool_time < 0)
            cool_time = 0;
        coolTimePct.cool_time[i] = static_cast<uint32_t>(cool_time);
        ;
    }
    pPlayer->SendPacket(coolTimePct);
}

void Messages::SendMixResult(Player *pPlayer, std::vector<uint> *pHandles)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_MIX_RESULT mixPct{};
    if (pHandles != nullptr && !pHandles->empty())
    {
        for (const auto &pHandle : *pHandles)
        {
            mixPct.handles.emplace_back(pHandle);
        }
    }

    pPlayer->SendPacket(mixPct);
}

void Messages::SendItemWearInfoMessage(Player *pPlayer, Unit *pTarget, Item *pItem)
{
    TS_SC_ITEM_WEAR_INFO wearPct{};
    wearPct.target_handle = (pTarget != nullptr ? pTarget->GetHandle() : 0);
    wearPct.item_handle = pItem->GetHandle();
    wearPct.wear_position = pItem->m_Instance.nWearInfo;
    wearPct.enhance = pItem->m_Instance.nEnhance;

    pPlayer->SendPacket(wearPct);
}

void Messages::ShowSoulStoneRepairWindow(Player *pPlayer)
{
    TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW soulPct{};
    pPlayer->SetLastContact("RepairSoulStone", 1);
    pPlayer->SendPacket(soulPct);
}

void Messages::ShowSoulStoneCraftWindow(Player *pPlayer)
{
    TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW soulPct{};
    pPlayer->SetLastContact("SoulStoneCraft", 1);
    pPlayer->SendPacket(soulPct);
}

void Messages::SendPartyInfo(Player *pPlayer)
{
    if (pPlayer == nullptr || pPlayer->GetPartyID() == 0)
        return;

    auto leader = sGroupManager.GetLeaderName(pPlayer->GetPartyID());
    auto name = sGroupManager.GetPartyName(pPlayer->GetPartyID());
    int min_lvl = sGroupManager.GetMinLevel(pPlayer->GetPartyID());
    int max_lvl = sGroupManager.GetMaxLevel(pPlayer->GetPartyID());
    int share_mode = sGroupManager.GetShareMode(pPlayer->GetPartyID());

    std::string msg = NGemity::StringFormat("PINFO|{}|{}|{}|{}|{}|", name, leader, share_mode, min_lvl, max_lvl);

    struct PInfo
    {
        uint handle;
        int hp;
        int mp;
        int x;
        int y;
        int race;
        int isOnline;
    };

    sGroupManager.DoEachMemberTag(pPlayer->GetPartyID(), [&msg](PartyMemberTag &tag) {
        PInfo info{};
        auto player = Player::FindPlayer(tag.strName);
        if (player != nullptr)
        {
            info.handle = player->GetHandle();
            info.hp = (int)GetPct((float)player->GetHealth(), player->GetMaxHealth());
            info.mp = (int)GetPct((float)player->GetMana(), player->GetMaxMana());
            info.x = (int)player->GetPositionX();
            info.y = (int)player->GetPositionY();
            info.race = player->GetRace();
            info.isOnline = 2;
        }
        msg.append(NGemity::StringFormat("{}|{}|{}|{}|{}|{}|{}|{}|{}|", info.handle, tag.strName, info.race, tag.nJobID, info.hp, info.mp, info.x, info.y, info.isOnline));
    });
    SendChatMessage(100, "@PARTY", pPlayer, msg);
}

void Messages::SendRegionAckMessage(Player *pPlayer, uint rx, uint ry)
{
    if (pPlayer == nullptr)
        return;

    TS_SC_REGION_ACK ackPct{};
    ackPct.rx = rx;
    ackPct.ry = ry;
    pPlayer->SendPacket(ackPct);
}

void Messages::SendOpenStorageMessage(Player *pPlayer)
{
    TS_SC_OPEN_STORAGE storagePct{};
    // Some dirty hacks unknown to mankind to fill
    // this packet with various, godlike and important infos
    // jk, packet is empty
    pPlayer->SendPacket(storagePct);
}

void Messages::SendSkillCardInfo(Player *pPlayer, Item *pItem)
{
    TS_SC_SKILLCARD_INFO scInfo{};
    scInfo.item_handle = pItem->GetHandle();
    scInfo.target_handle = pItem->m_hBindedTarget;
    pPlayer->SendPacket(scInfo);
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

    TS_SC_AURA auraPct{};
    auraPct.caster = pUnit->GetHandle();
    auraPct.skill_id = static_cast<uint16_t>(skill_id);
    ;
    auraPct.status = static_cast<uint8_t>(status != 0 ? 1 : 0);
    player->SendPacket(auraPct);
}

void Messages::SendRemoveSummonMessage(Player *pPlayer, Summon *pSummon)
{
    if (pSummon == nullptr || pPlayer == nullptr)
        return;

    TS_SC_REMOVE_SUMMON_INFO removePct{};
    removePct.card_handle = pSummon->m_pItem->GetHandle();
    pPlayer->SendPacket(removePct);
}

void Messages::BroadcastPartyMemberInfo(Player *pClient)
{
    if (pClient == nullptr || pClient->GetPartyID() == 0)
        return;

    int partyID = pClient->GetPartyID();
    auto hp = (int)GetPct((float)pClient->GetHealth(), pClient->GetMaxHealth());
    auto mp = (int)GetPct((float)pClient->GetMana(), pClient->GetMaxMana());

    auto buf = NGemity::StringFormat("MINFO|{}|{}|{}|{}|{}|{}|{}|{}|{}|",
                                     pClient->GetHandle(), pClient->GetName(), pClient->GetRace(), pClient->GetCurrentJob(), hp, mp, pClient->GetPositionX(), pClient->GetPositionY(), 2);

    SendPartyChatMessage(100, "@PARTY", partyID, buf);
}

void Messages::BroadcastPartyLoginStatus(int nPartyID, bool bIsOnline, const std::string &szName)
{
    auto partyName = sGroupManager.GetPartyName(nPartyID);
    auto szMsg = bIsOnline ? NGemity::StringFormat("LOGIN|{}|{}|", partyName, szName) : NGemity::StringFormat("LOGOUT|{}|", szName);
    SendPartyChatMessage(100, "@PARTY", nPartyID, szMsg);
}

void Messages::SendTradeCancelMessage(Player *pClient)
{
    Player *tradeTarget = pClient->GetTradeTarget();
    if (tradeTarget == nullptr)
        return;

    TS_TRADE tradePct{};
    tradePct.target_player = tradeTarget->GetHandle();
    tradePct.mode = static_cast<uint8_t>(TM_CANCEL_TRADE);
    pClient->SendPacket(tradePct);
}

void Messages::SendTradeItemInfo(int32 nTradeMode, Item *pItem, int32 nCount, Player *pPlayer, Player *pTarget)
{
    TS_TRADE tradePct{};
    tradePct.target_player = pPlayer->GetHandle();
    tradePct.mode = static_cast<uint8_t>(nTradeMode);
    auto info = fillItemInfo(pItem);
    if (info.has_value())
        tradePct.item_info = info.value();

    pPlayer->SendPacket(tradePct);
    pTarget->SendPacket(tradePct);
}

void Messages::SendResult(Player *pPlayer, NGemity::Packets pPacketID, uint16 result, uint32 handle)
{
    Messages::SendResult(pPlayer, static_cast<uint16>(pPacketID), result, handle);
}

void Messages::SendResult(WorldSession *pPlayer, NGemity::Packets pPacketID, uint16 result, uint32 handle)
{
    Messages::SendResult(pPlayer, static_cast<uint16>(pPacketID), result, handle);
}

void Messages::SendStateMessage(Player *pPlayer, uint32_t handle, State *pState, bool bIsCancel)
{
    TS_SC_STATE stateMsg{};
    stateMsg.handle = handle;
    stateMsg.state_handle = pState->GetUID();
    stateMsg.state_code = pState->GetCode();

    stateMsg.state_value = pState->m_nStateValue;
    stateMsg.state_string_value = pState->m_szStateValue;

    if (!bIsCancel)
    {
        stateMsg.state_level = pState->GetLevel();
        if (pState->IsAura())
            stateMsg.end_time = -1;
        else
            stateMsg.end_time = pState->GetEndTime();

        stateMsg.start_time = pState->GetStartTime();
    }

    pPlayer->SendPacket(stateMsg);
}