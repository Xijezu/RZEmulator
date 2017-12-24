/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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
#include "Player.h"
#include "ClientPackets.h"
#include "GameHandler.h"
#include "World.h"
#include "Skill.h"

void Messages::SendEXPMessage(Player *pPlayer, Unit *pUnit)
{
    if(pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket resultPct(TS_SC_EXP_UPDATE);
    resultPct << (uint32_t)pUnit->GetHandle();
    resultPct << (int64)pUnit->GetEXP();
    resultPct << (uint32_t)pUnit->GetJP();
    pPlayer->GetSession().GetSocket().SendPacket(resultPct);
}

void Messages::SendLevelMessage(Player *pPlayer, Unit *pUnit)
{
    if(pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket resultPct(TS_SC_LEVEL_UPDATE);
    resultPct << (uint32_t)pUnit->GetHandle();
    resultPct << (uint32_t)pUnit->getLevel();
    resultPct << pUnit->GetCurrentJLv();
    pPlayer->GetSession().GetSocket().SendPacket(resultPct);
}

void Messages::SendHPMPMessage(Player *pPlayer, Unit *pUnit, int add_hp, float add_mp, bool display)
{
    if(pPlayer == nullptr || pUnit == nullptr)
        return;

    XPacket statPct(TS_SC_HPMP);
    statPct << (uint32_t)pUnit->GetHandle();

    statPct << (int32_t)add_hp;
    statPct << (int32_t)pUnit->GetHealth();
    statPct << (int32_t)pUnit->GetMaxHealth();

    statPct << (int32_t)add_mp;
    statPct << (int32_t)pUnit->GetMana();
    statPct << (int32_t)pUnit->GetMaxMana();
    statPct << (uint8_t)(display ? 1 : 0);
    pPlayer->GetSession().GetSocket().SendPacket(statPct);
}

void Messages::SendStatInfo(Player *pPlayer, Unit *pUnit)
{
    if(pPlayer == nullptr || pUnit == nullptr)
        return;


    XPacket statPct(1000);
    statPct << (uint32_t) pUnit->GetHandle();
    pUnit->m_cStat.WriteToPacket(statPct);
    pUnit->m_cAtribute.WriteToPacket(statPct);
    statPct << (uint8_t) 0;
    pPlayer->GetSession().GetSocket().SendPacket(statPct);

    statPct.Reset();
    statPct << (uint32_t) pUnit->GetHandle();
    pUnit->m_cStatByState.WriteToPacket(statPct);
    pUnit->m_cAtributeByState.WriteToPacket(statPct);
    statPct << (uint8_t) 1;
    pPlayer->GetSession().GetSocket().SendPacket(statPct);
}

void Messages::SendAddSummonMessage(Player *pPlayer, Summon *pSummon)
{
    if(pPlayer == nullptr || pSummon == nullptr)
        return;

    XPacket summonPct(TS_SC_ADD_SUMMON_INFO);
    summonPct << (uint32_t)pSummon->GetCardHandle();
    summonPct << (uint32_t)pSummon->GetHandle();
    summonPct.fill(pSummon->GetName(), 19);
    summonPct << (int32_t)pSummon->GetSummonCode();
    summonPct << (int32_t)pSummon->getLevel();
    summonPct << (int32_t)1000; // TODO: SP
    pPlayer->GetSession().GetSocket().SendPacket(summonPct);

    SendStatInfo(pPlayer, pSummon);

    SendHPMPMessage(pPlayer, pSummon, pSummon->GetHealth(), pSummon->GetMana(), false);
    // Doesnt work or I'm too dumb

    //SendPropertyMessage(pPlayer, pSummon, "hp", 500);
    //SendPropertyMessage(pPlayer, pSummon, "mp", 600);
    //SendPropertyMessage(pPlayer, pSummon, "max_hp", 700);
    //SendPropertyMessage(pPlayer, pSummon, "max_mana", 800);
    //SendPropertyMessage(pPlayer, pSummon, "hp", (int64_t)pSummon->GetHealth());
    //SendPropertyMessage(pPlayer, pSummon, "max_hp", (int64_t)pSummon->GetMaxHealth());
    //SendPropertyMessage(pPlayer, pSummon, "mp", (int64_t)pSummon->GetMana());
    //SendPropertyMessage(pPlayer, pSummon, "max_mp", (int64_t)pSummon->GetMaxMana());

    SendLevelMessage(pPlayer, pSummon);
    SendEXPMessage(pPlayer, pSummon);
    // SendSPMessage(pPlayer, pSummon);
}

void Messages::SendCreatureEquipMessage(Player *pPlayer, bool bShowDialog)
{
    if(pPlayer == nullptr)
        return;

    XPacket summonPct(TS_EQUIP_SUMMON);
    summonPct << ((uint8_t)(bShowDialog ? 1 : 0));
    for (auto &i : pPlayer->m_aBindSummonCard) {
        if(i != nullptr) {
            summonPct << (uint32_t) i->m_nHandle;
        } else {
            summonPct << (uint32_t)0;
        }
    }
    pPlayer->GetSession().GetSocket().SendPacket(summonPct);
}

void Messages::SendPropertyMessage(Player *pPlayer, Unit *pUnit, std::string szKey, int64_t nValue)
{
        XPacket packet(TS_SC_PROPERTY);
        packet << (uint32_t)pUnit->GetHandle();
        packet << (uint8) 1;
        packet.fill(szKey, 16);
        packet << (int64)nValue;
        packet.FinalizePacket();
        pPlayer->GetSession().GetSocket().SendPacket(packet);
}

void Messages::SendDialogMessage(Player *pPlayer, uint32_t npc_handle, int type, std::string szTitle, std::string szText, std::string szMenu)
{
    if(pPlayer == nullptr)
        return;

    XPacket dialogPct(TS_SC_DIALOG);
    dialogPct << type;
    dialogPct << npc_handle;
    dialogPct << (int16_t)szTitle.length();
    dialogPct << (int16_t)szText.length();
    dialogPct << (int16_t)szMenu.length();
    dialogPct.fill(szTitle, 0);
    dialogPct.fill(szText,0);
    dialogPct.fill(szMenu,0);
    pPlayer->GetSession().GetSocket().SendPacket(dialogPct);
}

void Messages::SendSkillList(Player *pPlayer, Unit *pUnit, int skill_id)
{
    XPacket skillPct(TS_SC_SKILL_LIST);
    skillPct << (uint32_t)pUnit->GetHandle();
    if(skill_id == 0) {
        skillPct << (uint16_t) pUnit->m_vSkillList.size();
        skillPct << (uint8_t) 0; // reset | modification_type ?

        for (auto t : pUnit->m_vSkillList) {
            skillPct << (int32_t) t->skill_id;
            skillPct << (int8_t) pUnit->GetBaseSkillLevel(t->skill_level);
            skillPct << (int8_t) t->skill_level;
            skillPct << (uint32_t) 0;
            skillPct << (uint32_t) 0;
        }
    }else {
        auto skill = pUnit->GetSkill(skill_id);
        if(skill == nullptr)
            return;
        skillPct << (ushort)1; // Size
        skillPct << (uint8_t)0; // reset | modification_type?
        skillPct << (int)skill_id;
        skillPct << (int8_t) pUnit->GetBaseSkillLevel(skill->skill_level);
        skillPct << (int8_t)skill->skill_level;
        skillPct << (uint32_t) 0;
        skillPct << (uint32_t) 0;
    }
    pPlayer->GetSession().GetSocket().SendPacket(skillPct);
}

void Messages::SendChatMessage(int nChatType, std::string szSenderName, Player* target, std::string szMsg)
{
    if(target == nullptr)
        return;

    if(szMsg.length() <= 30000) {
        XPacket chatPct(TS_SC_CHAT);
        chatPct.fill(szSenderName, 21);
        chatPct << (uint16_t)szMsg.length();
        chatPct << (uint8_t)nChatType;
        chatPct.fill(szMsg, szMsg.length());
        target->GetSession().GetSocket().SendPacket(chatPct);
    }
}

void Messages::SendMarketInfo(Player *pPlayer, uint32_t npc_handle, std::vector<MarketInfo> pMarket)
{
    if (pPlayer == nullptr || pMarket.empty())
        return;

    XPacket marketPct(TS_SC_MARKET);
    pPlayer->SetLastContact("market", pMarket[0].name);

    marketPct << npc_handle;
    marketPct << (uint16_t) pMarket.size();
    for (auto info : pMarket) {
        marketPct << (int32_t) info.code;
#if EPIC >= 5
        marketPct << (int64_t) info.price_ratio;
        marketPct << (int32_t) 0;//info.huntaholic_ratio;
#else
        marketPct << (int32_t) info.price_ratio;
#endif // EPIC >= 5
    }

    pPlayer->GetSession().GetSocket().SendPacket(marketPct);
}

void Messages::SendItemMessage(Player *pPlayer, Item *pItem)
{
    if(pPlayer == nullptr || pItem == nullptr)
        return;

    XPacket inventoryPct(TS_SC_INVENTORY);
    inventoryPct << (uint16_t)1;
    fillItemInfo(inventoryPct, pItem);
    pPlayer->GetSession().GetSocket().SendPacket(inventoryPct);
}

void Messages::fillItemInfo(XPacket &packet, Item *item)
{
    packet << (uint32_t) item->m_nHandle;
    packet << (int32_t) item->m_Instance.Code;
    packet << (int64) item->m_Instance.UID;
#if EPIC >= 5
    packet << (int64) item->m_Instance.nCount;
#else
    packet << (int32) item->m_Instance.nCount;
#endif // EPIC >= 5

    packet << (uint32_t) item->m_Instance.nEndurance;
    packet << (uint8_t) item->m_Instance.nEnhance;
    packet << (uint8_t) item->m_Instance.nLevel;
    packet << (uint32_t) item->m_Instance.Flag;

    packet << (int32_t) item->m_Instance.Socket[0];
    packet << (int32_t) item->m_Instance.Socket[1];
    packet << (int32_t) item->m_Instance.Socket[2];
    packet << (int32_t) item->m_Instance.Socket[3];
    // Prior to Epic 6 we have to use 2 dummy socket slots.
    // Can you imagine how much time I wasted on this?
    packet << (int32_t) 0;
    packet << (int32_t) 0;
    packet << (int32_t) item->m_Instance.tExpire;

    packet << (int16_t) item->m_Instance.nWearInfo;
    packet << (uint32_t) (item->m_Instance.nOwnSummonUID > 0 ? item->m_Instance.OwnSummonHandle : 0);
#if EPIC >= 5
    packet << (int32_t) item->m_Instance.nIdx;
#endif // EPIC >= 5
}

void Messages::SendTimeSynch(Player* pPlayer)
{
    if(pPlayer == nullptr)
        return;

    XPacket result(TS_TIMESYNC);
    result << (uint32_t)sWorld->GetArTime();
    pPlayer->GetSession().GetSocket().SendPacket(result);
}

void Messages::SendGameTime(Player *pPlayer)
{
    if(pPlayer == nullptr)
        return;

    XPacket gtPct(TS_SC_GAME_TIME);
    gtPct << (uint32)sWorld->GetArTime();
    gtPct << (int64)time(nullptr);
    pPlayer->GetSession().GetSocket().SendPacket(gtPct);
}

void Messages::SendItemList(Player *pPlayer, bool bIsStorage)
{
    if (!pPlayer->m_lInventory.empty()) {
        ulong count = pPlayer->m_lInventory.size();
        ulong idx   = 0;
        if (count != 0) {
            do {
                XPacket packet(CSPACKETS::TS_SC_INVENTORY);
                auto    lcnt   = idx;
                ulong    mcount = 200;
                if (count - idx <= 200)
                    mcount = count - idx;

                packet << (uint16_t) mcount;

                auto ltotal = idx + mcount;
                if (idx < ltotal) {
                    do {
                        if (bIsStorage)
                            continue;
                        else
                            fillItemInfo(packet, pPlayer->m_lInventory[lcnt]);
                        ++lcnt;
                    } while (lcnt < ltotal);
                }
                pPlayer->GetSession().GetSocket().SendPacket(packet);
                idx += 200;
            } while (idx < count);
        }
    }
}

void Messages::SendResult(Player *pPlayer, uint16_t nMsg, uint16 nResult, uint16 nValue)
{
    if(pPlayer == nullptr)
        return;

    XPacket packet(CSPACKETS::TS_SC_RESULT);
    packet << nMsg;
    packet << nResult;
    packet << (uint32_t)nValue;
    pPlayer->GetSession().GetSocket().SendPacket(packet);
}

void Messages::sendEnterMessage(Player *pPlayer, WorldObject *pObj, bool bAbsolute)
{
    if(pObj == nullptr || pPlayer == nullptr)
        return;
    pObj->SendEnterMsg(pPlayer);

    if(pObj->GetObjType() != 0 && pObj->bIsMoving /*&& pObj->IsInWorld()*/)
        SendMoveMessage(pPlayer, dynamic_cast<Unit*>(pObj));
}

void Messages::SendMoveMessage(Player *pPlayer, Unit *pUnit)
{
    if(pPlayer == nullptr || pUnit == nullptr)
        return;

    if(pUnit->ends.size() < 2000) {
        XPacket movePct(TS_SC_MOVE);
        movePct << (uint32_t)pUnit->lastStepTime;
        movePct << (uint32)pUnit->GetHandle();
        movePct << (uint8_t)pUnit->GetLayer();
        movePct << (uint8_t)pUnit->speed;
        movePct << (uint16)pUnit->ends.size();
        for(auto& pos : pUnit->ends) {
            movePct << (float)pos.end.m_positionX;
            movePct << (float)pos.end.m_positionY;
        }
        pPlayer->GetSession().GetSocket().SendPacket(movePct);
    }
}

void Messages::SendWearInfo(Player *pPlayer, Unit *pUnit)
{
    XPacket packet(TS_SC_WEAR_INFO);
    packet << pUnit->GetHandle();
    for (int i = 0; i < Item::MAX_ITEM_WEAR; i++) {
        int wear_info = (pUnit->m_anWear[i] != nullptr ? pUnit->m_anWear[i]->m_Instance.Code : 0);
        if (i == 2 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 2);
        if (i == 4 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 3);
        if (i == 5 && wear_info == 0)
            wear_info = pUnit->GetInt32Value(UNIT_FIELD_MODEL + 4);
        packet << wear_info;
    }
    for (auto &i : pUnit->m_anWear) {
        packet << (i != nullptr ? i->m_Instance.nEnhance : 0);
    }
    for (auto &i : pUnit->m_anWear) {
        packet << (i != nullptr ? i->m_Instance.nLevel : 0);
    }
    packet.FinalizePacket();
    pPlayer->GetSession().GetSocket().SendPacket(packet);
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
    sWorld->Broadcast((uint)(pUnit->GetPositionX() / g_nRegionSize), (uint)(pUnit->GetPositionY() / g_nRegionSize), pUnit->GetLayer(), hpmpPct);
}

void Messages::BroadcastLevelMsg(Unit *pUnit)
{
    XPacket levelPct(TS_SC_LEVEL_UPDATE);
    levelPct << (uint32)pUnit->GetHandle();
    levelPct << (int)pUnit->getLevel();
    levelPct << (int)pUnit->GetCurrentJLv();
    sWorld->Broadcast((uint)(pUnit->GetPositionX() / g_nRegionSize), (uint)(pUnit->GetPositionY() / g_nRegionSize), pUnit->GetLayer(), levelPct);
}
