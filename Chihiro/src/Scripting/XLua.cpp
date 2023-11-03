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

#include "XLua.h"

#include <filesystem>

#include "Config.h"
#include "DungeonManager.h"
#include "GameContent.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "ObjectMgr.h"
#include "World.h"

XLua::XLua()
{
    m_pState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::package);
}

bool XLua::InitializeLua()
{

    auto configFile = std::filesystem::path(ConfigMgr::instance()->GetCorrectPath("Resource/Script/"));

    // Monster relevant
    m_pState.set_function("set_way_point_type", &XLua::SCRIPT_SetWayPointType, this);
    m_pState.set_function("add_way_point", &XLua::SCRIPT_AddWayPoint, this);
    m_pState.set_function("respawn_rare_mob", &XLua::SCRIPT_RespawnRareMob, this);
    m_pState.set_function("respawn_roaming_mob", &XLua::SCRIPT_RespawnRoamingMob, this);
    m_pState.set_function("respawn_guardian", &XLua::SCRIPT_RespawnGuardian, this);
    m_pState.set_function("respawn", &XLua::SCRIPT_AddRespawnInfo, this);
    m_pState.set_function("raid_respawn", &XLua::SCRIPT_CPrint,
        this); /// TODO!!!!
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
    m_pState.set_function("insert_gold", &XLua::SCRIPT_InsertGold, this);
    m_pState.set_function("cprint", &XLua::SCRIPT_CPrint, this);
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
    m_pState.set_function("add_cstate", &XLua::SCRIPT_AddCreatureState, this);

    int32_t nFiles{0};
    for (auto &it : std::filesystem::directory_iterator(configFile)) {
        if (it.path().extension().string() == ".lua"s) {
            auto t = m_pState.do_file(it.path().string());
            if (!t.valid()) {
                sol::error err = t;
                NG_LOG_ERROR("server.scripting", "%s", err.what());
            }
            else {
                nFiles++;
            }
        }
    }
    try {
        m_pState.script("on_server_init()");
    }
    catch (sol::error &ex) {
        NG_LOG_ERROR("server.scripting", "%s", ex.what());
        return false;
    }
    NG_LOG_INFO("server.scripting", "Loaded %d files.", nFiles);
    return true;
}

bool XLua::RunString(Unit *pObject, std::string szLua, std::string &szResult)
{
    m_pUnit = pObject;
    szResult = "";
    if (szLua == "0")
        return true;

    try {
        m_pState.script(szLua);
    }
    catch (std::exception &ex) {
        Messages::SendChatMessage(50, "@SCRIPT", m_pUnit->As<Player>(), ex.what());
        NG_LOG_ERROR("server.scripting", "%s", ex.what());
    }
    return true;
}

bool XLua::RunString(Unit *pObject, std::string pScript)
{
    std::string buf{};
    return RunString(pObject, pScript, buf);
}

bool XLua::RunString(std::string szScript)
{
    try {
        m_pState.script(szScript);
    }
    catch (sol::error &err) {
        NG_LOG_ERROR("server.scripting", "%s", err.what());
        return false;
    }
    return true;
}

void XLua::SCRIPT_SetWayPointType(int32_t waypoint_id, int32_t waypoint_type)
{
    sObjectMgr.SetWayPointType(waypoint_id, waypoint_type);
}

void XLua::SCRIPT_AddWayPoint(int32_t waypoint_id, int32_t x, int32_t y)
{
    sObjectMgr.AddWayPoint(waypoint_id, x, y);
}

void XLua::SCRIPT_RespawnRareMob(sol::variadic_args args)
{
    if (args.size() < 7)
        return;

    uint32_t id = args[0].get<int32_t>();
    uint32_t interval = args[1].get<uint32_t>();
    float left = args[2].get<float>();
    float top = args[3].get<float>();
    float right = left + 1;
    float bottom = top + 1;
    uint32_t monster_id = args[4].get<uint32_t>();
    uint32_t max_num = args[5].get<uint32_t>();
    bool is_wander = args[6].get<bool>();
    int32_t wander_id = args.size() > 7 ? args[7].get<uint32_t>() : 0;

    MonsterRespawnInfo info(id, interval, left, top, right, bottom, monster_id, max_num, max_num, is_wander, wander_id);
    sObjectMgr.RegisterMonsterRespawnInfo(info);
}

