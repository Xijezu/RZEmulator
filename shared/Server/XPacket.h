#pragma once
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
#include "Common.h"
#include "ByteBuffer.h"
#include "TS_MESSAGE.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_ANTI_HACK.h"
#include "GameClient/TS_CS_ARRANGE_ITEM.h"
#include "GameClient/TS_CS_ATTACK_REQUEST.h"
#include "GameClient/TS_CS_AUCTION_BID.h"
#include "GameClient/TS_CS_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_CS_AUCTION_CANCEL.h"
#include "GameClient/TS_CS_AUCTION_INSTANT_PURCHASE.h"
#include "GameClient/TS_CS_AUCTION_REGISTER.h"
#include "GameClient/TS_CS_AUCTION_SEARCH.h"
#include "GameClient/TS_CS_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ENTER_WHILE_COUNTDOWN.h"
#include "GameClient/TS_CS_BATTLE_ARENA_EXERCISE_READY.h"
#include "GameClient/TS_CS_BATTLE_ARENA_EXERCISE_START.h"
#include "GameClient/TS_CS_BATTLE_ARENA_JOIN_QUEUE.h"
#include "GameClient/TS_CS_BATTLE_ARENA_LEAVE.h"
#include "GameClient/TS_CS_BIND_SKILLCARD.h"
#include "GameClient/TS_CS_BOOKMARK_TITLE.h"
#include "GameClient/TS_CS_BUY_FROM_BOOTH.h"
#include "GameClient/TS_CS_BUY_ITEM.h"
#include "GameClient/TS_CS_CANCEL_ACTION.h"
#include "GameClient/TS_CS_CHANGE_ALIAS.h"
#include "GameClient/TS_CS_CHANGE_ITEM_POSITION.h"
#include "GameClient/TS_CS_CHANGE_LOCATION.h"
#include "GameClient/TS_CS_CHANGE_SUMMON_NAME.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_CHECK_BOOTH_STARTABLE.h"
#include "GameClient/TS_CS_CHECK_CHARACTER_NAME.h"
#include "GameClient/TS_CS_CHECK_ILLEGAL_USER.h"
#include "GameClient/TS_CS_COMPETE_ANSWER.h"
#include "GameClient/TS_CS_COMPETE_REQUEST.h"
#include "GameClient/TS_CS_CONTACT.h"
#include "GameClient/TS_CS_CREATE_CHARACTER.h"
#include "GameClient/TS_CS_DECOMPOSE.h"
#include "GameClient/TS_CS_DELETE_CHARACTER.h"
#include "GameClient/TS_CS_DIALOG.h"
#include "GameClient/TS_CS_DONATE_ITEM.h"
#include "GameClient/TS_CS_DONATE_REWARD.h"
#include "GameClient/TS_CS_DROP_ITEM.h"
#include "GameClient/TS_CS_DROP_QUEST.h"
#include "GameClient/TS_CS_EMOTION.h"
#include "GameClient/TS_CS_END_QUEST.h"
#include "GameClient/TS_CS_ENTER_EVENT_AREA.h"
#include "GameClient/TS_CS_ERASE_ITEM.h"
#include "GameClient/TS_CS_FOSTER_CREATURE.h"
#include "GameClient/TS_CS_GAME_GUARD_AUTH_ANSWER.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_CS_GET_BOOTHS_NAME.h"
#include "GameClient/TS_CS_GET_REGION_INFO.h"
#include "GameClient/TS_CS_GET_SUMMON_SETUP_INFO.h"
#include "GameClient/TS_CS_GET_WEATHER_INFO.h"
#include "GameClient/TS_CS_GROUP_FINDER_LIST.h"
#include "GameClient/TS_CS_HIDE_EQUIP_INFO.h"
#include "GameClient/TS_CS_HUNTAHOLIC_BEGIN_HUNTING.h"
#include "GameClient/TS_CS_HUNTAHOLIC_CREATE_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_INSTANCE_LIST.h"
#include "GameClient/TS_CS_HUNTAHOLIC_JOIN_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_LEAVE_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_LEAVE_LOBBY.h"
#include "GameClient/TS_CS_INSTANCE_GAME_ENTER.h"
#include "GameClient/TS_CS_INSTANCE_GAME_EXIT.h"
#include "GameClient/TS_CS_INSTANCE_GAME_SCORE_REQUEST.h"
#include "GameClient/TS_CS_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_CS_ITEM_KEEPING_TAKE.h"
#include "GameClient/TS_CS_JOB_LEVEL_UP.h"
#include "GameClient/TS_CS_LEARN_SKILL.h"
#include "GameClient/TS_CS_LEAVE_EVENT_AREA.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_LOGOUT.h"
#include "GameClient/TS_CS_MIX.h"
#include "GameClient/TS_CS_MONSTER_RECOGNIZE.h"
#include "GameClient/TS_CS_MOVE_REQUEST.h"
#include "GameClient/TS_CS_NURSE_CREATURE.h"
#include "GameClient/TS_CS_OPEN_ITEM_SHOP.h"
#include "GameClient/TS_CS_PUTOFF_CARD.h"
#include "GameClient/TS_CS_PUTOFF_ITEM.h"
#include "GameClient/TS_CS_PUTON_CARD.h"
#include "GameClient/TS_CS_PUTON_ITEM.h"
#include "GameClient/TS_CS_PUTON_ITEM_SET.h"
#include "GameClient/TS_CS_QUERY.h"
#include "GameClient/TS_CS_QUEST_INFO.h"
#include "GameClient/TS_CS_RANKING_TOP_RECORD.h"
#include "GameClient/TS_CS_REGION_UPDATE.h"
#include "GameClient/TS_CS_REPAIR_SOULSTONE.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_REQUEST_FARM_INFO.h"
#include "GameClient/TS_CS_REQUEST_FARM_MARKET.h"
#include "GameClient/TS_CS_REQUEST_LOGOUT.h"
#include "GameClient/TS_CS_REQUEST_REMOVE_STATE.h"
#include "GameClient/TS_CS_REQUEST_RETURN_LOBBY.h"
#include "GameClient/TS_CS_RESURRECTION.h"
#include "GameClient/TS_CS_RETRIEVE_CREATURE.h"
#include "GameClient/TS_CS_RETURN_LOBBY.h"
#include "GameClient/TS_CS_SECURITY_NO.h"
#include "GameClient/TS_CS_SELL_ITEM.h"
#include "GameClient/TS_CS_SELL_TO_BOOTH.h"
#include "GameClient/TS_CS_SET_MAIN_TITLE.h"
#include "GameClient/TS_CS_SET_PET_NAME.h"
#include "GameClient/TS_CS_SET_PROPERTY.h"
#include "GameClient/TS_CS_SET_SUB_TITLE.h"
#include "GameClient/TS_CS_SKILL.h"
#include "GameClient/TS_CS_SOULSTONE_CRAFT.h"
#include "GameClient/TS_CS_START_BOOTH.h"
#include "GameClient/TS_CS_STOP_BOOTH.h"
#include "GameClient/TS_CS_STOP_WATCH_BOOTH.h"
#include "GameClient/TS_CS_STORAGE.h"
#include "GameClient/TS_CS_SUMMON.h"
#include "GameClient/TS_CS_SUMMON_CARD_SKILL_LIST.h"
#include "GameClient/TS_CS_SWAP_EQUIP.h"
#include "GameClient/TS_CS_TAKEOUT_COMMERCIAL_ITEM.h"
#include "GameClient/TS_CS_TAKE_ITEM.h"
#include "GameClient/TS_CS_TARGETING.h"
#include "GameClient/TS_CS_TRANSMIT_ETHEREAL_DURABILITY.h"
#include "GameClient/TS_CS_TRANSMIT_ETHEREAL_DURABILITY_TO_EQUIPMENT.h"
#include "GameClient/TS_CS_TURN_OFF_PK_MODE.h"
#include "GameClient/TS_CS_TURN_ON_PK_MODE.h"
#include "GameClient/TS_CS_UNBIND_SKILLCARD.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_CS_USE_ITEM.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_CS_WATCH_BOOTH.h"
#include "GameClient/TS_CS_XTRAP_CHECK.h"
#include "GameClient/TS_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_ACHIEVE_TITLE.h"
#include "GameClient/TS_SC_ADDED_SKILL_LIST.h"
#include "GameClient/TS_SC_ADD_PET_INFO.h"
#include "GameClient/TS_SC_ADD_SUMMON_INFO.h"
#include "GameClient/TS_SC_ANTI_HACK.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_SC_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_SC_AURA.h"
#include "GameClient/TS_SC_BATTLE_ARENA_ABSENCE_CHECK.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_INFO.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_SCORE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_STATUS.h"
#include "GameClient/TS_SC_BATTLE_ARENA_DISCONNECT_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS.h"
#include "GameClient/TS_SC_BATTLE_ARENA_JOIN_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_JOIN_QUEUE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_LEAVE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_PENALTY_INFO.h"
#include "GameClient/TS_SC_BATTLE_ARENA_RECONNECT_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_RESULT.h"
#include "GameClient/TS_SC_BATTLE_ARENA_UPDATE_WAIT_USER_COUNT.h"
#include "GameClient/TS_SC_BELT_SLOT_INFO.h"
#include "GameClient/TS_SC_BONUS_EXP_JP.h"
#include "GameClient/TS_SC_BOOKMARK_TITLE.h"
#include "GameClient/TS_SC_BOOTH_CLOSED.h"
#include "GameClient/TS_SC_BOOTH_TRADE_INFO.h"
#include "GameClient/TS_SC_CANT_ATTACK.h"
#include "GameClient/TS_SC_CHANGE_LOCATION.h"
#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_CHANGE_TITLE_CONDITION.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_CHAT_LOCAL.h"
#include "GameClient/TS_SC_CHAT_RESULT.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_INFO.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_LIST.h"
#include "GameClient/TS_SC_COMPETE_ANSWER.h"
#include "GameClient/TS_SC_COMPETE_COUNTDOWN.h"
#include "GameClient/TS_SC_COMPETE_END.h"
#include "GameClient/TS_SC_COMPETE_REQUEST.h"
#include "GameClient/TS_SC_COMPETE_START.h"
#include "GameClient/TS_SC_DECOMPOSE_RESULT.h"
#include "GameClient/TS_SC_DESTROY_ITEM.h"
#include "GameClient/TS_SC_DETECT_RANGE_UPDATE.h"
#include "GameClient/TS_SC_DIALOG.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_SC_DROP_RESULT.h"
#include "GameClient/TS_SC_EMOTION.h"
#include "GameClient/TS_SC_ENERGY.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_ERASE_ITEM.h"
#include "GameClient/TS_SC_EXP_UPDATE.h"
#include "GameClient/TS_SC_FARM_INFO.h"
#include "GameClient/TS_SC_GAME_GUARD_AUTH_QUERY.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "GameClient/TS_SC_GENERAL_MESSAGE_BOX.h"
#include "GameClient/TS_SC_GET_BOOTHS_NAME.h"
#include "GameClient/TS_SC_GET_CHAOS.h"
#include "GameClient/TS_SC_GOLD_UPDATE.h"
#include "GameClient/TS_SC_GROUP_FINDER_DETAIL.h"
#include "GameClient/TS_SC_GROUP_FINDER_LIST.h"
#include "GameClient/TS_SC_HAIR_INFO.h"
#include "GameClient/TS_SC_HIDE_EQUIP_INFO.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_HUNTAHOLIC_BEGIN_COUNTDOWN.h"
#include "GameClient/TS_SC_HUNTAHOLIC_BEGIN_HUNTING.h"
#include "GameClient/TS_SC_HUNTAHOLIC_HUNTING_SCORE.h"
#include "GameClient/TS_SC_HUNTAHOLIC_INSTANCE_INFO.h"
#include "GameClient/TS_SC_HUNTAHOLIC_INSTANCE_LIST.h"
#include "GameClient/TS_SC_HUNTAHOLIC_MAX_POINT_ACHIEVED.h"
#include "GameClient/TS_SC_HUNTAHOLIC_UPDATE_SCORE.h"
#include "GameClient/TS_SC_INSTANCE_GAME_SCORE_REQUEST.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ITEM_COOL_TIME.h"
#include "GameClient/TS_SC_ITEM_DROP_INFO.h"
#include "GameClient/TS_SC_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_SC_ITEM_WEAR_INFO.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MARKET.h"
#include "GameClient/TS_SC_MIX_RESULT.h"
#include "GameClient/TS_SC_MOUNT_SUMMON.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_MOVE_ACK.h"
#include "GameClient/TS_SC_NPC_TRADE_INFO.h"
#include "GameClient/TS_SC_OPEN_GUILD_WINDOW.h"
#include "GameClient/TS_SC_OPEN_ITEM_SHOP.h"
#include "GameClient/TS_SC_OPEN_STORAGE.h"
#include "GameClient/TS_SC_OPEN_TITLE.h"
#include "GameClient/TS_SC_OPEN_URL.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_QUEST_INFOMATION.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_QUEST_STATUS.h"
#include "GameClient/TS_SC_RANKING_TOP_RECORD.h"
#include "GameClient/TS_SC_REGEN_HPMP.h"
#include "GameClient/TS_SC_REGEN_INFO.h"
#include "GameClient/TS_SC_REGION_ACK.h"
#include "GameClient/TS_SC_REMAIN_TITLE_TIME.h"
#include "GameClient/TS_SC_REMOVE_PET_INFO.h"
#include "GameClient/TS_SC_REMOVE_SUMMON_INFO.h"
#include "GameClient/TS_SC_REQUEST_SECURITY_NO.h"
#include "GameClient/TS_SC_RESULT.h"
#include "GameClient/TS_SC_RESULT_FOSTER.h"
#include "GameClient/TS_SC_RESULT_NURSE.h"
#include "GameClient/TS_SC_RESULT_RETRIEVE.h"
#include "GameClient/TS_SC_SET_MAIN_TITLE.h"
#include "GameClient/TS_SC_SET_SUB_TITLE.h"
#include "GameClient/TS_SC_SET_TIME.h"
#include "GameClient/TS_SC_SHOW_CREATE_ALLIANCE.h"
#include "GameClient/TS_SC_SHOW_CREATE_GUILD.h"
#include "GameClient/TS_SC_SHOW_SET_PET_NAME.h"
#include "GameClient/TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW.h"
#include "GameClient/TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW.h"
#include "GameClient/TS_SC_SHOW_SUMMON_NAME_CHANGE.h"
#include "GameClient/TS_SC_SHOW_WINDOW.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_SKILLCARD_INFO.h"
#include "GameClient/TS_SC_SKILL_LEVEL_LIST.h"
#include "GameClient/TS_SC_SKILL_LIST.h"
#include "GameClient/TS_SC_SP.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_STATE_RESULT.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_SUMMON_EVOLUTION.h"
#include "GameClient/TS_SC_TAKE_ITEM_RESULT.h"
#include "GameClient/TS_SC_TAMING_INFO.h"
#include "GameClient/TS_SC_TARGET.h"
#include "GameClient/TS_SC_TITLE_CONDITION_LIST.h"
#include "GameClient/TS_SC_TITLE_LIST.h"
#include "GameClient/TS_SC_UNMOUNT_SUMMON.h"
#include "GameClient/TS_SC_UNSUMMON.h"
#include "GameClient/TS_SC_UNSUMMON_NOTICE.h"
#include "GameClient/TS_SC_UNSUMMON_PET.h"
#include "GameClient/TS_SC_UPDATE_ITEM_COUNT.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "GameClient/TS_SC_USE_ITEM_RESULT.h"
#include "GameClient/TS_SC_WARP.h"
#include "GameClient/TS_SC_WATCH_BOOTH.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "GameClient/TS_SC_WEATHER_INFO.h"
#include "GameClient/TS_SC_XTRAP_CHECK.h"
#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_TRADE.h"

