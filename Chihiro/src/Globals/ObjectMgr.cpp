#include "ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "Map/ArRegion.h"
#include "Utilities/Timer.h"
#include "MemPool.h"
#include "TerrainSeamlessWorld.h"
#include <fstream>

bool ObjectMgr::InitGameContent()
{
    if(!LoadMapContent() || !LoadWorldLocation())
        return false;
    if (!LoadStatResource())
        return false;
    else if (!LoadItemResource())
        return false;
    else if (!LoadNPCResource())
        return false;
    else if(!LoadMarketResource())
        return false;
    else if (!LoadMonsterResource())
        return false;
    else if (!LoadJobLevelBonus())
        return false;
    else if (!LoadJobResource())
        return false;
    else if (!LoadSummonResource())
        return false;
    else
        return true;
}

bool ObjectMgr::LoadItemResource()
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
                                                    "cool_time_group, script_text FROM ItemResource;");

    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 item templates. DB packetHandler `ItemResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
        Field        *fields = result->Fetch();
        ItemTemplate itemTemplate;

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
        itemTemplate.range            = fields[23].GetFloat();
        itemTemplate.weight           = fields[24].GetFloat();
        itemTemplate.price            = fields[25].GetUInt32();
        itemTemplate.endurance        = fields[26].GetInt32();
        itemTemplate.material         = fields[27].GetInt32();
        itemTemplate.summon_id        = fields[28].GetInt32();
        for (int i = 0; i < 19; i++) {
            itemTemplate.flaglist[i] = fields[29 + i].GetUInt8();
        }
        itemTemplate.available_period = fields[48].GetInt32();
        itemTemplate.decrease_type    = fields[49].GetInt16();
        itemTemplate.throw_range      = fields[50].GetFloat();
        itemTemplate.distribute_type  = fields[51].GetUInt8();
        int      y = 52;
        for (int i = 0; i < 4; i++) {
            itemTemplate.base_type[i]   = fields[y++].GetInt16();
            itemTemplate.base_var[i][0] = fields[y++].GetFloat();
            itemTemplate.base_var[i][1] = fields[y++].GetFloat();
        }
        y = 64;
        for (int i = 0; i < 4; i++) {
            itemTemplate.opt_type[i]   = fields[y++].GetInt16();
            itemTemplate.opt_var[i][0] = fields[y++].GetFloat();
            itemTemplate.opt_var[i][1] = fields[y++].GetFloat();
        }
        y = 76;
        for (int i = 0; i < 2; i++) {
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

        _itemTemplateStore[itemTemplate.id] = itemTemplate;

        ++count;
    } while (result->NextRow());

    MX_LOG_INFO("server.worldserver", ">> Loaded %u item templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}

bool ObjectMgr::LoadMonsterResource()
{
    return true;
}

bool ObjectMgr::LoadStatResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, str, vit, dex, agi, `int`, men, luk FROM StatResource;");
    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 Stat templates. DB packetHandler `StatResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
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

    MX_LOG_INFO("server.worldserver", ">> Loaded %u Stat templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}

bool ObjectMgr::LoadMarketResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT sort_id, name, code, price_ratio, huntaholic_ratio FROM MarketResource ORDER BY name, sort_id;");
    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 Markettemplates. DB packetHandler `MarketResource` is empty!");
        return false;
    }

    uint32 count = 0;
    std::string lastMarket = "";
    std::vector<MarketInfo> vContainer{};
    do {
        Field *field = result->Fetch();

        MarketInfo info {};
        info.sort_id = field[0].GetInt32();
        info.name = field[1].GetString();
        info.code = field[2].GetInt32();
        info.price_ratio = field[3].GetFloat();
        info.huntaholic_ratio = field[4].GetFloat();

        auto itemBase = GetItemBase(info.code);
        info.price_ratio = floor(info.price_ratio * itemBase.price);
        info.huntaholic_ratio = 0;

        if(lastMarket.empty())
            lastMarket = info.name;

        if(lastMarket != info.name) {
            _marketResourceStore[lastMarket] = vContainer;
            lastMarket = info.name;
            vContainer.clear();
        }
        vContainer.push_back(info);

        ++count;
    } while (result->NextRow());

    _marketResourceStore[lastMarket] = vContainer;

    MX_LOG_INFO("server.worldserver", ">> Loaded %u Markettemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}


bool ObjectMgr::LoadJobResource()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, stati_id, job_class, job_depth, up_lv, up_jlv,"
                                                       "available_job_0, available_job_1, available_job_2,"
                                                       "available_job_3 FROM JobResource;");
    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 job templates. DB packetHandler `JobResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
        int                 i      = 0;
        Field               *field = result->Fetch();
        JobResourceTemplate job{ };
        job.id        = field[i++].GetInt32();
        job.stat_id   = field[i++].GetInt32();
        job.job_class = field[i++].GetInt32();
        job.job_depth = field[i++].GetInt32();
        job.up_lv     = field[i++].GetInt32();
        job.up_jlv    = field[i++].GetInt32();
        for (int j                = 0; j < 4; j++) {
            job.available_job[j] = field[i++].GetInt32();
        }
        _jobTemplateStore[job.id] = job;
        ++count;
    } while (result->NextRow());

    MX_LOG_INFO("server.worldserver", ">> Loaded %u job templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}

bool ObjectMgr::LoadJobLevelBonus()
{
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT job_id, "
                                                       "strength_0, vital_0, dexterity_0, agility_0, intelligence_0, mentality_0, luck_0, "
                                                       "strength_1, vital_1, dexterity_1, agility_1, intelligence_1, mentality_1, luck_1, "
                                                       "strength_2, vital_2, dexterity_2, agility_2, intelligence_2, mentality_2, luck_2, "
                                                       "strength_3, vital_3, dexterity_3, agility_3, intelligence_3, mentality_3, luck_3 "
                                                       "FROM JobLevelBonus;");
    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 job level bonus templates. DB packetHandler `JobLevelBonus` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
        Field                 *field = result->Fetch();
        JobLevelBonusTemplate bonus{ };

        int i = 0, j = 0;
        bonus.job_id = field[i++].GetInt32();
        for (j                       = 0; j < 4; j++) {
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

    MX_LOG_INFO("server.worldserver", ">> Loaded %u job level bonus templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}

bool ObjectMgr::LoadNPCResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, x, y, z, face, local_flag, contact_script FROM NPCResource;");

    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 NPC templates. DB packetHandler `NPCResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
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

        if (npc.local_flag == 0) {
            auto *_npc = new NPC{ };
            _npc->Relocate((float) npc.x, (float) npc.y, (float) npc.z);
            _npc->SetUInt32Value(UNIT_FIELD_UID, npc.id);
            _npc->SetLayer(0);
            _npc->m_pBase = npc;
            sMemoryPool->AllocMiscHandle(*_npc);
            auto region = sArRegion->GetRegion(*_npc);
            region->AddObject(_npc);
            ++count;
        }
    } while (result->NextRow());
    MX_LOG_INFO("server.worldserver", ">> Loaded %u NPC templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}


bool ObjectMgr::LoadWorldLocation()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, location_type, time_id, weather_id, weather_ratio, weather_change_time FROM WorldLocation;");

    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 WorldLocation templates. DB packetHandler `NPCResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
        Field *field = result->Fetch();
        auto idx = field[0].GetInt32();
        auto location_type = field[1].GetUInt8();
        auto time_idx = field[2].GetInt32();
        auto weather_id = field[3].GetInt32();
        auto weather_ratio = field[4].GetUInt8();
        auto weather_change_time = (field[5].GetUInt8()) * 6000;
        sWorldLocationMgr->RegisterWorldLocation(idx, location_type, time_idx, weather_id, weather_ratio, weather_change_time,0);
        ++count;
    } while (result->NextRow());
    MX_LOG_INFO("server.worldserver", ">> Loaded %u WorldLocation templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}


bool ObjectMgr::LoadSummonResource()
{
    uint32      oldMSTime = getMSTime();
    QueryResult result    = GameDatabase.Query("SELECT id, type, magic_type, rate, stat_id, standard_walk_speed, standard_run_speed,"
                                                       "walk_speed, run_speed, is_riding_only, attack_range, material, weapon_type,"
                                                       "form, evolve_target, card_id FROM SummonResource;");

    if (!result) {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 Summon templates. DB packetHandler `SummonResource` is empty!");
        return false;
    }

    uint32 count = 0;
    do {
        Field                  *field = result->Fetch();
        SummonResourceTemplate summon{ };
        int                    i      = 0;
        summon.id                  = field[i++].GetInt32();
        summon.type                = field[i++].GetInt32();
        summon.magic_type          = field[i++].GetInt32();
        summon.rate                = field[i++].GetInt32();
        summon.stat_id             = field[i++].GetInt32();
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
        summon.card_id             = field[i++].GetInt32();
        _summonResourceStore[summon.id] = summon;
        ++count;
    } while (result->NextRow());
    MX_LOG_INFO("server.worldserver", ">> Loaded %u Summon templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    return true;
}

CreatureStat ObjectMgr::GetStatInfo(int stat_id)
{
    if (_creatureBaseStore.count(stat_id) == 1)
        return _creatureBaseStore[stat_id];
    return CreatureStat{ };
}

ItemTemplate ObjectMgr::GetItemBase(int item_id)
{
    if (_itemTemplateStore.count(item_id) == 1) {
        return _itemTemplateStore[item_id];
    }
    return ItemTemplate{ };
}

CreatureStat ObjectMgr::GetJobLevelBonus(int depth, int jobs[], int levels[])
{
    CreatureStat stat{ };
    if (depth >= 0) {
        for (int i = 0; i < 4; i++) {
            if (_jobBonusStore.count(jobs[i])) {
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
                stat.strength += (int) ((v1 * jlb.strength[1]) + (v2 * jlb.strength[2]) + (jlb.strength[3]) + (v0 * jlb.strength[0]));
                stat.vital += (int) ((v1 * jlb.vital[1]) + (v2 * jlb.vital[2]) + (jlb.vital[3]) + (v0 * jlb.vital[0]));
                stat.dexterity += (int) ((v1 * jlb.dexterity[1]) + (v2 * jlb.dexterity[2]) + (jlb.dexterity[3]) + (v0 * jlb.dexterity[0]));
                stat.agility += (int) ((v1 * jlb.agility[1]) + (v2 * jlb.agility[2]) + (jlb.agility[3]) + (v0 * jlb.agility[0]));
                stat.intelligence += (int) ((v1 * jlb.intelligence[1]) + (v2 * jlb.intelligence[2]) + (jlb.intelligence[3]) + (v0 * jlb.intelligence[0]));
                stat.mentality += (int) ((v1 * jlb.mentality[1]) + (v2 * jlb.mentality[2]) + (jlb.mentality[3]) + (v0 * jlb.mentality[0]));
                stat.luck += (int) ((v1 * jlb.luck[1]) + (v2 * jlb.luck[2]) + (jlb.luck[3]) + (v0 * jlb.luck[0]));
            }
        }
    }

    return stat;
}

JobResourceTemplate ObjectMgr::GetJobInfo(int job_id)
{
    if (_jobTemplateStore.count(job_id) == 1) {
        return _jobTemplateStore[job_id];
    }
    return JobResourceTemplate{ };
}

SummonResourceTemplate ObjectMgr::GetSummonBase(int idx)
{
    if (_summonResourceStore.count(idx) == 1)
        return _summonResourceStore[idx];
    return SummonResourceTemplate{ };
}

std::vector<MarketInfo> ObjectMgr::GetMarketInfo(std::string szKey)
{
    if(_marketResourceStore.count(szKey) == 1) {
        return _marketResourceStore[szKey];
    }
    return std::vector<MarketInfo>();
}

bool ObjectMgr::LoadMapContent()
{
    TerrainSeamlessWorldInfo seamlessWorldInfo{ };
    if (!seamlessWorldInfo.Initialize("terrainseamlessworld.cfg", false)) {
        MX_LOG_FATAL("server.worldserver", "TerrainSeamlessWorld.cfg read error !");
        return false;
    }

    int y = 0;
    fTileSize  = seamlessWorldInfo.m_fTileLength;
    fMapLength = seamlessWorldInfo.m_nSegmentCountPerMap * seamlessWorldInfo.m_fTileLength * seamlessWorldInfo.m_nTileCountPerSegment;
    for (float fAttrLen = seamlessWorldInfo.m_fTileLength * 0.125f; y < seamlessWorldInfo.m_sizMapCount.height; ++y) {
        for (int i = 0; i < seamlessWorldInfo.m_sizMapCount.width; ++i) {
            std::string strLocationFileName = seamlessWorldInfo.GetLocationFileName(i, y);

            if (strLocationFileName.length() != 0) {
                int wid = seamlessWorldInfo.GetWorldID(i, y);
                if (wid != -1) {
                    SetDefaultLocation(i, y, fMapLength, wid);
                }
                LoadLocationFile(("Resource/NewMap/"s+strLocationFileName), i, y, fAttrLen, fMapLength);
            }
        }
    }
    return true;
}

void ObjectMgr::SetDefaultLocation(int x, int y, float fMapLength, int LocationId)
{
    X2D::Pointf begin{};
    X2D::Pointf end{};

    begin.x = (x * fMapLength);
    begin.y = (y * fMapLength);
    end.x = ((x + 1) * fMapLength);
    end.y = ((y + 1) * fMapLength);
    MapLocationInfo li(begin,end, LocationId, 2147483646);
    RegisterMapLocationInfo(li);
}

void ObjectMgr::RegisterMapLocationInfo(MapLocationInfo location_info)
{
    if(g_qtLocationInfo == nullptr)
    {
        g_qtLocationInfo = new X2D::QuadTreeMapInfo(g_nMapWidth, g_nMapHeight);
    }
    g_qtLocationInfo->Add(std::move(location_info));
}

#include "Scripting/XLua.h"
void ObjectMgr::LoadLocationFile(std::string szFilename, int x, int y, float fAttrLen, float fMapLength)
{
    int nCharSize;
    LocationInfoHeader lih{};
    int nPolygonCount;
    int nPointCount;

    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0,std::ios::end);
    int size = infile.tellg();
    infile.seekg(0,std::ios::beg);
    if(size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize(size);
    infile.read(reinterpret_cast<char*>(&buffer[0]), size);
    infile.close();

    auto total_entries = buffer.read<int>();
    for(int i = 0; i < total_entries; ++i) {
        lih.nPriority = buffer.read<int>();
        lih.x = buffer.read<float>();
        lih.y = buffer.read<float>();
        lih.z = buffer.read<float>();
        lih.fRadius = buffer.read<float>();

        nCharSize = buffer.read<int>();
        if(nCharSize > 1) {
            buffer.read_skip(nCharSize);
        }
        nCharSize = buffer.read<int>();
        g_currentLocationId = 0;
        if(nCharSize <= 1)
            continue;

        auto script = buffer.ReadString(nCharSize);
        sScriptingMgr->RunString(script);
        if(g_currentLocationId == 0)
            continue;

        nPolygonCount = buffer.read<int>();
        for(int cp = 0; cp < nPolygonCount; ++cp) {
            nPointCount = buffer.read<int>();
            float sx = x * fMapLength;
            float sy = y * fMapLength;
            std::vector<X2D::Pointf> points{};

            for(int p = 0; p < nPointCount; ++p) {
                X2D::Pointf pt{ };
                pt.x = sx + ((float) buffer.read<int>() * fAttrLen);
                pt.x = sy + ((float) buffer.read<int>() * fAttrLen);
                points.emplace_back(pt);
                auto location_info = MapLocationInfo(points, g_currentLocationId, 0);
                RegisterMapLocationInfo(location_info);
            }
        }
    }
}

int ObjectMgr::GetLocationID(float x, float y)
{
    int loc_id = 0;
    int priority = 0x7fffffff;
    X2D::Pointf pt{};
    pt.x = x;
    pt.y = y;
    X2D::QuadTreeMapInfo::FunctorAdaptor fn{};
    g_qtLocationInfo->Enum(pt, fn);

    for(auto& info : fn.pResult)
    {
        if(info.priority < priority)
        {
            loc_id = info.location_id;
            priority = info.priority;
        }
    }
    return loc_id;
}