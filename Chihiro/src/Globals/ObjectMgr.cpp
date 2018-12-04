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

#include <fstream>

#include "DungeonManager.h"
#include "MixManager.h"
#include "GameRule.h"
#include "WorldLocation.h"

ObjectMgr::ObjectMgr() : g_qtBlockInfo(sWorld.getIntConfig(CONFIG_MAP_WIDTH), sWorld.getIntConfig(CONFIG_MAP_HEIGHT))
{
}

void ObjectMgr::InitGameContent()
{
    LoadStatResource();
    LoadItemResource();
    LoadNPCResource();
    LoadMarketResource();
    LoadDropGroupResource();
    LoadMonsterResource();
    LoadFieldPropResource();
    LoadJobLevelBonus();
    LoadStateResource();
    LoadQuestResource();
    LoadQuestLinkResource();
    LoadLevelResource();
    LoadJobResource();
    LoadJobLevelBonus();
    LoadSummonResource();
    LoadSummonLevelResource();
    LoadSummonLevelBonus();
    LoadDungeonResource();
    LoadEnhanceResource();
    LoadMixResource();
    LoadSkillResource();
    LoadSkillJP();
    LoadSkillTreeResource();
    LoadWorldLocation();
    LoadStringResource();
    LoadSummonNameResource();
}

void ObjectMgr::UnloadAll()
{
    g_vWayPoint.clear();
    g_vRespawnInfo.clear();
    _jobTemplateStore.clear();
    _itemTemplateStore.clear();
    _creatureBaseStore.clear();
    _jobBonusStore.clear();
    _summonResourceStore.clear();
    _marketResourceStore.clear();
    _skillTreeResourceStore.clear();
    _summonLevelStore.clear();
    _stringResourceStore.clear();
    _levelResourceStore.clear();
    _skillBaseStore.clear();
    _monsterBaseStore.clear();
    _dropTemplateStore.clear();
    _questTemplateStore.clear();
    _questLinkStore.clear();
    _npcTemplateStore.clear();
    _fieldPropTemplateStore.clear();
    _summonPrefixStore.clear();
    _summonPostfixStore.clear();
    _summonBonusStore.clear();
    _stateTemplateStore.clear();
}