void XLua::SCRIPT_RespawnRoamingMob(sol::variadic_args args)
{
    if (args.size() < 4)
        return;

    auto nRoamingID = args[0].get<int32_t>();
    auto nMonsterID = args[1].get<int32_t>();
    auto nAngle = args[2].get<int32_t>();
    auto nDistance = args[3].get<float>() * GameRule::DEFAULT_UNIT_SIZE;
    uint32_t nRespawnInterval = (args.size() >= 5) ? args[4].get<uint32_t>() * 100 : 0;
}

void XLua::SCRIPT_RespawnGuardian(int, int, int, int, int, int, int, int32_t) {}

int32_t XLua::SCRIPT_GetNPCID()
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return -1;

    auto t = sMemoryPool.GetObjectInWorld<WorldObject>(player->GetLastContactLong("npc"));
    if (t != nullptr) {
        return t->GetUInt32Value(UNIT_FIELD_UID);
    }
    return 0;
}

void XLua::SCRIPT_DialogTitle(std::string szTitle)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    player->SetDialogTitle(szTitle, 0);
}

void ::XLua::SCRIPT_DialogText(std::string szText)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    player->SetDialogText(szText);
    sWorld.ShowQuestMenu(player);
}

void XLua::SCRIPT_DialogTextWithoutQuestMenu(std::string szText)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    player->SetDialogText(szText);
}

void XLua::SCRIPT_DialogMenu(std::string szKey, std::string szValue)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    player->AddDialogMenu(szKey, szValue);
}

void XLua::SCRIPT_DialogShow()
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    player->ShowDialog();
}

int32_t XLua::SCRIPT_GetLocalFlag()
{
    return sWorld.getIntConfig(CONFIG_LOCAL_FLAG);
}

constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

sol::object XLua::SCRIPT_GetValue(sol::variadic_args args)
{
    auto szKey = args[0].get<std::string>();
    if (args.size() > 1)
        m_pUnit = Player::FindPlayer(args[2].get<std::string>());

    if (m_pUnit == nullptr)
        return return_object("");

    switch (str2int(szKey.c_str())) {
    case str2int("race"):
        return return_object(m_pUnit->GetInt32Value(UNIT_FIELD_RACE));
    case str2int("job"):
        return return_object(m_pUnit->GetCurrentJob());
    case str2int("hp"):
    case str2int("health"):
        return return_object(m_pUnit->GetHealth());
    case str2int("mp"):
    case str2int("mana"):
        return return_object(m_pUnit->GetMana());
    case str2int("max_hp"):
        return return_object(m_pUnit->GetMaxHealth());
    case str2int("max_mp"):
        return return_object(m_pUnit->GetMaxMana());
    case str2int("x"):
        return return_object(m_pUnit->GetPositionX());
    case str2int("y"):
        return return_object(m_pUnit->GetPositionY());
    case str2int("auto_user"):
        return return_object((int32_t)0);
    case str2int("lv"):
    case str2int("level"):
        return return_object(m_pUnit->GetLevel());
    case str2int("job_depth"):
        return m_pUnit->IsPlayer() ? return_object(m_pUnit->As<Player>()->GetJobDepth()) : return_object(""s);
    case str2int("job_level"):
    case str2int("jlv"):
        return return_object(m_pUnit->GetCurrentJLv());
    case str2int("stamina"):
    case str2int("stanima"): // I do that typo all the time /shrug
        return return_object(m_pUnit->GetInt32Value(UNIT_FIELD_STAMINA));
    case str2int("layer"):
        return return_object(m_pUnit->GetLayer());
    case str2int("jp"):
        return return_object(m_pUnit->GetJP());
    case str2int("name"):
        return return_object(m_pUnit->GetName());
    case str2int("job_0"):
        return return_object(m_pUnit->GetPrevJobId(0));
    case str2int("job_1"):
        return return_object(m_pUnit->GetPrevJobId(1));
    case str2int("job_2"):
        return return_object(m_pUnit->GetPrevJobId(2));
    case str2int("jlv_0"):
        return return_object(m_pUnit->GetPrevJobLv(0));
    case str2int("jlv_1"):
        return return_object(m_pUnit->GetPrevJobLv(1));
    case str2int("jlv_2"):
        return return_object(m_pUnit->GetPrevJobLv(2));
    default:
        break;
    }

    if (!m_pUnit->IsPlayer())
        return return_object("");

    auto pPlayer = m_pUnit->As<Player>();
    switch (str2int(szKey.c_str())) {
    case str2int("gold"):
        return return_object((int64_t)pPlayer->GetGold());
    case str2int("guild_id"):
        return return_object(pPlayer->GetInt32Value(PLAYER_FIELD_GUILD_ID));
    case str2int("permission"):
        return return_object(pPlayer->GetPermission());
    case str2int("chaos"):
        return return_object(pPlayer->GetInt32Value(PLAYER_FIELD_CHAOS));
    }

    NG_LOG_WARN("server.scripting", "Warning: Invalid key for get_value(key): %s", szKey.c_str());
    return return_object("");
}

