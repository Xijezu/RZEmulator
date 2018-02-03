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

#include "XLua.h"
#include <experimental/filesystem>
#include "Messages.h"
#include "ObjectMgr.h"
#include "MemPool.h"
#include "World.h"
#include "DungeonManager.h"

namespace fs = std::experimental::filesystem;

XLua::XLua()
{
    m_pState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);
}

bool XLua::InitializeLua()
{
    if (!fs::exists("Resource/Script/"s))
        return false;

    // Monster relevant
    m_pState.set_function("set_way_point_type", &XLua::SCRIPT_SetWayPointType, this);
    m_pState.set_function("add_way_point", &XLua::SCRIPT_AddWayPoint, this);
    m_pState.set_function("respawn_rare_mob", &XLua::SCRIPT_RespawnRareMob, this);
    m_pState.set_function("respawn_roaming_mob", &XLua::SCRIPT_RespawnRoamingMob, this);
    m_pState.set_function("respawn_guardian", &XLua::SCRIPT_RespawnGuardian, this);
    // NPC relevant
    m_pState.set_function("get_npc_id", &XLua::SCRIPT_GetNPCID, this);
    m_pState.set_function("dlg_title", &XLua::SCRIPT_DialogTitle, this);
    m_pState.set_function("dlg_text", &XLua::SCRIPT_DialogText, this);
    m_pState.set_function("dlg_text_without_quest_menu", &XLua::SCRIPT_DialogTextWithoutQuestMenu, this);
    m_pState.set_function("dlg_menu", &XLua::SCRIPT_DialogMenu, this);
    m_pState.set_function("dlg_show", &XLua::SCRIPT_DialogShow, this);
    m_pState.set_function("open_market", &XLua::SCRIPT_ShowMarket, this);
    m_pState.set_function("get_quest_progress", &XLua::SCRIPT_GetQuestProgress, this);
    m_pState.set_function("get_own_dungeon_id", &XLua::SCRIPT_GetOwnDungeonID, this);
    m_pState.set_function("get_siege_dungeon_id", &XLua::SCRIPT_GetSiegeDungeonID, this);
    // Getters
    m_pState.set_function("get_local_info", &XLua::SCRIPT_GetLocalFlag, this);
    m_pState.set_function("get_value", &XLua::SCRIPT_GetValue, this);
    m_pState.set_function("set_value", &XLua::SCRIPT_SetValue, this);
    m_pState.set_function("gv", &XLua::SCRIPT_GetValue, this);
    m_pState.set_function("sv", &XLua::SCRIPT_SetValue, this);
    m_pState.set_function("get_flag", &XLua::SCRIPT_GetFlag, this);
    m_pState.set_function("set_flag", &XLua::SCRIPT_SetFlag, this);
    m_pState.set_function("get_env", &XLua::SCRIPT_GetEnv, this);
    m_pState.set_function("get_proper_channel_num", &XLua::SCRIPT_GetProperChannelNum, this);
    m_pState.set_function("get_layer_of_channel", &XLua::SCRIPT_GetLayerOfChannel, this);
    m_pState.set_function("sconv", &XLua::SCRIPT_Conv, this);
    m_pState.set_function("message", &XLua::SCRIPT_Message, this);
    m_pState.set_function("call_lc_In", &XLua::SCRIPT_SetCurrentLocationID, this);
    m_pState.set_function("warp", &XLua::SCRIPT_Warp, this);
    m_pState.set_function("get_server_category", &XLua::SCRIPT_GetServerCategory, this);
    // Blacksmith functionality
    m_pState.set_function("get_wear_item_handle", &XLua::SCRIPT_GetWearItemHandle, this);
    m_pState.set_function("get_item_level", &XLua::SCRIPT_GetItemLevel, this);
    m_pState.set_function("set_item_level", &XLua::SCRIPT_SetItemLevel, this);
    m_pState.set_function("get_item_enhance", &XLua::SCRIPT_GetItemEnhance, this);
    m_pState.set_function("get_item_name_id", &XLua::SCRIPT_GetItemNameID, this);
    m_pState.set_function("get_item_code", &XLua::SCRIPT_GetItemCode, this);
    m_pState.set_function("update_gold_chaos", &XLua::SCRIPT_UpdateGoldChaos, this);
    m_pState.set_function("get_item_price", &XLua::SCRIPT_GetItemPrice, this);
    m_pState.set_function("get_item_rank", &XLua::SCRIPT_GetItemRank, this);
    m_pState.set_function("save", &XLua::SCRIPT_SavePlayer, this);
    m_pState.set_function("insert_item", &XLua::SCRIPT_InsertItem, this);
    m_pState.set_function("respawn", &XLua::SCRIPT_AddRespawnInfo, this);
    m_pState.set_function("cprint", &XLua::SCRIPT_CPrint, this);
    m_pState.set_function("raid_respawn", &XLua::SCRIPT_CPrint, this); /// TODO!!!!
    m_pState.set_function("add_npc", &XLua::SCRIPT_AddMonster, this);
    m_pState.set_function("get_creature_value", &XLua::SCRIPT_GetCreatureValue, this);
    m_pState.set_function("set_creature_value", &XLua::SCRIPT_SetCreatureValue, this);
    m_pState.set_function("gcv", &XLua::SCRIPT_GetCreatureValue, this);
    m_pState.set_function("scv", &XLua::SCRIPT_SetCreatureValue, this);
    m_pState.set_function("get_creature_handle", &XLua::SCRIPT_GetCreatureHandle, this);
    m_pState.set_function("creature_evolution", &XLua::SCRIPT_CreatureEvolution, this);
    m_pState.set_function("quest_info", &XLua::SCRIPT_QuestInfo, this);
    m_pState.set_function("start_quest", &XLua::SCRIPT_StartQuest, this);
    m_pState.set_function("end_quest", &XLua::SCRIPT_EndQuest, this);
    m_pState.set_function("enter_dungeon", &XLua::SCRIPT_EnterDungeon, this);
    m_pState.set_function("warp_to_revive_position", &XLua::SCRIPT_WarpToRevivePosition, this);
    m_pState.set_function("learn_all_skill", &XLua::SCRIPT_LearnAllSkill, this);
    m_pState.set_function("show_soulstone_craft_window", &XLua::SCRIPT_ShowSoulStoneCraftWindow, this);
    m_pState.set_function("show_soulstone_repair_window", &XLua::SCRIPT_ShowSoulStoneRepairWindow, this);
    m_pState.set_function("open_storage", &XLua::SCRIPT_OpenStorage, this);
    m_pState.set_function("add_state", &XLua::SCRIPT_AddState, this);

    for (auto &it : fs::directory_iterator("Resource/Script/"s)) {
        if (it.path().extension().string() == ".lua"s) {
            auto t = m_pState.do_file(it.path().string());
            if (!t.valid()) {
                sol::error err = t;
                MX_LOG_ERROR("scripting", err.what());
            }
        }
    }
    try {
        m_pState.script("on_server_init()");
    } catch (sol::error &ex) {
        MX_LOG_ERROR("scripting", ex.what());
        return false;
    }
    return true;
}

