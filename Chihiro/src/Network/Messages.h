#pragma once
/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include "Common.h"
#include "Object.h"
#include "XPacket.h"

class Unit;
class Player;
class Summon;
class Monster;
class Item;
class XPacket;
class WorldSession;
struct MarketInfo;
class WorldObject;
class State;
class Quest;
struct TS_ITEM_INFO;

class Messages {
public:
    static void GetEncodedInt(XPacket &, uint32_t);
    static uint32_t GetStatusCode(WorldObject *pObj, Player *pClient);
    static void SendEXPMessage(Player *, Unit *);
    static void SendHPMPMessage(Player *, Unit *, int, float, bool);
    static void SendLevelMessage(Player *, Unit *);
    static void SendStatInfo(Player *, Unit *);
    static void SendAddSummonMessage(Player *, Summon *);
    static void SendCreatureEquipMessage(Player *, bool);
    static void SendPropertyMessage(Player *, Unit *, const std::string &, int64_t);
    static void SendPropertyMessage(Player *, Unit *, const std::string &, const std::string &);
    static void SendDialogMessage(Player *, uint32_t, int, const std::string &, const std::string &, const std::string &);
    static void SendSkillList(Player *, Unit *, int32_t);
    static void SendChatMessage(int, const std::string &, Player *, const std::string &);
    static void SendPartyChatMessage(int, const std::string &, int, const std::string &);
    static void SendMarketInfo(Player *, uint32_t, const std::vector<MarketInfo> &);
    static void SendItemList(Player *, bool);
    static void SendItemMessage(Player *, Item *);
    static void SendItemCountMessage(Player *, Item *);
    static void SendItemDestroyMessage(Player *, Item *);
    static void SendSkillCastFailMessage(Player *, uint32_t caster, uint32_t target, uint16_t skill_id, uint8_t skill_level, Position pos, int32_t error_code);
    static void SendGameTime(Player *);
    static void SendResult(Player *, uint16_t, uint16_t, uint32_t);
    static void SendResult(WorldSession *worldSession, uint16_t, uint16_t, uint32_t);
    static void SendResult(Player *, NGemity::Packets, uint16_t, uint32_t);
    static void SendResult(WorldSession *, NGemity::Packets, uint16_t, uint32_t);
    static void SendDropResult(Player *pPlayer, uint32_t itemHandle, bool bIsSuccess);
    static void sendEnterMessage(Player *, WorldObject *, bool);
    static void SendMoveMessage(Player *, Unit *);
    static void SendTimeSynch(Player *);
    static void SendWearInfo(Player *, Unit *);
    static void BroadcastHPMPMessage(Unit *, int, float, bool need_to_display = false);
    static void BroadcastLevelMsg(Unit *);
    static void BroadcastStatusMessage(WorldObject *obj);
    static void BroadcastStateMessage(Unit *pUnit, State *pState, bool bIsCancel);
    static void BroadcastTamingMessage(Player *pPlayer, Monster *pMonster, int32_t mode);
    static void SendStateMessage(Player *pPlayer, uint32_t handle, State *pState, bool bIsCancel);
    static void SendWarpMessage(Player *);
    static void SendCantAttackMessage(Player *, uint32_t, uint32_t, int32_t);
    static void SendQuestInformation(Player *pPlayer, int32_t code, int32_t text, int32_t type);
    static void SendQuestList(Player *pPlayer);
    static void SendGlobalChatMessage(int32_t chatType, const std::string &szSenderName, const std::string &szString, uint32_t len);
    static void SendLocalChatMessage(int32_t nChatType, uint32_t handle, const std::string &szMessage, uint32_t len);
    static void SendQuestMessage(int32_t nChatType, Player *pTarget, const std::string &szString);
    static void SendNPCStatusInVisibleRange(Player *pPlayer);
    static void SendQuestStatus(Player *pPlayer, Quest *pQuest);
    static void SendItemCoolTimeInfo(Player *pPlayer);
    static void SendMixResult(Player *pPlayer, std::vector<uint32_t> *pHandles);
    static void SendItemWearInfoMessage(Player *pPlayer, Unit *pTarget, Item *pItem);
    static void ShowSoulStoneRepairWindow(Player *);
    static void ShowSoulStoneCraftWindow(Player *);
    static void SendPartyInfo(Player *);
    static void SendRegionAckMessage(Player *pPlayer, uint32_t rx, uint32_t ry);
    static void SendOpenStorageMessage(Player *pPlayer);
    static void SendSkillCardInfo(Player *pPlayer, Item *pItem);
    static void SendToggleInfo(Unit *pUnit, int32_t skill_id, bool status);
    static void SendRemoveSummonMessage(Player *pPlayer, Summon *pSummon);
    static void BroadcastPartyMemberInfo(Player *pClient);
    static void BroadcastPartyLoginStatus(int32_t nPartyID, bool bIsOnline, const std::string &szName);
    static void SendTradeCancelMessage(Player *);
    static void SendTradeItemInfo(int32_t nTradeMode, Item *pItem, int32_t nCount, Player *pPlayer, Player *pTarget);
    static void SendSPMessage(Player* pPlayer, Summon* pSummon);

private:
    static std::optional<TS_ITEM_INFO> fillItemInfo(Item *);
};