std::string XLua::SCRIPT_GetFlag(std::string szKey)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return "";

    return player->GetCharacterFlag(szKey);
}

void XLua::SCRIPT_SetFlag(sol::variadic_args args)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    if (args.size() != 2)
    {
        return;
    }

    auto key = args[0].get<std::string>();
    auto value = args[1].get<std::string>();

    player->SetCharacterFlag(key, value);
}

void XLua::SCRIPT_SetValue(std::string szKey, sol::variadic_args args)
{
    if (args.size() > 1)
        m_pUnit = Player::FindPlayer(args[1].get<std::string>());

    if (m_pUnit == nullptr)
        return;

    switch (str2int(szKey.c_str())) {
    case str2int("race"):
        m_pUnit->SetInt32Value(UNIT_FIELD_RACE, args[0].get<int32_t>());
        break;
    case str2int("job"):
        m_pUnit->SetCurrentJob(args[0].get<uint32_t>());
        break;
    case str2int("hp"):
    case str2int("health"):
        m_pUnit->SetHealth(args[0].get<uint32_t>());
        Messages::BroadcastHPMPMessage(m_pUnit, args[0].get<uint32_t>(), 0, false);
        break;
    case str2int("mp"):
    case str2int("mana"):
        m_pUnit->SetMana(args[0].get<uint32_t>());
        Messages::BroadcastHPMPMessage(m_pUnit, 0, args[0].get<uint32_t>(), false);
        break;
    case str2int("x"):
        m_pUnit->Relocate(args[0].get<int32_t>(), m_pUnit->GetPositionY());
        break;
    case str2int("y"):
        m_pUnit->Relocate(m_pUnit->GetPositionX(), args[0].get<int32_t>());
        break;
    case str2int("lv"):
    case str2int("level"):
        m_pUnit->SetEXP((uint32_t)sObjectMgr.GetNeedExp(args[0].get<uint32_t>()));
        break;
    case str2int("job_level"):
    case str2int("jlv"):
        m_pUnit->SetCurrentJLv(args[0].get<int32_t>());
        break;
    case str2int("stamina"):
    case str2int("stanima"): // I do that typo all the time /shrug
        m_pUnit->SetInt32Value(UNIT_FIELD_STAMINA, args[0].get<int32_t>());
        break;
    case str2int("jp"):
        m_pUnit->SetJP(args[0].get<int32_t>());
        Messages::SendPropertyMessage(m_pUnit->As<Player>(), m_pUnit, "jp", args[0].get<int32_t>());
        break;
    case str2int("job_0"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB, args[0].get<uint32_t>());
        break;
    case str2int("job_1"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, args[0].get<uint32_t>());
        break;
    case str2int("job_2"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JOB + 2, args[0].get<uint32_t>());
        break;
    case str2int("jlv_0"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV, args[0].get<uint32_t>());
        break;
    case str2int("jlv_1"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV + 1, args[0].get<uint32_t>());
        break;
    case str2int("jlv_2"):
        m_pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV + 2, args[0].get<uint32_t>());
        break;
    default:
        break;
    }

    if (!m_pUnit->IsPlayer())
        return;

    auto pPlayer = m_pUnit->As<Player>();
    switch (str2int(szKey.c_str())) {
    case str2int("gold"):
        pPlayer->ChangeGold(args[0].get<int64_t>());
        break;
    case str2int("permission"):
        pPlayer->SetInt32Value(PLAYER_FIELD_PERMISSION, args[0].get<uint32_t>());
        break;
    case str2int("chaos"):
        pPlayer->SetInt32Value(PLAYER_FIELD_CHAOS, args[0].get<uint32_t>());
        pPlayer->SendGoldChaosMessage();
        break;
    default:
        break;
    }

    pPlayer->onChangeProperty(szKey, args[0].get<int32_t>());
}