bool XLua::RunString(Unit *pObject, std::string szLua, std::string &szResult)
{
    m_pUnit  = pObject;
    szResult = "";
    if (szLua == "0")
        return true;

    try
    {
        m_pState.script(szLua);
    }
    catch (std::exception &ex)
    {
        Messages::SendChatMessage(50, "@SCRIPT", dynamic_cast<Player *>(m_pUnit), ex.what());
        MX_LOG_ERROR("scripting", ex.what());
    }
    return true;
}

bool XLua::RunString(Unit *pObject, std::string pScript)
{
    std::string buf{ };
    return RunString(pObject, pScript, buf);
}

bool XLua::RunString(std::string szScript)
{
    try {
        m_pState.script(szScript);
    }catch(sol::error err) {
        MX_LOG_ERROR("scripting", "%s", err.what());
		return false;
    }
	return true;
}

void XLua::SCRIPT_SetWayPointType(int waypoint_id, int waypoint_type)
{
    sObjectMgr->SetWayPointType(waypoint_id, waypoint_type);
}

void XLua::SCRIPT_AddWayPoint(int waypoint_id, int x, int y)
{
    sObjectMgr->AddWayPoint(waypoint_id, x, y);
}

void XLua::SCRIPT_RespawnRareMob(sol::variadic_args args)
{
    if(args.size() < 7)
        return;

    uint id = args[0].get<uint>();
    uint interval = args[1].get<uint>();
    float left = args[3].get<float>();
    float top = args[4].get<float>();
    float right = left + 1;
    float bottom = top + 1;
    uint monster_id = args[5].get<uint>();
    uint max_num = args[6].get<uint>();
    bool is_wander = args[7].get<bool>();
    int wander_id = args.size() > 7 ? args[8].get<uint>() : 0;

    MonsterRespawnInfo info(id, interval, left, top, right, bottom, monster_id, max_num, max_num, is_wander, wander_id);
    sObjectMgr->RegisterMonsterRespawnInfo(info);
}

