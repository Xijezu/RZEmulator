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

namespace fs = std::experimental::filesystem;

XLua::XLua()
{
    m_pState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
}

bool XLua::InitializeLua()
{
    if (!fs::exists("Resource/Script/"s))
        return false;

    // Monster relevant
    m_pState.set_function("set_way_point_type", &XLua::SCRIPT_SetWayPointType, this);
    m_pState.set_function("add_way_point", &XLua::SCRIPT_AddWayPoint, this);
    m_pState.set_function("respawn_rare_mob", &XLua::SCRIPT_RespawnRareMob, this);
    m_pState.set_function("respawn_rare_mob", &XLua::SCRIPT_RespawnRareMob2, this);
    m_pState.set_function("respawn_roaming_mob", &XLua::SCRIPT_RespawnRoamingMob, this);
    m_pState.set_function("respawn_guardian", &XLua::SCRIPT_RespawnGuardian, this);
    // NPC relevant
    m_pState.set_function("get_npc_id", &XLua::SCRIPT_GetNPCID, this);
    m_pState.set_function("dlg_title", &XLua::SCRIPT_DialogTitle, this);
    m_pState.set_function("dlg_text", &XLua::SCRIPT_DialogText, this);
    m_pState.set_function("dlg_text_without_quest_menu", &XLua::SCRIPT_DialogText, this);
    m_pState.set_function("dlg_menu", &XLua::SCRIPT_DialogMenu, this);
    m_pState.set_function("dlg_show", &XLua::SCRIPT_DialogShow, this);
    m_pState.set_function("open_market", &XLua::SCRIPT_ShowMarket, this);
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

    for (auto &it : fs::directory_iterator("Resource/Script/"s)) {
        if (it.path().extension().string() == ".lua"s) {
            auto t = m_pState.do_file(it.path());
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

    try {
        m_pState.script(szLua);
    } catch (std::exception &ex) {
        Messages::SendChatMessage(50, "@SCRIPT", dynamic_cast<Player *>(m_pUnit), ex.what());
        MX_LOG_ERROR("scripting", ex.what());
    }
    return true;
}

bool XLua::RunString(Unit *pObject, std::string pScript)
{
    std::string buf{ };
    RunString(pObject, pScript, buf);
}

bool XLua::RunString(std::string szScript)
{
    m_pState.script(szScript);
}

void XLua::SCRIPT_SetWayPointType(int waypoint_id, int waypoint_type)
{

}

void XLua::SCRIPT_AddWayPoint(int waypoint_id, int x, int y)
{

}

void XLua::SCRIPT_RespawnRareMob(int, int, int, int, int, int, int, int)
{

}

void XLua::SCRIPT_RespawnRareMob2(int, int, int, int, int, int, int)
{

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

    auto t = sMemoryPool->getMiscPtrFromId(player->GetLastContactLong("npc"));
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
    } else if (szKey == "level" || szKey == "lv") {
        return return_object(m_pUnit->getLevel());
    } else if (szKey == "job_depth") {
        if (m_pUnit->GetSubType() == ST_Player)
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

    if(m_pUnit->GetSubType() == ST_Player) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(szKey == "gold") {
            return return_object(player->GetGold());
        } else if(szKey == "guild_id") {
            return return_object(player->GetInt32Value(UNIT_FIELD_GUILD_ID));
        } else if(szKey == "permission") {
            return return_object(player->GetPermission());
        } else if(szKey == "chaos") {
            return return_object(player->GetInt32Value(UNIT_FIELD_CHAOS));
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

    return player->GetFlag(szKey);
}

void XLua::SCRIPT_SetFlag(sol::variadic_args args)
{
    auto player = dynamic_cast<Player *>(m_pUnit);
    if (player == nullptr)
        return;

    if (args.size() != 2)
        return;

    player->SetFlag(args[0], args[1]);
}

void XLua::SCRIPT_SetValue(std::string szKey, int64 nValue)
{
    if (m_pUnit == nullptr)
        return;

    if (szKey == "race") {
        m_pUnit->SetInt32Value(UNIT_FIELD_RACE, nValue);
    } else if (szKey == "job") {
        m_pUnit->SetCurrentJob(nValue);
    } else if (szKey == "level" || szKey == "lv") {
        m_pUnit->SetEXP(sObjectMgr->GetNeedExp(nValue));
    } else if (szKey == "job_level" || szKey == "jlv") {
        m_pUnit->SetCurrentJLv(nValue);
    } else if(szKey == "x") {
        m_pUnit->Relocate(nValue, m_pUnit->GetPositionY());
    } else if(szKey == "y") {
        m_pUnit->Relocate(m_pUnit->GetPositionX(), nValue);
    } else if (szKey == "stamina" || szKey == "stanima") { // I do that typo all the time /shrug
        m_pUnit->SetInt32Value(UNIT_FIELD_STAMINA, nValue);
    } else if (szKey == "jp") {
        m_pUnit->SetJP(nValue);
        Messages::SendPropertyMessage(dynamic_cast<Player*>(m_pUnit), m_pUnit, "jp", nValue);
    } else if(szKey == "hp") {
        m_pUnit->SetHealth(nValue);
        Messages::BroadcastHPMPMessage(m_pUnit, nValue, 0, false);
    } else if(szKey == "mp") {
        m_pUnit->SetMana(nValue);
        Messages::BroadcastHPMPMessage(m_pUnit,0,  nValue, false);
    } else if(szKey == "job_0")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB, nValue);
    else if(szKey == "job_1")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, nValue);
    else if(szKey == "job_2")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 2, nValue);
    else if(szKey == "jlv_0")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV, nValue);
    else if(szKey == "jlv_1")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV+ 1, nValue);
    else if(szKey == "jlv_2")
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV+ 2, nValue);

    if(m_pUnit->GetSubType() == ST_Player) {
        auto player = dynamic_cast<Player*>(m_pUnit);
        if(szKey == "gold") {
            player->SetUInt64Value(UNIT_FIELD_GOLD, nValue);
        } else if(szKey == "permission") {
            player->SetInt32Value(UNIT_FIELD_PERMISSION, nValue);
        } else if(szKey == "chaos") {
            player->SetInt32Value(UNIT_FIELD_CHAOS, nValue);
            player->SendGoldChaosMessage();
        }
    }
    auto player = dynamic_cast<Player*>(m_pUnit);
    if(player != nullptr)
        player->onChangeProperty(szKey, nValue);
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

    std::vector<MarketInfo> info = sObjectMgr->GetMarketInfo(szMarket);

    if(!info.empty()) {
        Messages::SendMarketInfo(player, player->GetLastContactLong("npc"), info);
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

    if(index < 0 || index > Item::MAX_ITEM_WEAR)
        return 0;

    return m_pUnit->m_anWear[index] == nullptr ? 0 : m_pUnit->m_anWear[index]->m_nHandle;
}