sol::object XLua::SCRIPT_GetEnv(std::string szKey)
{
    auto value = sConfigMgr->GetStringDefault(szKey.c_str(), "");
    if (isMXNumeric(value)) {
        int32_t val = std::stoi(value);
        return return_object(val);
    }
    else {
        return return_object(value);
    }
}

void XLua::SCRIPT_ShowMarket(std::string szMarket)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    auto info = sObjectMgr.GetMarketInfo(szMarket);

    if (info != nullptr && !info->empty()) {
        Messages::SendMarketInfo(player, player->GetLastContactLong("npc"), *info);
    }
}

std::string XLua::SCRIPT_Conv(sol::variadic_args args)
{
    std::string result{};
    if (args.size() >= 1 && args.size() % 2 == 1) {
        std::size_t count = 0;
        do {
            if (count != 0)
                result += '\v';
            result += args[count].get<std::string>();
            ++count;
        } while (count < args.size());
    }
    return result;
}

void XLua::SCRIPT_Message(std::string szMsg)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr || szMsg.empty())
        return;

    Messages::SendChatMessage(40, "@SCRIPT", player, szMsg);
}

void XLua::SCRIPT_SetCurrentLocationID(int32_t location_id)
{
    sObjectMgr.g_currentLocationId = location_id;
}

void XLua::SCRIPT_Warp(sol::variadic_args args)
{
    if (args.size() < 2)
        return;

    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    int32_t x = args[0].get<int32_t>();
    int32_t y = args[1].get<int32_t>();

    if (x < 0.0f || sWorld.getIntConfig(CONFIG_MAP_WIDTH) < x || y < 0.0f || sWorld.getIntConfig(CONFIG_MAP_HEIGHT) < y) {
        return;
    }
    player->PendWarp(x, y, 0);
}

int32_t XLua::SCRIPT_GetServerCategory()
{
    int32_t result = 0;
    result = (sWorld.getBoolConfig(CONFIG_SERVICE_SERVER) ? 1 : 0) << 29;
    result = result | ((sWorld.getBoolConfig(CONFIG_PK_SERVER) ? 1 : 0) << 28);
    result = result + sWorld.getIntConfig(CONFIG_SERVER_INDEX);
    return result;
}

int32_t XLua::SCRIPT_GetWearItemHandle(int32_t index)
{
    if (m_pUnit == nullptr)
        return 0;

    if (index < 0 || index > MAX_ITEM_WEAR)
        return 0;

    return m_pUnit->m_anWear[index] == nullptr ? 0 : m_pUnit->m_anWear[index]->m_nHandle;
}

int32_t XLua::SCRIPT_GetItemLevel(uint32_t handle)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr)
        return 0;
    return item->GetItemInstance().GetLevel();
}

int32_t XLua::SCRIPT_GetItemEnhance(uint32_t handle)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr)
        return 0;
    return item->GetItemInstance().GetEnhance();
}

int32_t XLua::SCRIPT_SetItemLevel(uint32_t handle, int32_t level)
{
    if (level > 255)
        return 0;
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr)
        return 0;
    item->GetItemInstance().SetLevel(level);
    item->m_bIsNeedUpdateToDB = true;
    Messages::SendItemMessage(m_pUnit->As<Player>(), item);
    m_pUnit->As<Player>()->UpdateQuestStatusByItemUpgrade();
    if (item->GetItemInstance().GetItemWearType() != WEAR_NONE) {
        if (item->GetItemInstance().GetOwnSummonHandle() != 0) {
            // get summon
        }
        else {
            m_pUnit->CalculateStat();
            Messages::SendStatInfo(m_pUnit->As<Player>(), m_pUnit);
        }
    }
    return level;
}

int32_t XLua::SCRIPT_GetItemRank(uint32_t handle)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr || item->GetItemTemplate() == nullptr)
        return 0;
    return item->GetItemTemplate()->rank;
}