void XLua::SCRIPT_RespawnRoamingMob(int, int, int, int, int)
{

}

void XLua::SCRIPT_RespawnGuardian(int, int, int, int, int, int, int, int)
{

}

int XLua::SCRIPT_GetNPCID()
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return -1;

    auto t = sMemoryPool->GetObjectInWorld<WorldObject>(player->GetLastContactLong("npc"));
    //sMemoryPool->getMiscPtrFromId(player->GetLastContactLong("npc"));
    if(t != nullptr)  {
        return t->GetUInt32Value(UNIT_FIELD_UID);
    }
    return 0;
}

void XLua::SCRIPT_DialogTitle(std::string szTitle)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    player->SetDialogTitle(szTitle, 0);
}

void ::XLua::SCRIPT_DialogText(std::string szText)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    player->SetDialogText(szText);
    sWorld->ShowQuestMenu(player);
}

void XLua::SCRIPT_DialogTextWithoutQuestMenu(std::string szText)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    player->SetDialogText(szText);
}


void XLua::SCRIPT_DialogMenu(std::string szKey, std::string szValue)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    player->AddDialogMenu(szKey, szValue);
}

void XLua::SCRIPT_DialogShow()
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    player->ShowDialog();
}

int XLua::SCRIPT_GetLocalFlag()
{
    return sConfigMgr->GetIntDefault("Game.LocalFlag", 8);
}

