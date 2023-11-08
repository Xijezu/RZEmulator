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
#include <unordered_map>

#include "Common.h"
#include "CreatureAttribute.h"
#include "DatabaseTemplates.h"
#include "FieldPropBase.h"
#include "MonsterBase.h"
#include "GameContent.h"
#include "NPCBase.h"
#include "Object.h"
#include "QuadTreeMapInfo.h"
#include "QuestBase.h"
#include "SkillBase.h"
#include "StateBase.h"
#include "SummonBase.h"

class Player;
class Item;
class Monster;
class Unit;
class NPC;

struct WayPointInfo {
    int32_t way_point_speed;
    int32_t way_point_type;
    int32_t way_point_id;
    std::vector<Position> vWayPoint;
};

class ObjectMgr {
public:
    friend class GameContent;
    ~ObjectMgr() = default;

    // Deleting the copy & assignment operators
    // Better safe than sorry
    ObjectMgr(const ObjectMgr &) = delete;
    ObjectMgr &operator=(const ObjectMgr &) = delete;

    static ObjectMgr &Instance()
    {
        static ObjectMgr instance;
        return instance;
    }

    typedef std::unordered_map<int32_t, std::shared_ptr<ItemTemplate>> ItemTemplateContainer;
    typedef std::unordered_map<int32_t, uint64_t> SummonLevelBaseContainer;
    typedef std::unordered_map<int32_t, CreatureStat> CreatureBaseStat;
    typedef std::unordered_map<int32_t, JobLevelBonusTemplate> JobLevelBonusTemplateContainer;
    typedef std::unordered_map<int32_t, SummonLevelBonus> SummonBonusTemplateContainer;
    typedef std::unordered_map<int32_t, JobResourceTemplate> JobResourceTemplateContainer;
    typedef std::unordered_map<int32_t, SummonResourceTemplate> SummonResourceTemplateContainer;
    typedef std::vector<SkillTreeGroup> SkillTreeTemplateContainer;
    typedef std::unordered_map<int32_t, SkillBase> SkillBaseContainer;
    typedef std::unordered_map<int32_t, MonsterBase> MonsterBaseContainer;
    typedef std::unordered_map<int32_t, LevelResourceTemplate> LevelTemplateContainer;
    typedef std::unordered_map<std::string, std::vector<MarketInfo>> MarketResourceTemplateContainer;
    typedef std::unordered_map<int, DropGroup> DropGroupTemplateContainer;
    typedef std::unordered_map<int, std::string> StringContainer;
    typedef std::unordered_map<int, QuestBaseServer> QuestResourceTemplateContainer;
    typedef std::unordered_map<int, FieldPropTemplate> FieldPropTemplateContainer;
    typedef std::vector<QuestLink> QuestLinkTemplateContainer;
    typedef std::unordered_map<int, NPCTemplate> NPCTemplateContainer;
    typedef std::unordered_map<int, StateTemplate> StateTemplateContainer;

    void LoadStatResource();
    void LoadJobResource();
    void LoadJobLevelBonus();
    void LoadNPCResource();
    void LoadMonsterResource();
    void LoadItemResource();
    void LoadSummonLevelResource();
    void LoadSummonResource();
    void LoadQuestResource();
    void LoadQuestLinkResource();
    void LoadDropGroupResource();
    void LoadFieldPropResource();
    void LoadStateResource();
    void LoadMarketResource();
    void LoadSummonLevelBonus();
    void LoadWorldLocation();
    void LoadSkillResource();
    void LoadLevelResource();
    void LoadSkillTreeResource();
    void LoadDungeonResource();
    void LoadEnhanceResource();
    void LoadMixResource();
    void LoadStringResource();
    void LoadSkillJP();
    void LoadSummonNameResource();
    void LoadEffectResource();
    void InitGameContent();

    void UnloadAll();

    const std::string &GetValueFromNameID(int32_t name_id);
    CreatureStat *GetStatInfo(int32_t stat_id);
    std::shared_ptr<ItemTemplate> GetItemBase(int32_t item_id);
    SkillBase *GetSkillBase(int32_t);
    CreatureStat GetJobLevelBonus(int32_t depth, int32_t jobs[], const int32_t levels[]);
    CreatureStat GetSummonLevelBonus(int32_t summon_code, int32_t growth_depth, int32_t level);
    JobResourceTemplate *GetJobInfo(int32_t job_id);
    SummonResourceTemplate *GetSummonBase(int32_t idx);
    MonsterBase *GetMonsterInfo(int32_t idx);
    FieldPropTemplate *GetFieldPropBase(int32_t idx);
    std::vector<MarketInfo> *GetMarketInfo(const std::string &);
    QuestBaseServer *GetQuestBase(int32_t code);
    QuestLink *GetQuestLink(int32_t code, int32_t start_id);
    StateTemplate *GetStateInfo(int32_t code);
    bool checkQuestTypeFlag(QuestType type, int32_t flag);
    std::string GetSummonName();
    int32_t GetNeedJpForJobLevelUp(int, int32_t);
    int32_t GetNeedJpForSkillLevelUp(int32_t skill_id, int32_t skill_level, int32_t nJobID);
    int64_t GetNeedExp(int32_t level);
    int64_t GetNeedSummonExp(int32_t level);
    WayPointInfo *GetWayPoint(int32_t waypoint_id);
    DropGroup *GetDropGroupInfo(int32_t drop_group_id);

    void RegisterMonsterRespawnInfo(GameContent::MonsterRespawnInfo info);
    void AddWayPoint(int32_t waypoint_id, float_t x, float_t y);
    void SetWayPointType(int32_t waypoint_id, int32_t type);

    int32_t g_currentLocationId{0};

    std::unordered_map<int, WayPointInfo> g_vWayPoint{};
    std::vector<GameContent::MonsterRespawnInfo> g_vRespawnInfo{};
    X2D::QuadTreeMapInfo g_qtBlockInfo;

private:
    JobResourceTemplateContainer _jobTemplateStore;
    ItemTemplateContainer _itemTemplateStore;
    CreatureBaseStat _creatureBaseStore;
    JobLevelBonusTemplateContainer _jobBonusStore;
    SummonResourceTemplateContainer _summonResourceStore;
    MarketResourceTemplateContainer _marketResourceStore;
    SkillTreeTemplateContainer _skillTreeResourceStore;
    SummonLevelBaseContainer _summonLevelStore;
    StringContainer _stringResourceStore;
    LevelTemplateContainer _levelResourceStore;
    SkillBaseContainer _skillBaseStore;
    MonsterBaseContainer _monsterBaseStore;
    DropGroupTemplateContainer _dropTemplateStore;
    QuestResourceTemplateContainer _questTemplateStore;
    QuestLinkTemplateContainer _questLinkStore;
    NPCTemplateContainer _npcTemplateStore;
    FieldPropTemplateContainer _fieldPropTemplateStore;
    std::vector<int32_t> _summonPrefixStore;
    std::vector<int32_t> _summonPostfixStore;
    SummonBonusTemplateContainer _summonBonusStore;
    StateTemplateContainer _stateTemplateStore;

    void RegisterSkillTree(SkillTreeBase base);
    std::vector<SkillTreeBase> getSkillTree(int32_t job_id);

protected:
    ObjectMgr();
};

#define sObjectMgr ObjectMgr::Instance()