class MessageBuffer;

namespace NGemity
{
    enum class Packets;
}

class XPacket : public ByteBuffer
{
    public:
        // just container for later use
        XPacket() : ByteBuffer(0), m_nPacketID(0)
        {
        }

        explicit XPacket(NGemity::Packets packID) : ByteBuffer(0), m_nPacketID(static_cast<uint16>(packID))
        {
            resize(7);
            put(4, m_nPacketID);
        }

        explicit XPacket(uint16 packID) : ByteBuffer(0), m_nPacketID(packID)
        {
            resize(7);
            put(4, packID);
        }

        explicit XPacket(uint16 packID, int32 res, char *encrypted) : ByteBuffer(res), m_nPacketID(packID)
        {
            append(encrypted, 7);
        }

        // copy constructor
        XPacket(const XPacket &packet) : ByteBuffer(packet), m_nPacketID(packet.m_nPacketID)
        {

        }

        explicit XPacket(uint16 packID, MessageBuffer &&buffer) : ByteBuffer(std::move(buffer)), m_nPacketID(packID)
        {
        }

        void FinalizePacket()
        {
            put(0, (uint32)size());
            put(6, (uint8)TS_MESSAGE::GetChecksum(m_nPacketID, size()));
        }

        void Reset()
        {
            clear();
            resize(7);
            put(4, m_nPacketID);
        }

        void Initialize(uint16 packID, size_t newres = 200)
        {
            clear();
            _storage.reserve(newres);
            m_nPacketID = packID;
        }

        uint16 GetPacketID() const { return m_nPacketID; }

        void SetPacketID(uint16 packID) { m_nPacketID = packID; }

    protected:
        uint16 m_nPacketID;
};