sol::object XLua::SCRIPT_GetValue(std::string szKey)
{
    if (m_pUnit == nullptr)
        return return_object("");

    if (szKey == "race") {
        return return_object(m_pUnit->GetInt32Value(UNIT_FIELD_RACE));
    } else if (szKey == "job") {
        return return_object(m_pUnit->GetCurrentJob());
    } else if (szKey == "max_hp") {
        return return_object(m_pUnit->GetMaxHealth());
    } else if (szKey == "max_mp") {
        return return_object(m_pUnit->GetMaxMana());
    } else if (szKey == "x") {
        return return_object(m_pUnit->GetPositionX());
    } else if (szKey == "y") {
        return return_object(m_pUnit->GetPositionY());
    } else if(szKey == "auto_user") {
        return return_object((int)0);
    } else if (szKey == "level" || szKey == "lv") {
        return return_object(m_pUnit->GetLevel());
    } else if (szKey == "job_depth") {
        if (m_pUnit->IsPlayer())
            return return_object(dynamic_cast<Player *>(m_pUnit)->GetJobDepth());
        else
            return return_object(""s);
    } else if (szKey == "job_level" || szKey == "jlv") {
        return return_object(m_pUnit->GetCurrentJLv());
    } else if (szKey == "stamina" || szKey == "stanima") { // I do that typo all the time /shrug
        return return_object(m_pUnit->GetInt32Value(UNIT_FIELD_STAMINA));
    } else if (szKey == "layer") {
        return return_object(m_pUnit->GetLayer());
    } else if (szKey == "jp") {
        return return_object(m_pUnit->GetJP());
    } else if (szKey == "name") {
        return return_object(m_pUnit->GetName());
    }
    else if(szKey == "job_0")
        return return_object(m_pUnit->GetPrevJobId(0));
    else if(szKey == "job_1")
        return return_object(m_pUnit->GetPrevJobId(1));
    else if(szKey == "job_2")
        return return_object(m_pUnit->GetPrevJobId(2));
    else if(szKey == "jlv_0")
        return return_object(m_pUnit->GetPrevJobLv(0));
    else if(szKey == "jlv_1")
        return return_object(m_pUnit->GetPrevJobLv(1));
    else if(szKey == "jlv2")
        return return_object(m_pUnit->GetPrevJobLv(2));

    if(m_pUnit->IsPlayer()) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(szKey == "gold") {
            return return_object((int64)player->GetGold());
        } else if(szKey == "guild_id") {
            return return_object(player->GetInt32Value(PLAYER_FIELD_GUILD_ID));
        } else if(szKey == "permission") {
            return return_object(player->GetPermission());
        } else if(szKey == "chaos") {
            return return_object(player->GetInt32Value(PLAYER_FIELD_CHAOS));
        }
    }
    MX_LOG_WARN("scripting", "Warning: Invalid key for get_value(key): %s", szKey.c_str());
    return return_object("");
}

std::string XLua::SCRIPT_GetFlag(std::string szKey)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return "";

    return player->GetCharacterFlag(szKey);
}

void XLua::SCRIPT_SetFlag(sol::variadic_args args)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    if (args.size() != 2)
        return;

    auto key = args[0].get<std::string>();
    auto value = args[1].get<std::string>();

    player->SetCharacterFlag(key, value);
}

void XLua::SCRIPT_SetValue(std::string szKey, sol::variadic_args args)
{
    if (m_pUnit == nullptr)
        return;

    if (szKey == "race") {
        m_pUnit->SetInt32Value(UNIT_FIELD_RACE, args[0].get<int>());
    } else if (szKey == "job") {
        m_pUnit->SetCurrentJob(args[0].get<uint>());
    } else if (szKey == "level" || szKey == "lv") {
        m_pUnit->SetEXP((uint)sObjectMgr->GetNeedExp(args[0].get<uint>()));
    } else if (szKey == "job_level" || szKey == "jlv") {
        m_pUnit->SetCurrentJLv(args[0].get<int>());
    } else if(szKey == "x") {
        m_pUnit->Relocate(args[0].get<int>(), m_pUnit->GetPositionY());
    } else if(szKey == "y") {
        m_pUnit->Relocate(m_pUnit->GetPositionX(), args[0].get<int>());
    } else if (szKey == "stamina" || szKey == "stanima") { // I do that typo all the time /shrug
        m_pUnit->SetInt32Value(UNIT_FIELD_STAMINA, args[0].get<int>());
    } else if (szKey == "jp") {
        m_pUnit->SetJP(args[0].get<int>());
        Messages::SendPropertyMessage(dynamic_cast<Player*>(m_pUnit), m_pUnit, "jp", args[0].get<int>());
    } else if(szKey == "hp") {
        m_pUnit->SetHealth(args[0].get<uint>());
        Messages::BroadcastHPMPMessage(m_pUnit, args[0].get<uint>(), 0, false);
    } else if(szKey == "mp") {
        m_pUnit->SetMana(args[0].get<uint>());
        Messages::BroadcastHPMPMessage(m_pUnit,0,  args[0].get<uint>(), false);
    } else if(szKey == "job_0")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB, args[0].get<uint>());
    else if(szKey == "job_1")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, args[0].get<uint>());
    else if(szKey == "job_2")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 2, args[0].get<uint>());
    else if(szKey == "jlv_0")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV, args[0].get<uint>());
    else if(szKey == "jlv_1")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV+ 1, args[0].get<uint>());
    else if(szKey == "jlv_2")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV+ 2, args[0].get<uint>());

    if(m_pUnit->IsPlayer()) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(szKey == "gold") {
            player->ChangeGold(args[0].get<int64>());
        } else if(szKey == "permission") {
            player->SetInt32Value(PLAYER_FIELD_PERMISSION, args[0].get<uint>());
        } else if(szKey == "chaos") {
            player->SetInt32Value(PLAYER_FIELD_CHAOS, args[0].get<uint>());
            player->SendGoldChaosMessage();
        }
    }
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player != nullptr)
        player->onChangeProperty(szKey, args[0].get<int>());
}