int32_t XLua::SCRIPT_GetItemPrice(uint32_t handle)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr || item->GetItemTemplate() == nullptr)
        return 0;
    return item->GetItemTemplate()->price;
}

int32_t XLua::SCRIPT_GetItemNameID(int32_t code)
{
    auto base = sObjectMgr.GetItemBase(code);
    if (base == nullptr)
        return 0;
    else
        return base->nNameID;
}

int32_t XLua::SCRIPT_GetItemCode(uint32_t handle)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (item == nullptr)
        return 0;
    return item->GetItemInstance().GetCode();
}

int32_t XLua::SCRIPT_UpdateGoldChaos()
{
    if (m_pUnit == nullptr)
        return 0;
    m_pUnit->As<Player>()->SendGoldChaosMessage();
    return 1;
}

void XLua::SCRIPT_SavePlayer()
{
    if (m_pUnit == nullptr)
        return;
    m_pUnit->As<Player>()->Save(false);
}

uint32_t XLua::SCRIPT_InsertItem(sol::variadic_args args)
{
    if (m_pUnit == nullptr)
        return 0;

    if (args.size() < 2)
        return 0;

    int32_t nCode = args[0].get<int32_t>();
    int32_t nCount = args[1].get<int32_t>();
    int32_t nEnhance = args.size() >= 3 ? args[2].get<int32_t>() : 0;
    int32_t nLevel = args.size() >= 4 ? args[3].get<int32_t>() : 0;
    int32_t nFlag = args.size() >= 5 ? args[4].get<int32_t>() : 0;
    auto pName = args.size() >= 6 ? args[5].get<std::string>() : m_pUnit->GetNameAsString();

    auto player = Player::FindPlayer(pName);
    uint32_t handle = 0;
    if (player != nullptr && nCount >= 1 && nLevel >= 0 && nEnhance >= 0) {
        auto item = Item::AllocItem(0, nCode, nCount, BY_SCRIPT, nLevel, nEnhance, nFlag, 0, 0, 0, 0, 0);

        Item *pNewItem = player->PushItem(item, nCount, false);
        if (pNewItem != nullptr)
            handle = pNewItem->GetHandle();
        else
            handle = item->GetHandle();

        if (pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
            Item::PendFreeItem(item);
    }
    return handle;
}

void XLua::SCRIPT_AddRespawnInfo(sol::variadic_args args)
{
    if (args.size() < 9)
        return;

    auto id = args[0].get<uint32_t>();
    auto interval = args[1].get<uint32_t>();
    auto left = args[2].get<float>();
    auto top = args[3].get<float>();
    auto right = args[4].get<float>();
    auto bottom = args[5].get<float>();
    auto monster_id = args[6].get<uint32_t>();
    auto max_num = args[7].get<uint32_t>();
    auto inc = args[8].get<uint32_t>();
    auto wander_id = args.size() > 9 ? args[9].get<int32_t>() : 0;

    assert(max_num != 0);

    MonsterRespawnInfo info(id, interval, left, top, right, bottom, monster_id, max_num, inc, true, wander_id);
    sObjectMgr.RegisterMonsterRespawnInfo(info);
}

void XLua::SCRIPT_CPrint(sol::variadic_args) {}

void XLua::SCRIPT_AddMonster(int32_t x, int32_t y, int32_t id, int32_t amount)
{
    for (int32_t i = 0; i < amount; i++) {
        auto mob = GameContent::RespawnMonster((float)x, (float)y, 0, id, true, 0, nullptr, false);
        mob->m_bNearClient = true;
    }
}

int32_t XLua::SCRIPT_GetCreatureHandle(int32_t idx)
{
    if (m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return 0;

    auto player = m_pUnit->As<Player>();
    if (player->m_aBindSummonCard[idx] != nullptr && player->m_aBindSummonCard[idx]->m_pSummon != nullptr) {
        return player->m_aBindSummonCard[idx]->m_pSummon->GetHandle();
    }
    return 0;
}

void XLua::SCRIPT_SetCreatureValue(int32_t handle, std::string key, sol::object value)
{
    auto summon = sMemoryPool.GetObjectInWorld<Summon>((uint32_t)handle);
    if (summon == nullptr && handle < 6 && handle > 0) {
        auto player = m_pUnit->As<Player>();
        if (player != nullptr) {
            if (player->m_aBindSummonCard[handle] != nullptr && player->m_aBindSummonCard[handle]->m_pSummon != nullptr) {
                summon = player->m_aBindSummonCard[handle]->m_pSummon;
            }
        }
    }

    if (summon == nullptr)
        return;

    auto type = value.get_type();
    if (type == sol::type::number) {
        if (key == "level"s || key == "lv"s) {
            summon->SetEXP(sObjectMgr.GetNeedSummonExp(value.as<int32_t>()));
            Messages::SendEXPMessage(m_pUnit->As<Player>(), summon);
        }
        else if (key == "ev_1_ID"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB, value.as<int32_t>());
        }
        else if (key == "ev_2_ID"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, value.as<int32_t>());
        }
        else if (key == "ev_1_level"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV, value.as<int32_t>());
        }
        else if (key == "ev_2_level"s) {
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV + 1, value.as<int32_t>());
        }
        else if (key == "hp") {
            summon->SetHealth(value.as<float>());
            Messages::SendHPMPMessage(m_pUnit->As<Player>(), summon, summon->GetHealth(), 0, false);
        }
        else if (key == "mp") {
            summon->SetMana(value.as<float>());
            Messages::SendHPMPMessage(m_pUnit->As<Player>(), summon, 0, summon->GetMana(), false);
        }
    }
}