void ObjectMgr::LoadItemResource()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = GameDatabase.Query("SELECT id, type, `group`, class, wear_type, set_id, set_part_flag, rank, level,"
                                            "enhance, socket, status_flag, limit_deva, limit_asura, limit_gaia, limit_fighter,"
                                            "limit_hunter, limit_magician, limit_summoner, use_min_level, use_max_level, target_min_level,"
                                            "target_max_level, `range`, weight, price, endurance, material, summon_id, flag_cashitem,"
                                            "flag_wear, flag_use, flag_target_use, flag_duplicate, flag_drop, flag_trade, flag_sell,"
                                            "flag_storage, flag_overweight, flag_riding, flag_move, flag_sit, flag_enhance, flag_quest,"
                                            "flag_raid, flag_secroute, flag_eventmap, flag_huntaholic, available_period, decrease_type,"
                                            "throw_range, distribute_type, base_type_0, base_var1_0, base_var2_0, base_type_1, base_var1_1,"
                                            "base_var2_1, base_type_2, base_var1_2, base_var2_2, base_type_3, base_var1_3, base_var2_3, "
                                            "opt_type_0, opt_var1_0, opt_var2_0, opt_type_1, opt_var1_1, opt_var2_1, opt_type_2,"
                                            "opt_var1_2, opt_var2_2, opt_type_3, opt_var1_3, opt_var2_3, enhance_0_id, enhance_0_01,"
                                            "enhance_0_02, enhance_0_03, enhance_0_04, enhance_1_id, enhance_1_01, enhance_1_02,"
                                            "enhance_1_03, enhance_1_04, skill_id, state_id, state_level, state_time, state_type, cool_time, "
                                            "cool_time_group, script_text, name_id FROM ItemResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Items. Table `ItemResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field        *fields = result->Fetch();
        ItemTemplate itemTemplate{ };

        itemTemplate.id               = fields[0].GetInt32();
        itemTemplate.type             = fields[1].GetInt32();
        itemTemplate.group            = fields[2].GetInt32();
        itemTemplate.iclass           = fields[3].GetInt32();
        itemTemplate.wear_type        = fields[4].GetInt32();
        itemTemplate.set_id           = fields[5].GetInt32();
        itemTemplate.set_part_flag    = fields[6].GetInt32();
        itemTemplate.rank             = fields[7].GetInt32();
        itemTemplate.level            = fields[8].GetInt32();
        itemTemplate.enhance          = fields[9].GetInt32();
        itemTemplate.socket           = fields[10].GetInt32();
        itemTemplate.status_flag      = fields[11].GetInt32();
        itemTemplate.limit_deva       = fields[12].GetInt32();
        itemTemplate.limit_asura      = fields[13].GetInt32();
        itemTemplate.limit_gaia       = fields[14].GetInt32();
        itemTemplate.limit_fighter    = fields[15].GetInt32();
        itemTemplate.limit_hunter     = fields[16].GetInt32();
        itemTemplate.limit_magician   = fields[17].GetInt32();
        itemTemplate.limit_summoner   = fields[18].GetInt32();
        itemTemplate.use_min_level    = fields[19].GetInt32();
        itemTemplate.use_max_level    = fields[20].GetInt32();
        itemTemplate.target_min_level = fields[21].GetInt32();
        itemTemplate.target_max_level = fields[22].GetInt32();
        itemTemplate.range            = fields[23].GetFloat() * 100;
        itemTemplate.weight           = fields[24].GetFloat();
        itemTemplate.price            = fields[25].GetUInt32();
        itemTemplate.endurance        = fields[26].GetInt32();
        itemTemplate.material         = fields[27].GetInt32();
        itemTemplate.summon_id        = fields[28].GetInt32();
        for (int i = 0; i < 19; i++)
        {
            itemTemplate.flaglist[i] = fields[29 + i].GetUInt8();
        }
        itemTemplate.available_period = fields[48].GetInt32();
        itemTemplate.decrease_type    = fields[49].GetInt16();
        itemTemplate.throw_range      = fields[50].GetFloat();
        itemTemplate.distribute_type  = fields[51].GetUInt8();
        int      y = 52;
        for (int i = 0; i < 4; i++)
        {
            itemTemplate.base_type[i]   = fields[y++].GetInt16();
            itemTemplate.base_var[i][0] = fields[y++].GetFloat();
            itemTemplate.base_var[i][1] = fields[y++].GetFloat();
        }
        y = 64;
        for (int i = 0; i < 4; i++)
        {
            itemTemplate.opt_type[i]   = fields[y++].GetInt16();
            itemTemplate.opt_var[i][0] = fields[y++].GetFloat();
            itemTemplate.opt_var[i][1] = fields[y++].GetFloat();
        }
        y = 76;
        for (int i = 0; i < 2; i++)
        {
            itemTemplate.enhance_id[i]  = fields[y++].GetInt16();
            itemTemplate._enhance[i][0] = fields[y++].GetFloat();
            itemTemplate._enhance[i][1] = fields[y++].GetFloat();
            itemTemplate._enhance[i][2] = fields[y++].GetFloat();
            itemTemplate._enhance[i][3] = fields[y++].GetFloat();
        }

        itemTemplate.skill_id        = fields[86].GetInt32();
        itemTemplate.state_id        = fields[87].GetInt32();
        itemTemplate.state_level     = fields[88].GetInt32();
        itemTemplate.state_time      = fields[89].GetInt32();
        itemTemplate.state_type      = fields[90].GetInt32();
        itemTemplate.cool_time       = fields[91].GetInt32();
        itemTemplate.cool_time_group = fields[92].GetInt16();
        itemTemplate.script_text     = fields[93].GetString();
        itemTemplate.name_id         = fields[94].GetInt32();

        _itemTemplateStore[itemTemplate.id] = itemTemplate;

        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadMonsterResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM MonsterResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Monstertemplates. Table `MonsterResource` is empty!");
        return;
    }

    uint32 count = 0, y = 0;
    do
    {
        Field       *field = result->Fetch();
        int         idx    = 0;
        MonsterBase base{ };
        base.id            = field[idx++].GetInt32();
        base.monster_group = field[idx++].GetInt32();
        base.name_id       = field[idx++].GetInt32();
        base.location_id   = field[idx++].GetInt32();
        idx += 5; // 14 unused columns, mostly for rendering clientside
        base.size  = field[idx++].GetFloat();
        base.scale = field[idx++].GetFloat();
        idx += 7;
        base.level         = field[idx++].GetInt32();
        base.grp           = field[idx++].GetInt32();
        base.magic_type    = field[idx++].GetInt32();
        base.race          = field[idx++].GetInt32();
        base.visible_range = field[idx++].GetInt32() * 12;
        base.chase_range   = field[idx++].GetInt32();
        for (auto &curr : base.flag)
        {
            curr = field[idx++].GetInt32();
        }
        base.monster_type = field[idx++].GetInt32();
        base.stat_id      = field[idx++].GetInt32();
        base.fight_type   = field[idx++].GetInt32();
        idx += 9;
        base.weapon_type         = field[idx++].GetInt32();
        base.attack_motion_speed = field[idx++].GetInt32();
        base.ability             = field[idx++].GetInt32();
        base.standard_walk_speed = field[idx++].GetInt32();
        base.standard_run_speed  = field[idx++].GetInt32();
        base.walk_speed          = field[idx++].GetInt32();
        base.run_speed           = field[idx++].GetInt32();
        base.attack_range        = field[idx++].GetFloat() * 100;
        base.hp                  = field[idx++].GetInt32();
        base.mp                  = field[idx++].GetInt32();
        base.attacK_point        = field[idx++].GetInt32();
        base.magic_point         = field[idx++].GetInt32();
        base.defence             = field[idx++].GetInt32();
        base.magic_defence       = field[idx++].GetInt32();
        base.attack_speed        = field[idx++].GetInt32();
        base.magic_speed         = field[idx++].GetInt32();
        base.accuracy            = field[idx++].GetInt32();
        base.magic_accuracy      = field[idx++].GetInt32();
        base.avoid               = field[idx++].GetInt32();
        base.magic_avoid         = field[idx++].GetInt32();
        base.taming_id           = field[idx++].GetInt32();
        base.taming_percentage   = field[idx++].GetFloat();
        base.taming_exp_mod      = field[idx++].GetFloat();
        for (y = 0; y < 2; y++)
        {
            base.exp[y] = field[idx++].GetInt32();
            base.jp[y]  = field[idx++].GetInt32();
            if (y == 0)
                base.gold_drop_percentage = field[idx++].GetInt32();
            base.gold_min[y] = field[idx++].GetInt32();
            base.gold_max[y] = field[idx++].GetInt32();
            if (y == 0)
                base.chaos_drop_percentage = field[idx++].GetInt32();
            base.chaos_min[y] = field[idx++].GetInt32();
            base.chaos_max[y] = field[idx++].GetInt32();
        }
        for (y = 0; y < 10; y++)
        {
            base.drop_item_id[y]    = field[idx++].GetInt32();
            base.drop_percentage[y] = (int)(field[idx++].GetFloat() * 100000000);
            base.drop_min_count[y]  = field[idx++].GetInt32();
            base.drop_max_count[y]  = field[idx++].GetInt32();
            base.drop_min_level[y]  = field[idx++].GetInt32();
            base.drop_max_level[y]  = field[idx++].GetInt32();
        }
        for (y = 0; y < 4; y++)
        {
            base.skill_id[y]          = field[idx++].GetInt32();
            base.skill_lv[y]          = field[idx++].GetInt32();
            base.skill_probability[y] = field[idx++].GetFloat();
        }
        base.local_flag = field[idx].GetInt32();
        _monsterBaseStore[base.id] = base;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Monstertemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadQuestResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM QuestResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Quests. Table `QuestResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        int   idx    = 0;

        QuestBaseServer q{ };
        q.nCode            = field[idx++].GetInt32();
        q.nQuestTextID     = field[idx++].GetInt32();
        q.nSummaryTextID   = field[idx++].GetInt32();
        q.nStatusTextID    = field[idx++].GetInt32();
        q.nLimitLevel      = field[idx++].GetInt32();
        q.nLimitJobLevel   = field[idx++].GetInt32();
        q.nLimitIndication = field[idx++].GetUInt8();

        int limit_deva  = field[idx++].GetInt32();
        int limit_asura = field[idx++].GetInt32();
        int limit_gaia  = field[idx++].GetInt32();

        int limit_fighter  = field[idx++].GetInt32();
        int limit_hunter   = field[idx++].GetInt32();
        int limit_magician = field[idx++].GetInt32();
        int limit_summoner = field[idx++].GetInt32();

        q.nLimitJob          = field[idx++].GetInt32();
        q.nLimitFavorGroupID = field[idx++].GetInt32();
        q.nLimitFavor        = field[idx++].GetInt32();
        q.bIsRepeatable      = field[idx++].GetUInt8() == 1;
        q.nInvokeCondition   = field[idx++].GetInt32();
        q.nInvokeValue       = field[idx++].GetInt32();
        q.nType              = (QuestType)field[idx++].GetInt32();
        for (int &i : q.nValue)
        {
            i = field[idx++].GetInt32();
        }
        q.nDropGroupID            = field[idx++].GetInt32();
        q.nQuestDifficulty        = field[idx++].GetInt32();
        q.nFavorGroupID           = field[idx++].GetInt32();
        q.nHateGroupID            = field[idx++].GetInt32();
        q.nFavor                  = field[idx++].GetInt32();
        q.nEXP                    = field[idx++].GetUInt64();
        q.nJP                     = field[idx++].GetInt32();
        q.nGold                   = field[idx++].GetInt64();
        q.DefaultReward.nItemCode = field[idx++].GetInt32();
        q.DefaultReward.nLevel    = field[idx++].GetInt32();
        q.DefaultReward.nQuantity = field[idx++].GetInt32();
        for (auto &i : q.OptionalReward)
        {
            i.nItemCode = field[idx++].GetInt32();
            i.nLevel    = field[idx++].GetInt32();
            i.nQuantity = field[idx++].GetInt32();
        }
        for (int  &i : q.nForeQuest)
        {
            i = field[idx++].GetInt32();
        }
        q.bForceCheckType = field[idx++].GetUInt8() != 0;
        q.strAcceptScript = field[idx++].GetString();
        q.strClearScript  = field[idx++].GetString();
        q.strScript       = field[idx].GetString();

        if (limit_asura != 0)
            q.LimitFlag |= 4;
        if (limit_gaia != 0)
            q.LimitFlag |= 8u;
        if (limit_deva != 0)
            q.LimitFlag |= 2u;
        if (limit_hunter != 0)
            q.LimitFlag |= 0x20u;
        if (limit_fighter != 0)
            q.LimitFlag |= 0x10u;
        if (limit_magician != 0)
            q.LimitFlag |= 0x40u;
        if (limit_summoner != 0)
            q.LimitFlag |= 0x80u;

        _questTemplateStore[q.nCode] = q;

        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Quests in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadFieldPropResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM FieldPropResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 FieldProps. Table `FieldPropResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        int   idx    = 0;

        FieldPropTemplate propTemplate{ };
        propTemplate.nPropID     = field[idx++].GetUInt32();
        propTemplate.nPropTextID = field[idx++].GetInt32();
        idx++; // tooltip_id
        propTemplate.nType        = field[idx++].GetInt32();
        propTemplate.nLocalFlag   = field[idx++].GetInt32();
        propTemplate.nCastingTime = field[idx++].GetUInt32() * 100;
        propTemplate.nUseCount    = field[idx++].GetInt32();
        propTemplate.nRegenTime   = field[idx++].GetUInt32() * 100;
        propTemplate.nLifeTime    = field[idx++].GetUInt32() * 100;
        idx += 2; // casting range & target_fx_size
        propTemplate.nMinLevel = field[idx++].GetInt32();
        propTemplate.nMaxLevel = field[idx++].GetInt32();
        int limit_deva     = field[idx++].GetUInt8();
        int limit_asura    = field[idx++].GetUInt8();
        int limit_gaia     = field[idx++].GetUInt8();
        int limit_fighter  = field[idx++].GetUInt8();
        int limit_hunter   = field[idx++].GetUInt8();
        int limit_magician = field[idx++].GetUInt8();
        int limit_summoner = field[idx++].GetUInt8();
        propTemplate.nLimitJobID = field[idx++].GetInt32();
        for (int i = 0; i < 2; i++)
        {
            propTemplate.nActivateID[i]       = field[idx++].GetInt32();
            propTemplate.nActivateValue[i][0] = field[idx++].GetInt32();
            propTemplate.nActivateValue[i][1] = field[idx++].GetInt32();
        }
        propTemplate.nActivateSkillID = field[idx++].GetInt32();
        for (auto &i : propTemplate.drop_info)
        {
            i.code      = field[idx++].GetInt32();
            i.ratio     = (int)(field[idx++].GetFloat() * 100000000);
            i.min_count = field[idx++].GetInt32();
            i.max_count = field[idx++].GetInt32();
            i.min_level = field[idx++].GetInt32();
            i.max_level = field[idx++].GetInt32();
        }

        propTemplate.strScript = field[idx].GetString();

        propTemplate.nLimit = 0;
        if (limit_asura != 0)
            propTemplate.nLimit |= 8;
        if (limit_gaia != 0)
            propTemplate.nLimit |= 0x10;
        if (limit_deva != 0)
            propTemplate.nLimit |= 4;
        if (limit_hunter != 0)
            propTemplate.nLimit |= 0x800;
        if (limit_fighter != 0)
            propTemplate.nLimit |= 0x400;
        if (limit_magician != 0)
            propTemplate.nLimit |= 0x1000;
        if (limit_summoner != 0)
            propTemplate.nLimit |= 0x2000;

        _fieldPropTemplateStore[propTemplate.nPropID] = propTemplate;

        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u FieldProps in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadQuestLinkResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM QuestLinkResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 QuestLinks. Table `QuestLinkResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        int   idx    = 0;

        QuestLink ql{ };
        ql.nNPCID            = field[idx++].GetInt32();
        ql.code              = field[idx++].GetInt32();
        ql.bLF_Start         = field[idx++].GetUInt8() == 1;
        ql.bLF_Progress      = field[idx++].GetUInt8() == 1;
        ql.bLF_End           = field[idx++].GetUInt8() == 1;
        ql.nStartTextID      = field[idx++].GetInt32();
        ql.nInProgressTextID = field[idx++].GetInt32();
        ql.nEndTextID        = field[idx].GetInt32();

        _questLinkStore.emplace_back(ql);

        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u QuestLinks in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadDropGroupResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM DropGroupResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 DropGroups. Table `DropGroupResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field     *field = result->Fetch();
        int       idx    = 0;
        DropGroup dg{ };
        dg.uid = field[idx++].GetInt32();
        for (int i                 = 0; i < MAX_DROP_GROUP; i++)
        {
            dg.drop_item_id[i]    = field[idx++].GetInt32();
            dg.drop_percentage[i] = (int)(field[idx++].GetFloat() * 100000000);
        }
        _dropTemplateStore[dg.uid] = dg;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u DropGroups in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSkillTreeResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM SkillTreeResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Skilltrees. Table `SkillTreeResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field         *field = result->Fetch();
        int           idx    = 0;
        SkillTreeBase base{ };
        base.job_id       = field[idx++].GetInt32();
        base.skill_id     = field[idx++].GetInt32();
        base.min_skill_lv = field[idx++].GetInt32();
        base.max_skill_lv = field[idx++].GetInt32();
        base.lv           = field[idx++].GetInt32();
        base.job_lv       = field[idx++].GetInt32();
        base.jp_ratio     = field[idx++].GetFloat();
        for (int i = 0; i < 3; i++)
        {
            base.need_skill_id[i] = field[idx++].GetInt32();
            base.need_skill_lv[i] = field[idx++].GetInt32();
        }
        RegisterSkillTree(base);
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u SkillTrees in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSkillResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM SkillResource");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Skills. Table `SkillResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        int       idx    = 0;
        Field     *field = result->Fetch();
        SkillBase base{ };
        base.id      = field[idx++].GetInt32();
        base.text_id = field[idx++].GetInt32();
        idx += 2;
        base.is_valid                 = field[idx++].GetInt16();
        base.elemental                = field[idx++].GetUInt8();
        base.is_passive               = field[idx++].GetUInt8();
        base.is_physical_act          = field[idx++].GetUInt8();
        base.is_harmful               = field[idx++].GetUInt8();
        base.is_need_target           = field[idx++].GetUInt8();
        base.is_corpse                = field[idx++].GetUInt8();
        base.is_toggle                = field[idx++].GetUInt8();
        base.toggle_group             = field[idx++].GetInt32();
        base.casting_type             = field[idx++].GetUInt8();
        base.casting_level            = field[idx++].GetUInt8();
        base.cast_range               = field[idx++].GetInt32();
        base.valid_range              = field[idx++].GetInt32();
        base.cost_hp                  = field[idx++].GetInt32();
        base.cost_hp_per_skl          = field[idx++].GetInt32();
        base.cost_mp                  = field[idx++].GetInt32();
        base.cost_mp_per_skl          = field[idx++].GetInt32();
        base.cost_mp_per_enhance      = field[idx++].GetInt32();
        base.cost_hp_per              = field[idx++].GetFloat();
        base.cost_hp_per_skl_per      = field[idx++].GetFloat();
        base.cost_mp_per              = field[idx++].GetFloat();
        base.cost_mp_per_skl_per      = field[idx++].GetFloat();
        base.cost_havoc               = field[idx++].GetInt32();
        base.cost_havoc_per_skl       = field[idx++].GetInt32();
        base.cost_energy              = field[idx++].GetFloat();
        base.cost_energy_per_skl      = field[idx++].GetFloat();
        base.cost_exp                 = field[idx++].GetInt32();
        base.cost_exp_per_enhance     = field[idx++].GetInt32();
        base.cost_jp                  = field[idx++].GetInt32();
        base.cost_jp_per_enhance      = field[idx++].GetInt32();
        base.cost_item                = field[idx++].GetInt32();
        base.cost_item_count          = field[idx++].GetInt32();
        base.cost_item_count_per      = field[idx++].GetInt32();
        base.need_level               = field[idx++].GetInt32();
        base.need_hp                  = field[idx++].GetInt32();
        base.need_mp                  = field[idx++].GetInt32();
        base.need_havoc               = field[idx++].GetInt32();
        base.need_havoc_burst         = field[idx++].GetInt32();
        base.need_state_id            = field[idx++].GetInt16();
        base.need_state_level         = field[idx++].GetInt16();
        base.need_state_exhaust       = field[idx++].GetInt16();
        base.vf_one_hand_sword        = field[idx++].GetUInt8();
        base.vf_two_hand_sword        = field[idx++].GetUInt8();
        base.vf_double_sword          = field[idx++].GetUInt8();
        base.vf_dagger                = field[idx++].GetUInt8();
        base.vf_double_dagger         = field[idx++].GetUInt8();
        base.vf_spear                 = field[idx++].GetUInt8();
        base.vf_axe                   = field[idx++].GetUInt8();
        base.vf_one_hand_axe          = field[idx++].GetUInt8();
        base.vf_double_axe            = field[idx++].GetUInt8();
        base.vf_one_hand_mace         = field[idx++].GetUInt8();
        base.vf_two_hand_mace         = field[idx++].GetUInt8();
        base.vf_lightbow              = field[idx++].GetUInt8();
        base.vf_heavybow              = field[idx++].GetUInt8();
        base.vf_crossbow              = field[idx++].GetUInt8();
        base.vf_one_hand_staff        = field[idx++].GetUInt8();
        base.vf_two_hand_staff        = field[idx++].GetUInt8();
        base.vf_shield_only           = field[idx++].GetUInt8();
        base.vf_is_not_need_weapon    = field[idx++].GetUInt8();
        base.delay_cast               = field[idx++].GetFloat() * 100;
        base.delay_cast_per_skl       = field[idx++].GetFloat() * 100;
        base.delay_cast_mode_per      = field[idx++].GetFloat();
        base.delay_common             = field[idx++].GetFloat() * 100;
        base.delay_cooltime           = field[idx++].GetFloat() * 100;
        base.delay_cooltime_mode      = field[idx++].GetFloat();
        base.cool_time_group_id       = field[idx++].GetInt32();
        base.uf_self                  = field[idx++].GetUInt8();
        base.uf_party                 = field[idx++].GetUInt8();
        base.uf_guild                 = field[idx++].GetUInt8();
        base.uf_neutral               = field[idx++].GetUInt8();
        base.uf_purple                = field[idx++].GetUInt8();
        base.uf_enemy                 = field[idx++].GetUInt8();
        base.tf_avatar                = field[idx++].GetUInt8();
        base.tf_summon                = field[idx++].GetUInt8();
        base.tf_monster               = field[idx++].GetUInt8();
        base.target                   = field[idx++].GetInt16();
        base.effect_type              = field[idx++].GetInt16();
        base.state_id                 = field[idx++].GetInt32();
        base.state_level_base         = field[idx++].GetInt32();
        base.state_level_per_skl      = field[idx++].GetFloat();
        base.state_level_per_enhance  = field[idx++].GetFloat();
        base.state_second             = field[idx++].GetFloat() * 100;
        base.state_second_per_level   = field[idx++].GetFloat() * 100;
        base.state_second_per_enhance = field[idx++].GetFloat() * 100;
        base.state_type               = field[idx++].GetUInt8();
        base.probability_on_hit       = field[idx++].GetInt32();
        base.probability_inc_by_slv   = field[idx++].GetInt32();
        base.hit_bonus                = field[idx++].GetInt16();
        base.hit_bonus_per_enhance    = field[idx++].GetInt16();
        base.percentage               = field[idx++].GetInt16();
        base.hate_mod                 = field[idx++].GetFloat();
        base.hate_basic               = field[idx++].GetInt16();
        base.hate_per_skl             = field[idx++].GetFloat();
        base.hate_per_enhance         = field[idx++].GetFloat();
        base.critical_bonus           = field[idx++].GetInt32();
        base.critical_bonus_per_skl   = field[idx++].GetInt32();
        for (float &i : base.var)
        {
            i = field[idx++].GetFloat();
        }
        idx += 2;
        base.is_projectile           = field[idx++].GetInt16();
        base.projectile_speed        = field[idx++].GetFloat();
        base.projectile_acceleration = field[idx].GetFloat();
        _skillBaseStore[base.id] = base;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Skills in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadDungeonResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM DungeonResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Dungeons. Table `DungeonResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field           *field = result->Fetch();
        int             idx    = 0;
        DungeonTemplate base{ };
        base.id = field[idx++].GetInt32();
        idx += 2;
        int start_pos_x = field[idx++].GetInt32();
        int start_pos_y = field[idx++].GetInt32();
        base.raid_start_pos = Position{(float)start_pos_x, (float)start_pos_y};
        int defense_pos_x = field[idx++].GetInt32();
        int defense_pos_y = field[idx++].GetInt32();
        base.siege_defense_pos = Position{(float)defense_pos_x, (float)defense_pos_y};
        base.connector_id      = field[idx++].GetInt32();
        int connector_x = field[idx++].GetInt32();
        int connector_y = field[idx++].GetInt32();
        base.connector_pos = Position{(float)connector_x, (float)connector_y};
        idx++;
        base.core_id = field[idx++].GetInt32();
        int core_pos_x = field[idx++].GetInt32();
        int core_pos_y = field[idx++].GetInt32();
        base.core_pos            = Position{(float)core_pos_x, (float)core_pos_y};
        base.core_offset_x       = field[idx++].GetFloat();
        base.core_offset_y       = field[idx++].GetFloat();
        base.core_offset_z       = field[idx++].GetFloat();
        base.core_around_x       = field[idx++].GetFloat();
        base.core_around_y       = field[idx++].GetFloat();
        base.core_around_z       = field[idx++].GetFloat();
        base.core_scale_x        = field[idx++].GetFloat();
        base.core_scale_y        = field[idx++].GetFloat();
        base.core_scale_z        = field[idx++].GetFloat();
        base.core_is_lock_height = field[idx++].GetUInt8() != 0;
        base.core_lock_height    = field[idx++].GetFloat();
        for (auto &boss : base.boss_id)
            boss = field[idx++].GetInt32();
        base.raid_start_time = field[idx++].GetInt32();
        base.raid_end_time   = field[idx++].GetInt32();
        base.start_time      = field[idx++].GetInt32();
        base.end_time        = field[idx++].GetInt32();
        int seamless_x = field[idx++].GetInt32();
        int seamless_y = field[idx++].GetInt32();
        base.max_guild_party = field[idx++].GetInt32();
        idx++;
        base.max_raid_party = field[idx].GetInt32();
        base.box.begin      = {seamless_x * sWorld.getFloatConfig(CONFIG_MAP_LENGTH), seamless_y * sWorld.getFloatConfig(CONFIG_MAP_LENGTH)};
        base.box.end        = {(seamless_x + 1) * sWorld.getFloatConfig(CONFIG_MAP_LENGTH), (seamless_y + 1) * sWorld.getFloatConfig(CONFIG_MAP_LENGTH)};
        sDungeonManager.RegisterDungeonTemplate(base);
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Dungeons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadLevelResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM LevelResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Leveltemplates. DB packetHandler `LevelResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field                 *field = result->Fetch();
        LevelResourceTemplate base{ };
        base.level      = field[0].GetInt32();
        base.normal_exp = field[1].GetInt64();
        for (int i = 0; i < 4; i++)
            base.jlv[i]                 = field[2 + i].GetInt32();
        _levelResourceStore[base.level] = base;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Leveltemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadStateResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM StateResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 States. Table `StateResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field         *field = result->Fetch();
        int           idx    = 0;
        StateTemplate state{ };
        state.state_id        = field[idx++].GetInt32();
        state.text_id         = field[idx++].GetInt32();
        state.tooltip_id      = field[idx++].GetInt32();
        state.is_harmful      = field[idx++].GetUInt8();
        state.state_time_type = field[idx++].GetInt32();
        state.state_group     = field[idx++].GetInt32();
        for (auto &dg : state.duplicate_group)
            dg = field[idx++].GetInt32();
        state.uf_avatar          = field[idx++].GetUInt8();
        state.uf_summon          = field[idx++].GetUInt8();
        state.uf_monster         = field[idx++].GetUInt8();
        state.base_effect_id     = field[idx++].GetInt32();
        state.fire_interval      = field[idx++].GetInt32();
        state.elemental_type     = field[idx++].GetInt32();
        state.amplify_base       = field[idx++].GetFloat();
        state.amplify_per_skl    = field[idx++].GetFloat();
        state.add_damage_base    = field[idx++].GetInt32();
        state.add_damage_per_skl = field[idx++].GetInt32();
        state.effect_type        = field[idx++].GetInt32();
        for (auto &val : state.value)
            val = field[idx++].GetFloat();

        _stateTemplateStore[state.state_id] = state;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u States in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadStatResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, str, vit, dex, agi, `int`, men, luk FROM StatResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Stats. Table `StatResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field        *field = result->Fetch();
        CreatureStat stat{ };
        stat.stat_id      = field[0].GetInt16();
        stat.strength     = field[1].GetInt32();
        stat.vital        = field[2].GetInt32();
        stat.dexterity    = field[3].GetInt32();
        stat.agility      = field[4].GetInt32();
        stat.intelligence = field[5].GetInt32();
        stat.mentality    = field[6].GetInt32();
        stat.luck         = field[7].GetInt32();
        _creatureBaseStore[stat.stat_id] = stat;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Stats in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadMarketResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT sort_id, name, code, price_ratio, huntaholic_ratio FROM MarketResource ORDER BY name, sort_id;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Markettemplates. Table `MarketResource` is empty!");
        return;
    }

    uint32                  count = 0;
    std::string             lastMarket{ };
    std::vector<MarketInfo> vContainer{ };
    do
    {
        Field *field = result->Fetch();

        MarketInfo info{ };
        info.sort_id          = field[0].GetInt32();
        info.name             = field[1].GetString();
        info.code             = field[2].GetUInt32();
        info.price_ratio      = field[3].GetFloat();
        info.huntaholic_ratio = field[4].GetFloat();

        auto itemBase = GetItemBase(info.code);
        info.price_ratio      = floor(info.price_ratio * itemBase->price);
        info.huntaholic_ratio = 0;

        if (lastMarket.empty())
            lastMarket = info.name;

        if (lastMarket != info.name)
        {
            _marketResourceStore[lastMarket] = vContainer;
            lastMarket = info.name;
            vContainer.clear();
        }
        vContainer.push_back(info);

        ++count;
    } while (result->NextRow());

    _marketResourceStore[lastMarket] = vContainer;

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Markettemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadJobResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, stati_id, job_class, job_depth, up_lv, up_jlv,"
                                               "available_job_0, available_job_1, available_job_2,"
                                               "available_job_3 FROM JobResource;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Jobs. Table `JobResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        int                 i      = 0;
        Field               *field = result->Fetch();
        JobResourceTemplate job{ };
        job.id        = field[i++].GetInt32();
        job.stat_id   = field[i++].GetUInt32();
        job.job_class = field[i++].GetInt32();
        job.job_depth = field[i++].GetUInt32();
        job.up_lv     = field[i++].GetInt32();
        job.up_jlv    = field[i++].GetInt32();
        for (int &j : job.available_job)
        {
            j = field[i++].GetInt32();
        }
        _jobTemplateStore[job.id] = job;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Jobs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSummonLevelResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT level, normal_exp FROM SummonLevelResource ORDER BY level ASC");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 SummonLeveltemplates. Table `SummonLevelResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        auto  level  = field[0].GetInt32();
        auto  exp    = field[1].GetUInt64();
        _summonLevelStore[level] = exp;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u SummonLeveltemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSummonLevelBonus()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT summon_id, strength, vital, dexterity, agility, intelligence, mentality, luck FROM CreatureLevelBonus;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 SummonLevelBonus templates. Table `CreatureLevelBonus` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field            *field = result->Fetch();
        SummonLevelBonus bonus{ };

        int i = 0;
        bonus.summon_id    = field[i++].GetInt32();
        bonus.strength     = field[i++].GetFloat();
        bonus.vital        = field[i++].GetFloat();
        bonus.dexterity    = field[i++].GetFloat();
        bonus.agility      = field[i++].GetFloat();
        bonus.intelligence = field[i++].GetFloat();
        bonus.mentality    = field[i++].GetFloat();
        bonus.luck         = field[i].GetFloat();
        _summonBonusStore[bonus.summon_id] = bonus;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u SummonLevelBonus templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadJobLevelBonus()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT job_id, "
                                               "strength_0, vital_0, dexterity_0, agility_0, intelligence_0, mentality_0, luck_0, "
                                               "strength_1, vital_1, dexterity_1, agility_1, intelligence_1, mentality_1, luck_1, "
                                               "strength_2, vital_2, dexterity_2, agility_2, intelligence_2, mentality_2, luck_2, "
                                               "strength_3, vital_3, dexterity_3, agility_3, intelligence_3, mentality_3, luck_3 "
                                               "FROM JobLevelBonus;");
    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 job level bonus templates. Table `JobLevelBonus` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field                 *field = result->Fetch();
        JobLevelBonusTemplate bonus{ };

        int i = 0, j = 0;
        bonus.job_id = field[i++].GetInt32();
        for (j                       = 0; j < 4; j++)
        {
            bonus.strength[j]     = field[i++].GetFloat();
            bonus.vital[j]        = field[i++].GetFloat();
            bonus.dexterity[j]    = field[i++].GetFloat();
            bonus.agility[j]      = field[i++].GetFloat();
            bonus.intelligence[j] = field[i++].GetFloat();
            bonus.mentality[j]    = field[i++].GetFloat();
            bonus.luck[j]         = field[i++].GetFloat();
        }
        _jobBonusStore[bonus.job_id] = bonus;
        ++count;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u job level bonus templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadNPCResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, x, y, z, face, local_flag, contact_script, spawn_type FROM NPCResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 NPCs. Table `NPCResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field       *field = result->Fetch();
        NPCTemplate npc{ };
        // SELECT id, x, y, z, face, local_flag, contact_script FROM npcresource;
        npc.id             = field[0].GetUInt32();
        npc.x              = field[1].GetUInt32();
        npc.y              = field[2].GetUInt32();
        npc.z              = field[3].GetUInt32();
        npc.face           = field[4].GetUInt32();
        npc.local_flag     = field[5].GetUInt32();
        npc.contact_script = field[6].GetString();
        npc.spawn_type     = field[7].GetInt32();

        _npcTemplateStore[npc.id] = npc;
        ++count;

    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u NPCs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSkillJP()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM SkillJPResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 SkillJPTemplates. Table `SkillJPResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field     *field   = result->Fetch();
        int       off      = 0;
        int       skill_id = field[off++].GetInt32();
        SkillBase *sb      = &_skillBaseStore[skill_id];
        if (sb->id != 0)
        {
            for (int &v : sb->m_need_jp)
            {
                v = field[off++].GetInt32();
            }
        }
        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u SkillJPTemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadWorldLocation()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, location_type, time_id, weather_id, weather_ratio, weather_change_time FROM WorldLocation;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 WorldLocation templates. Table `WorldLocation` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field              = result->Fetch();
        auto  idx                 = field[0].GetUInt32();
        auto  location_type       = field[1].GetUInt8();
        auto  time_idx            = field[2].GetUInt32();
        auto  weather_id          = field[3].GetUInt32();
        auto  weather_ratio       = field[4].GetUInt8();
        auto  weather_change_time = (field[5].GetUInt32()) * 6000;
        sWorldLocationMgr.RegisterWorldLocation(idx, location_type, time_idx, weather_id, weather_ratio, weather_change_time, 0);
        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u WorldLocation in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadEnhanceResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM EnhanceResource");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Enhancetemplates. Table `EnhanceResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        int   idx    = 0;

        EnhanceInfo info{ };
        info.nSID          = field[idx++].GetInt32();
        info.Flag          = field[idx++].GetUInt32();
        info.nFailResult   = field[idx++].GetUInt8();
        info.nMaxEnhance   = field[idx++].GetInt32();
        info.nLocalFlag    = field[idx++].GetUInt32();
        info.nNeedItemCode = field[idx++].GetInt32();
        for (auto &perc : info.fPercentage)
        {
            perc = field[idx++].GetFloat();
        }
        if ((GameRule::GetLocalFlag() & info.nLocalFlag) != 0)
        {
            sMixManager.RegisterEnhanceInfo(info);
        }

        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u Enhancetemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadMixResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT * FROM MixResource");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Mixtemplates. Table `MixResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();
        int   idx    = 0;

        MixBase info{ };
        info.id   = field[idx++].GetInt32();
        info.type = field[idx++].GetInt32();
        for (int &i : info.value)
        {
            i = field[idx++].GetInt32();
        }
        info.sub_material_cnt = field[idx++].GetInt32();
        for (int  i = 0; i < MATERIAL_INFO_COUNT; ++i)
        {
            info.main_material.type[i]  = field[idx++].GetInt32();
            info.main_material.value[i] = field[idx++].GetInt32();
        }
        for (auto &i : info.sub_material)
        {
            for (int j = 0; j < MATERIAL_INFO_COUNT; ++j)
            {
                i.type[j]  = field[idx++].GetInt32();
                i.value[j] = field[idx++].GetInt32();
            }
        }
        sMixManager.RegisterMixInfo(info);

        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u Mixtemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSummonResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, type, magic_type, rate, stat_id, size, scale, standard_walk_speed, standard_run_speed,"
                                               "walk_speed, run_speed, is_riding_only, attack_range, material, weapon_type,"
                                               "form, evolve_target, card_id FROM SummonResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Summons. Table `SummonResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field                  *field = result->Fetch();
        SummonResourceTemplate summon{ };
        int                    i      = 0;
        summon.id                  = field[i++].GetInt32();
        summon.type                = field[i++].GetInt32();
        summon.magic_type          = field[i++].GetInt32();
        summon.rate                = field[i++].GetInt32();
        summon.stat_id             = field[i++].GetInt32();
        summon.size                = field[i++].GetFloat();
        summon.scale               = field[i++].GetFloat();
        summon.standard_walk_speed = field[i++].GetInt32();
        summon.standard_run_speed  = field[i++].GetInt32();
        summon.walk_speed          = field[i++].GetInt32();
        summon.run_speed           = field[i++].GetInt32();
        summon.is_riding_only      = field[i++].GetBool();
        summon.attack_range        = field[i++].GetFloat();
        summon.material            = field[i++].GetInt32();
        summon.weapon_type         = field[i++].GetInt32();
        summon.form                = field[i++].GetInt32();
        summon.evolve_target       = field[i++].GetInt32();
        summon.card_id             = field[i].GetInt32();
        _summonResourceStore[summon.id] = summon;
        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u Summons in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadStringResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT code, value FROM StringResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Strings. Table `StringResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();

        int               code  = field[0].GetInt32();
        const std::string value = field[1].GetString();
        _stringResourceStore[code] = value;

        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u Strings in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ObjectMgr::LoadSummonNameResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT is_post_fix, text_id FROM SummonDefaultNameResource;");

    if (!result)
    {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 SummonDefaultNames. Table `SummonDefaultNameResource` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field *field = result->Fetch();

        bool isPostFix = field[0].GetInt32() != 0;
        int  text_id   = field[1].GetInt32();
        if (!isPostFix)
            _summonPrefixStore.emplace_back(text_id);
        else
            _summonPostfixStore.emplace_back(text_id);

        ++count;
    } while (result->NextRow());
    NG_LOG_INFO("server.worldserver", ">> Loaded %u SummonDefaultNames in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

CreatureStat *ObjectMgr::GetStatInfo(const int stat_id)
{
    if (_creatureBaseStore.count(stat_id) == 1)
        return &_creatureBaseStore[stat_id];
    return nullptr;
}

ItemTemplate *ObjectMgr::GetItemBase(const int item_id)
{
    if (_itemTemplateStore.count(item_id) == 1)
        return &_itemTemplateStore[item_id];
    return nullptr;
}

FieldPropTemplate *ObjectMgr::GetFieldPropBase(int idx)
{
    if (_fieldPropTemplateStore.count(idx) == 1)
        return &_fieldPropTemplateStore[idx];
    return nullptr;
}

void ObjectMgr::AddWayPoint(int waypoint_id, float x, float y)
{
    Position pos{ };
    pos.Relocate(x, y, 0);

    for (auto &wpi : g_vWayPoint)
    {
        if (wpi.first == waypoint_id)
        {
            wpi.second.vWayPoint.emplace_back(pos);
            return;
        }
    }

    WayPointInfo wp{ };
    wp.way_point_id    = waypoint_id;
    wp.way_point_speed = 0;
    wp.way_point_type  = 1;
    wp.vWayPoint.emplace_back(pos);
    g_vWayPoint[waypoint_id] = wp;
}

void ObjectMgr::SetWayPointType(int waypoint_id, int type)
{
    for (const auto &wpi : g_vWayPoint)
    {
        if (wpi.first == waypoint_id)
            return;
    }

    WayPointInfo info{ };
    info.way_point_speed = 0;
    info.way_point_type  = type;
    info.way_point_id    = waypoint_id;
    g_vWayPoint[waypoint_id] = info;

}

WayPointInfo *ObjectMgr::GetWayPoint(int waypoint_id)
{
    if (g_vWayPoint.count(waypoint_id) != 0)
        return &g_vWayPoint[waypoint_id];
    return nullptr;
}

DropGroup *ObjectMgr::GetDropGroupInfo(int drop_group_id)
{
    if (_dropTemplateStore.count(drop_group_id) == 1)
        return &_dropTemplateStore[drop_group_id];
    return nullptr;
}

void ObjectMgr::RegisterMonsterRespawnInfo(MonsterRespawnInfo info)
{
    if (_monsterBaseStore.count(info.monster_id))
        g_vRespawnInfo.emplace_back(info);
    else
        NG_LOG_WARN("misc", "[respawn_rare_mob] Monster %d does not exist!", info.monster_id);
}

CreatureStat ObjectMgr::GetJobLevelBonus(int depth, int jobs[], const int levels[])
{
    CreatureStat stat{ };
    if (depth >= 0)
    {
        for (int i = 0; i < 4; i++)
        {
            if (_jobBonusStore.count((uint)jobs[i]))
            {
                auto jlb = _jobBonusStore[jobs[i]];

                float v1 = (levels[i] - 20);
                if (levels[i] > 40)
                    v1 = 20;

                float v2 = (levels[i] - 40);
                if (levels[i] >= 50)
                    v2 = 10;

                float v0 = levels[i];
                if (levels[i] >= 20)
                    v0 = 20;

                if (v1 <= 0)
                    v1 = 0;

                if (v2 <= 0)
                    v2 = 0;
                stat.strength += (int)((v1 * jlb.strength[1]) + (v2 * jlb.strength[2]) + (jlb.strength[3]) + (v0 * jlb.strength[0]));
                stat.vital += (int)((v1 * jlb.vital[1]) + (v2 * jlb.vital[2]) + (jlb.vital[3]) + (v0 * jlb.vital[0]));
                stat.dexterity += (int)((v1 * jlb.dexterity[1]) + (v2 * jlb.dexterity[2]) + (jlb.dexterity[3]) + (v0 * jlb.dexterity[0]));
                stat.agility += (int)((v1 * jlb.agility[1]) + (v2 * jlb.agility[2]) + (jlb.agility[3]) + (v0 * jlb.agility[0]));
                stat.intelligence += (int)((v1 * jlb.intelligence[1]) + (v2 * jlb.intelligence[2]) + (jlb.intelligence[3]) + (v0 * jlb.intelligence[0]));
                stat.mentality += (int)((v1 * jlb.mentality[1]) + (v2 * jlb.mentality[2]) + (jlb.mentality[3]) + (v0 * jlb.mentality[0]));
                stat.luck += (int)((v1 * jlb.luck[1]) + (v2 * jlb.luck[2]) + (jlb.luck[3]) + (v0 * jlb.luck[0]));
            }
        }
    }
    return stat;
}

JobResourceTemplate *ObjectMgr::GetJobInfo(const int job_id)
{
    if (_jobTemplateStore.count(job_id) == 1)
        return &_jobTemplateStore[job_id];
    return nullptr;
}

SummonResourceTemplate *ObjectMgr::GetSummonBase(const int idx)
{
    if (_summonResourceStore.count(idx) == 1)
        return &_summonResourceStore[idx];
    return nullptr;
}

std::vector<MarketInfo> *ObjectMgr::GetMarketInfo(const std::string &szKey)
{
    if (_marketResourceStore.count(szKey) == 1)
        return &_marketResourceStore[szKey];
    return nullptr;
}

int ObjectMgr::GetNeedJpForJobLevelUp(const int jlv, const int depth)
{
    if (depth > 3 || jlv > 49)
        return 0;
    return _levelResourceStore[jlv].jlv[depth];
}

void ObjectMgr::RegisterSkillTree(SkillTreeBase base)
{
    for (auto      &stg : _skillTreeResourceStore)
    {
        if (stg.skill_id == base.skill_id && stg.job_id == base.job_id)
        {
            stg.skillTrees.emplace_back(base);
            return;
        }
    }
    SkillTreeGroup g{ };
    g.job_id   = base.job_id;
    g.skill_id = base.skill_id;
    g.skillTrees.emplace_back(base);
    _skillTreeResourceStore.emplace_back(g);
}

std::vector<SkillTreeBase> ObjectMgr::getSkillTree(int job_id)
{
    std::vector<SkillTreeBase> skills{ };
    for (auto                  &stg : _skillTreeResourceStore)
    {
        if (stg.job_id == job_id)
        {
            for (auto st : stg.skillTrees)
            {
                skills.emplace_back(st);
            }
        }
    }
    return skills;
}

int ObjectMgr::GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID)
{
    auto                       pSkillBase = GetSkillBase((uint)skill_id);
    std::vector<SkillTreeBase> trees      = getSkillTree(nJobID);
    float                      jp_ratio   = -1.0f;
    if (pSkillBase->id != 0 && skill_level <= 50 && !trees.empty())
    {
        for (auto st : trees)
        {
            if (st.skill_id == skill_id && st.max_skill_lv >= skill_level)
            {
                jp_ratio = st.jp_ratio;
            }
        }
        if (jp_ratio == -1.0f)
            jp_ratio = 1.0f;
        return (int)(pSkillBase->GetNeedJobPoint(skill_level) * jp_ratio);
    }
    return -1;
}

SkillBase *ObjectMgr::GetSkillBase(const int skill_id)
{
    if (_skillBaseStore.count(skill_id) == 1)
        return &_skillBaseStore[skill_id];
    return nullptr;
}

int64 ObjectMgr::GetNeedExp(int level)
{
    int l = level;
    if (l < 1)
        l = 1;
    if (l > 300)
        l = 300;
    if ((int)_levelResourceStore.size() < l)
        l = (int)(_levelResourceStore.size() - 1);
    return _levelResourceStore[l - 1].normal_exp;
}

MonsterBase *ObjectMgr::GetMonsterInfo(int idx)
{
    if (_monsterBaseStore.count(idx) == 1)
        return &_monsterBaseStore[idx];
    return nullptr;
}

int64 ObjectMgr::GetNeedSummonExp(int level)
{
    if (level <= 300 && level > 0)
        return _summonLevelStore[level];
    return 0;
}

QuestBaseServer *ObjectMgr::GetQuestBase(int code)
{
    if (_questTemplateStore.count(code) == 1)
        return &_questTemplateStore[code];

    return nullptr;
}

bool ObjectMgr::checkQuestTypeFlag(QuestType type, int flag)
{
    switch (type)
    {
        case QuestType::QUEST_MISC:
            return (flag & 1) != 0;

        case QuestType::QUEST_KILL_TOTAL:
            return (flag & 2) != 0;

        case QuestType::QUEST_KILL_INDIVIDUAL:
            return (flag & 4) != 0;

        case QuestType::QUEST_COLLECT:
            return (flag & 8) != 0;

        case QuestType::QUEST_HUNT_ITEM:
            return (flag & 0x10) != 0;

        case QuestType::QUEST_HUNT_ITEM_FROM_ANY_MONSTERS:
            return (flag & 0x1000) != 0;

        case QuestType::QUEST_LEARN_SKILL:
            return (flag & 0x20) != 0;

        case QuestType::QUEST_UPGRADE_ITEM:
            return (flag & 0x40) != 0;

        case QuestType::QUEST_CONTACT:
            return (flag & 0x80) != 0;

        case QuestType::QUEST_JOB_LEVEL:
            return (flag & 0x100) != 0;

        case QuestType::QUEST_PARAMETER:
            return (flag & 0x200) != 0;

        case QuestType::QUEST_RANDOM_KILL_INDIVIDUAL:
            return (flag & 0x400) != 0;

        case QuestType::QUEST_RANDOM_COLLECT:
            return (flag & 0x800) != 0;

        default:
            return false;
    }

}

QuestLink *ObjectMgr::GetQuestLink(int code, int start_id)
{
    auto l = std::find_if(_questLinkStore.begin(),
                          _questLinkStore.end(),
                          [&code, start_id](const QuestLink &ql) {
                              return ql.code == code && (ql.nStartTextID == start_id || start_id == 0);
                          });
    return l != _questLinkStore.end() ? &*l : nullptr;
}

StateTemplate *ObjectMgr::GetStateInfo(int code)
{
    if (_stateTemplateStore.count(code) == 1)
        return &_stateTemplateStore[code];
    return nullptr;
}

CreatureStat ObjectMgr::GetSummonLevelBonus(int summon_code, int growth_depth /* evolve_type*/, int level)
{
    if (_summonBonusStore.count(summon_code) == 0)
        return CreatureStat{ };

    CreatureStat stat{ };
    auto         bonus = _summonBonusStore[summon_code];
    if (growth_depth != 1)
    {
        level = level - 50 * growth_depth + 51;
    }

    stat.stat_id      = (short)summon_code;
    stat.strength     = bonus.strength * level;
    stat.vital        = bonus.vital * level;
    stat.dexterity    = bonus.dexterity * level;
    stat.agility      = bonus.agility * level;
    stat.intelligence = bonus.intelligence * level;
    stat.mentality    = bonus.mentality * level;
    stat.luck         = bonus.luck * level;

    return stat;
}

const std::string &ObjectMgr::GetValueFromNameID(const int name_id)
{
    static const std::string empty = { };
    if (_stringResourceStore.count(name_id) != 0)
        return _stringResourceStore[name_id];
    return empty;
}

std::string ObjectMgr::GetSummonName()
{
    int pre  = irand(0, (int)_summonPrefixStore.size() - 1);
    int post = irand(0, (int)_summonPostfixStore.size() - 1);
    return NGemity::StringFormat("%s%s", GetValueFromNameID(_summonPrefixStore[pre]), GetValueFromNameID(_summonPostfixStore[post]));
}