int XLua::SCRIPT_GetItemLevel(uint handle)
{
    auto item = sMemoryPool->getItemPtrFromId(handle);
    if(item == nullptr)
        return 0;
    return item->m_Instance.nLevel;
}

int XLua::SCRIPT_GetItemEnhance(uint handle)
{
    auto item = sMemoryPool->getItemPtrFromId(handle);
    if(item == nullptr)
        return 0;
    return item->m_Instance.nEnhance;
}

int XLua::SCRIPT_SetItemLevel(uint handle, int level)
{
    if(level > 255)
        return 0;
    auto item = sMemoryPool->getItemPtrFromId(handle);
    if(item == nullptr)
        return 0;
    item->m_Instance.nLevel = level;
    item->m_bIsNeedUpdateToDB = true;
    Messages::SendItemMessage(dynamic_cast<Player*>(m_pUnit), item);
    if(item->m_Instance.nWearInfo != WearNone) {
        if(item->m_Instance.OwnSummonHandle != 0) {
            // get summon
        } else {
            m_pUnit->CalculateStat();
        }
    }
    return level;
}

int XLua::SCRIPT_GetItemRank(uint handle)
{
    auto item = sMemoryPool->getItemPtrFromId(handle);
    if(item == nullptr)
        return 0;
    return item->m_pItemBase.rank;
}


int XLua::SCRIPT_GetItemPrice(uint handle)
{
    auto item = sMemoryPool->getItemPtrFromId(handle);
    if(item == nullptr)
        return 0;
    return item->m_pItemBase.price;
}

int XLua::SCRIPT_GetItemNameID(int code)
{
    return sObjectMgr->GetItemBase(code).name_id;
}

int XLua::SCRIPT_GetItemCode(uint handle)
{
    auto item = sMemoryPool->getItemPtrFromId(handle);
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
    auto player = dynamic_cast<Player*>(m_pUnit);

    if(player != nullptr && nCount >= 1 && nLevel >= 0 && nEnhance >= 0) {
        auto item = Item::AllocItem(0, nCode, nCount, GenerateCode::ByScript, nLevel, nEnhance, nFlag, 0, 0, 0, 0, 0);
        player->PushItem(item, nCount, false);

        return item->m_nHandle;
    }
    return 0;
}