sol::object XLua::SCRIPT_GetCreatureValue(int32_t handle, std::string key)
{
    auto summon = sMemoryPool.GetObjectInWorld<Summon>((uint32_t)handle);
    if (summon == nullptr && handle < 6 && handle > 0) {
        auto player = m_pUnit->As<Player>();
        if (player != nullptr) {
            if (player->m_aBindSummonCard[handle] != nullptr && player->m_aBindSummonCard[handle]->m_pSummon != nullptr) {
                summon = player->m_aBindSummonCard[handle]->m_pSummon;
            }
        }
    }

    if (summon == nullptr)
        return return_object((int32_t)0);

    if (key == "hp"s) {
        return return_object(summon->GetHealth());
    }
    else if (key == "mp"s) {
        return return_object(summon->GetMana());
    }
    else if (key == "max_hp"s) {
        return return_object(summon->GetMaxHealth());
    }
    else if (key == "max_mp"s) {
        return return_object(summon->GetMaxMana());
    }
    else if (key == "evolution_depth"s) {
        return return_object(summon->m_nTransform);
    }
    else if (key == "level"s) {
        return return_object(summon->GetLevel());
    }
    else if (key == "job"s) {
        return return_object(summon->GetSummonCode());
    }
    else if (key == "name"s) {
        return return_object(summon->GetName());
    }
    else if (key == "summon_state"s) {
        return return_object(summon->IsInWorld() ? 1 : 0);
    }
    return return_object((int32_t)0);
}

void XLua::SCRIPT_CreatureEvolution(int32_t slot)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    auto summon = player->GetSummonByHandle(slot);
    if (summon != nullptr) {
        summon->DoEvolution();
    }
}

void XLua::SCRIPT_QuestInfo(int32_t code, sol::variadic_args args)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;
    int32_t textID = 0;
    if (args.size() >= 1)
        textID = args[0].get<int32_t>();
    Messages::SendQuestInformation(player, code, textID, 0);
}

int32_t XLua::SCRIPT_GetQuestProgress(int32_t quest)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return -1;
    return player->GetQuestProgress(quest);
}

int32_t XLua::SCRIPT_GetOwnDungeonID()
{
    return 0;
}

int32_t XLua::SCRIPT_GetSiegeDungeonID()
{
    return 0;
}

void XLua::SCRIPT_StartQuest(int32_t code, sol::variadic_args args)
{
    if (code != 0 && args.size() >= 1) {
        auto player = m_pUnit->As<Player>();
        if (player == nullptr)
            return;

        bool bForce = false;
        int32_t nStartID = args[0].get<int32_t>();
        if (args.size() == 2)
            bForce = args[1].get<bool>();

        player->StartQuest(code, nStartID, bForce);
    }
}

void XLua::SCRIPT_EndQuest(int32_t quest_id, int32_t reward_id, sol::variadic_args args)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    bool bForce{false};
    if (args.size() != 0 && args[0].get_type() == sol::type::boolean)
        bForce = args[0].get<bool>();

    player->EndQuest(quest_id, reward_id, bForce);
}