sol::object XLua::SCRIPT_GetEnv(std::string szKey)
{
    auto value = sConfigMgr->GetStringDefault(szKey.c_str(), "");
    if(isMXNumeric(value)) {
        int val = std::stoi(value);
        return return_object(val);
    } else {
        return return_object(value);
    }
}

void XLua::SCRIPT_ShowMarket(std::string szMarket)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    auto info = sObjectMgr->GetMarketInfo(szMarket);

    if(info != nullptr && !info->empty()) {
        Messages::SendMarketInfo(player, player->GetLastContactLong("npc"), *info);
    }
}

std::string XLua::SCRIPT_Conv(sol::variadic_args args)
{
    std::string result = "";
    if(args.size() >= 1 && args.size() % 2 == 1) {
        int v6 = args.size() - 1;
        auto v5 = args.size() - 1 < 0;

        int count = 0;
        do {
            if(count != 0)
                result += '\v';
            result += args[count].get<std::string>();
            ++count;
        } while(count < args.size());
        return result;
    }
}

void XLua::SCRIPT_Message(std::string szMsg)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr || szMsg.empty())
        return;

    Messages::SendChatMessage(40, "@SCRIPT", player, szMsg);
}

void XLua::SCRIPT_SetCurrentLocationID(int location_id)
{
    sObjectMgr->g_currentLocationId = location_id;
}

void XLua::SCRIPT_Warp(sol::variadic_args args)
{
    if(args.size() < 2)
        return;

    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    int x = args[0].get<int>();
    int y = args[1].get<int>();

    if (x < 0.0f || sConfigMgr->GetFloatDefault("Game.MapWidth", 700000) < x || y < 0.0f || sConfigMgr->GetFloatDefault("Game.MapHeight", 1000000) < y) {
        return;
    }
    player->PendWarp(x, y, 0);
}

int XLua::SCRIPT_GetServerCategory()
{
    int result = 0;
    result = ((sConfigMgr->GetIntDefault("Game.ServiceServer", 0) != 0) ? 1 : 0) << 29;
    result = result | (((sConfigMgr->GetIntDefault("Game.PKServer", 0) != 0) ? 1 : 0) << 28);
    result = result + sConfigMgr->GetIntDefault("Game.ServerIndex", 0);
    return result;
}

int XLua::SCRIPT_GetWearItemHandle(int index)
{
    if(m_pUnit == nullptr)
        return 0;

    if(index < 0 || index > MAX_ITEM_WEAR)
        return 0;

    return m_pUnit->m_anWear[index] == nullptr ? 0 : m_pUnit->m_anWear[index]->m_nHandle;
}

int XLua::SCRIPT_GetItemLevel(uint handle)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr)
        return 0;
    return item->m_Instance.nLevel;
}

int XLua::SCRIPT_GetItemEnhance(uint handle)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr)
        return 0;
    return item->m_Instance.nEnhance;
}