void XLua::SCRIPT_EnterDungeon(int32_t nDungeonID)
{
    auto pos = sDungeonManager.GetRaidStartPosition(nDungeonID);
    if (pos.GetPositionX() != 0 && pos.GetPositionY() != 0) {
        m_pUnit->As<Player>()->PendWarp((int32_t)pos.GetPositionX(), (int32_t)pos.GetPositionY(), 0);
    }
}

int32_t XLua::SCRIPT_LearnAllSkill()
{
    if (m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return 0;

    if (GameContent::LearnAllSkill((Player *)m_pUnit))
        return 1;
    return 0;
}

void XLua::SCRIPT_WarpToRevivePosition(sol::variadic_args)
{
    auto player = m_pUnit->As<Player>();
    if (player == nullptr)
        return;

    auto revive_pos = player->GetLastTownPosition();

    player->PendWarp((int32_t)revive_pos.GetPositionX(), (int32_t)revive_pos.GetPositionY(), 0);
    player->SetMove(player->GetCurrentPosition(sWorld.GetArTime()), 0, 0);
}

void XLua::SCRIPT_ShowSoulStoneCraftWindow()
{
    if (m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    Messages::ShowSoulStoneCraftWindow(m_pUnit->As<Player>());
}

void XLua::SCRIPT_ShowSoulStoneRepairWindow()
{
    if (m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    Messages::ShowSoulStoneRepairWindow(m_pUnit->As<Player>());
}

void XLua::SCRIPT_OpenStorage()
{
    if (m_pUnit == nullptr || !m_pUnit->IsPlayer())
        return;
    m_pUnit->As<Player>()->OpenStorage();
}

void XLua::SCRIPT_AddState(sol::variadic_args args)
{
    if (args.size() < 3) {
        NG_LOG_ERROR("server.scripting", "SCRIPT_AddState: Invalid Parameters");
        return;
    }

    int32_t nStateCode = args[0].get<int32_t>();
    int32_t nStateLevel = args[1].get<uint8_t>();
    uint32_t nStateTime = args[2].get<uint32_t>();
    Player *player = args.size() == 4 ? Player::FindPlayer(args[3].get<std::string>()) : m_pUnit->As<Player>();
    if (player == nullptr) {
        NG_LOG_ERROR("server.scripting", "SCRIPT_AddState: Invalid Name");
        return;
    }

    player->AddState(SG_NORMAL, (StateCode)nStateCode, player->GetHandle(), nStateLevel, sWorld.GetArTime(), sWorld.GetArTime() + nStateTime, false, 0, "");
}

void XLua::SCRIPT_AddCreatureState(sol::variadic_args args)
{
    if (args.size() < 3) {
        NG_LOG_ERROR("server.scripting", "SCRIPT_AddCreatureState: Invalid Parameters");
        return;
    }

    int32_t nStateCode = args[0].get<int32_t>();
    int32_t nStateLevel = args[1].get<uint8_t>();
    uint32_t nStateTime = args[2].get<uint32_t>();
    Player *player = args.size() == 4 ? Player::FindPlayer(args[3].get<std::string>()) : m_pUnit->As<Player>();
    Summon *summon = player->m_pMainSummon;

    if (summon == nullptr || !summon->IsInWorld()) {
        NG_LOG_ERROR("server.scripting", "SCRIPT_AddCreatureState: Invalid Name");
        return;
    }

    summon->AddState(SG_NORMAL, (StateCode)nStateCode, summon->GetHandle(), nStateLevel, sWorld.GetArTime(), sWorld.GetArTime() + nStateTime, false, 0, "");
}

void XLua::SCRIPT_InsertGold(sol::variadic_args args)
{
    if (m_pUnit == nullptr)
        return;

    Player *pTarget = nullptr;
    uint32_t nGold = 0;

    if (args.size() > 1) {
        pTarget = Player::FindPlayer(args[0].get<std::string>());
        nGold = args[1].get<uint32_t>();
    }
    else {
        pTarget = m_pUnit->IsPlayer() ? m_pUnit->As<Player>() : nullptr;
        nGold = args[0].get<uint32_t>();
    }

    if (pTarget == nullptr)
        return;

    pTarget->ChangeGold(pTarget->GetGold() + nGold);
}