int XLua::SCRIPT_SetItemLevel(uint handle, int level)
{
    if(level > 255)
        return 0;
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr)
        return 0;
    item->m_Instance.nLevel = level;
    item->m_bIsNeedUpdateToDB = true;
    Messages::SendItemMessage(dynamic_cast<Player*>(m_pUnit), item);
    dynamic_cast<Player*>(m_pUnit)->UpdateQuestStatusByItemUpgrade();
    if(item->m_Instance.nWearInfo != WEAR_NONE) {
        if(item->m_Instance.OwnSummonHandle != 0) {
            // get summon
        } else {
            m_pUnit->CalculateStat();
            Messages::SendStatInfo(dynamic_cast<Player*>(m_pUnit), m_pUnit);
        }
    }
    return level;
}

int XLua::SCRIPT_GetItemRank(uint handle)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr || item->m_pItemBase == nullptr)
        return 0;
    return item->m_pItemBase->rank;
}


int XLua::SCRIPT_GetItemPrice(uint handle)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr || item->m_pItemBase == nullptr)
        return 0;
    return item->m_pItemBase->price;
}

int XLua::SCRIPT_GetItemNameID(int code)
{
    auto base = sObjectMgr->GetItemBase(code);
    if(base == nullptr)
        return 0;
    else
        return base->name_id;
}

int XLua::SCRIPT_GetItemCode(uint handle)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if(item == nullptr)
        return 0;
    return item->m_Instance.Code;
}

int XLua::SCRIPT_UpdateGoldChaos()
{
    if(m_pUnit == nullptr)
        return 0;
    dynamic_cast<Player*>(m_pUnit)->SendGoldChaosMessage();
    return 1;
}

void XLua::SCRIPT_SavePlayer()
{
    if(m_pUnit == nullptr)
        return;
    dynamic_cast<Player*>(m_pUnit)->Save(false);
}

uint XLua::SCRIPT_InsertItem(sol::variadic_args args)
{
    if(m_pUnit == nullptr)
        return 0;

    if(args.size() < 2)
        return 0;

    int nCode = args[0].get<int>();
    int nCount = args[1].get<int>();
    int nEnhance = args.size() >= 3 ? args[2].get<int>() : 0;
    int nLevel = args.size() >= 4 ? args[3].get<int>() : 0;
    int nFlag = args.size() >= 5 ? args[4].get<int>() : 0;
    auto pName = args.size() >= 6 ? args[5].get<std::string>() : m_pUnit->GetNameAsString();

    auto player = Player::FindPlayer(pName);
    uint handle = 0;
    if(player != nullptr && nCount >= 1 && nLevel >= 0 && nEnhance >= 0) {
        auto item = Item::AllocItem(0, nCode, nCount, BY_SCRIPT, nLevel, nEnhance, nFlag, 0, 0, 0, 0, 0);

        Item *pNewItem = player->PushItem(item, nCount, false);
        if(pNewItem != nullptr)
            handle = pNewItem->GetHandle();
        else
            handle = item->GetHandle();

        if(pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
            Item::PendFreeItem(item);
    }
    return handle;
}

void XLua::SCRIPT_AddRespawnInfo(sol::variadic_args args)
{
    if(args.size() < 9)
        return;
    auto id = args[0].get<int>();
    auto interval = args[1].get<uint>();
    auto left = args[2].get<float>();
    auto top =  args[3].get<float>();
    auto right =  args[4].get<float>();
    auto bottom =  args[5].get<float>();
    auto monster_id =  args[6].get<uint>();
    auto max_num =  args[7].get<uint>();
    auto inc =  args[8].get<uint>();
    auto wander_id = args.size() > 9 ? args[9].get<int>() : 0;
    MonsterRespawnInfo info(id, interval, left, top, right, bottom, monster_id, max_num, inc, true, wander_id);
    sObjectMgr->RegisterMonsterRespawnInfo(info);
}

void XLua::SCRIPT_CPrint(sol::variadic_args)
{

}

void XLua::SCRIPT_AddMonster(int x, int y, int id, int amount)
{
    for(int i = 0; i < amount; i++) {
        auto mob = sMemoryPool->AllocMonster(id);
        if(mob == nullptr)
            return;
        mob->SetCurrentXY(x, y);
        mob->SetRespawnPosition({(float)(x + irand(-10, 10)), (float)(y + irand(-10, 10)), 0});
        sWorld->AddMonsterToWorld(mob);
    }
}

int XLua::SCRIPT_GetCreatureHandle(int idx)
{
    if(m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return 0;

    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player->m_aBindSummonCard[idx] != nullptr && player->m_aBindSummonCard[idx]->m_pSummon != nullptr) {
        return player->m_aBindSummonCard[idx]->m_pSummon->GetHandle();
    }
    return 0;
}

void XLua::SCRIPT_SetCreatureValue(int handle, std::string key, sol::object value)
{
    //auto summon = dynamic_cast<Summon*>(sMemoryPool->getSummonPtrFromId(handle));
    auto summon = sMemoryPool->GetObjectInWorld<Summon>((uint)handle);
    if(summon == nullptr && handle < 6 && handle > 0) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(player != nullptr) {
            if(player->m_aBindSummonCard[handle] != nullptr && player->m_aBindSummonCard[handle]->m_pSummon != nullptr) {
                summon = player->m_aBindSummonCard[handle]->m_pSummon;
            }
        }
    }

    if(summon == nullptr)
        return;

    auto type = value.get_type();
    if(type == sol::type::number) {
        if(key == "level"s || key == "lv"s) {
            summon->SetEXP(sObjectMgr->GetNeedSummonExp(value.as<int>()));
            Messages::SendEXPMessage(dynamic_cast<Player*>(m_pUnit), summon);
        } else if(key == "ev_1_ID"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB, value.as<int>());
        } else if(key == "ev_2_ID"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, value.as<int>());
        } else if(key == "ev_1_level"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV, value.as<int>());
        } else if(key == "ev_2_level"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV + 1, value.as<int>());
        } else if(key == "hp") {
            summon->SetHealth(value.as<float>());
            Messages::SendHPMPMessage(dynamic_cast<Player*>(m_pUnit), summon, summon->GetHealth(), 0, false);
        } else if(key == "mp") {
            summon->SetMana(value.as<float>());
            Messages::SendHPMPMessage(dynamic_cast<Player*>(m_pUnit), summon, 0, summon->GetMana(), false);
        }
    }
}

sol::object XLua::SCRIPT_GetCreatureValue(int handle, std::string key)
{
    //auto summon = dynamic_cast<Summon*>(sMemoryPool->getSummonPtrFromId(handle));
    auto summon = sMemoryPool->GetObjectInWorld<Summon>((uint)handle);
    if(summon == nullptr && handle < 6 && handle > 0) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(player != nullptr) {
            if(player->m_aBindSummonCard[handle] != nullptr && player->m_aBindSummonCard[handle]->m_pSummon != nullptr) {
                summon = player->m_aBindSummonCard[handle]->m_pSummon;
            }
        }
    }

    if(summon == nullptr)
        return return_object((int)0);

    if(key == "hp"s) {
        return return_object(summon->GetHealth());
    } else if(key == "mp"s) {
        return return_object(summon->GetMana());
    } else if(key == "max_hp"s) {
        return return_object(summon->GetMaxHealth());///run creature_evolution(get_creature_handle(0))
    } else if(key == "max_mp"s) { ///run scv(get_creature_handle(0), "lv", 120)
        // /run insert_item(540006, 1, 0, 0, -2147483648)

        return return_object(summon->GetMaxMana());
    } else if(key == "evolution_depth"s) {
        return return_object(summon->m_nTransform);
    }  else if(key == "level"s) {
        return return_object(summon->GetLevel());
    }else if(key == "job"s) {
        return return_object(summon->GetSummonCode());
    } else if(key == "name"s) {
        return return_object(summon->GetName());
    } else if(key == "summon_state"s) {
        return return_object(summon->IsInWorld() ? 1 : 0);
    }
}

void XLua::SCRIPT_CreatureEvolution(int slot)
{
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player == nullptr)
        return;

    auto summon = player->GetSummonByHandle(slot);
    if(summon!= nullptr) {
        summon->DoEvolution();
    }
}

void XLua::SCRIPT_QuestInfo(int code, sol::variadic_args args)
{
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player == nullptr)
        return;
    int textID = 0;
    if(args.size() >= 1)
        textID = args[0].get<int>();
    Messages::SendQuestInformation(player, code, textID, 0);
}

int XLua::SCRIPT_GetQuestProgress(int quest)
{
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player == nullptr)
        return -1;
    return player->GetQuestProgress(quest);
}

int XLua::SCRIPT_GetOwnDungeonID()
{
    return 0;
}

int XLua::SCRIPT_GetSiegeDungeonID()
{
    return 0;
}

void XLua::SCRIPT_StartQuest(int code, sol::variadic_args args)
{
    if(code != 0 && args.size() >= 1)
    {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(player == nullptr)
            return;

        bool bForce = false;
        int nStartID = args[0].get<int>();
        if(args.size() == 2)
            bForce = args[1].get<bool>();

        player->StartQuest(code, nStartID, bForce);
    }
}

void XLua::SCRIPT_EndQuest(int quest_id, int reward_id, sol::variadic_args args)
{
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player == nullptr)
        return;

    bool bForce{false};
    if(args.size() != 0 && args[0].get_type() == sol::type::boolean)
        bForce = args[0].get<bool>();

    player->EndQuest(quest_id, reward_id, bForce);
}

void XLua::SCRIPT_EnterDungeon(int nDungeonID)
{
    auto pos = sDungeonManager->GetRaidStartPosition(nDungeonID);
    if(pos.GetPositionX() != 0 && pos.GetPositionY() != 0)
    {
        dynamic_cast<Player*>(m_pUnit)->PendWarp((int)pos.GetPositionX(), (int)pos.GetPositionY(), 0);
    }
}

int XLua::SCRIPT_LearnAllSkill()
{
    if(m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return 0;

    if(sObjectMgr->LearnAllSkill((Player*)m_pUnit))
        return 1;
    return 0;
}

void XLua::SCRIPT_WarpToRevivePosition(sol::variadic_args)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    auto revive_pos = player->GetLastTownPosition();

    player->PendWarp((int)revive_pos.GetPositionX(), (int)revive_pos.GetPositionY(), 0);
    player->SetMove(player->GetCurrentPosition(sWorld->GetArTime()), 0, 0);
}

void XLua::SCRIPT_ShowSoulStoneCraftWindow()
{
    if(m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    Messages::ShowSoulStoneCraftWindow(dynamic_cast<Player*>(m_pUnit));
}

void XLua::SCRIPT_ShowSoulStoneRepairWindow()
{
    if(m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    Messages::ShowSoulStoneRepairWindow(dynamic_cast<Player*>(m_pUnit));
}

void XLua::SCRIPT_OpenStorage()
{
    if(m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    dynamic_cast<Player*>(m_pUnit)->OpenStorage();
}

void XLua::SCRIPT_AddState(sol::variadic_args args)
{
    if(args.size() < 3)
    {
        MX_LOG_ERROR("scripting", "SCRIPT_AddState: Invalid Parameters");
        return;
    }

    int nStateCode = args[0].get<int>();
    int nStateLevel = args[1].get<uint8>();
    uint nStateTime = args[2].get<uint>();
    Player *player = args.size() == 4 ? Player::FindPlayer(args[3].get<std::string>()) : m_pUnit->As<Player>();
    if(player == nullptr)
    {
        MX_LOG_ERROR("scripting", "SCRIPT_AddState: Invalid Name");
        return;
    }

    player->AddState(SG_NORMAL, (StateCode)nStateCode, player->GetHandle(), nStateLevel, sWorld->GetArTime(), sWorld->GetArTime() + nStateTime, false, 0, "");
}
