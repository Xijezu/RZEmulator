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
#include "DatabaseTemplates.h"
#include <unordered_map>
#include "QuadTreeMapInfo.h"
#include "SkillBase.h"
#include "NPCBase.h"
#include "StateBase.h"
#include "QuestBase.h"
#include "FieldPropBase.h"
#include "SummonBase.h"
#include "CreatureAttribute.h"
#include "MonsterBase.h"
#include "Object.h"

class Player;
class Item;
class Monster;
class Unit;
class NPC;

struct WayPointInfo
{
    int                   way_point_speed;
    int                   way_point_type;
    int                   way_point_id;
    std::vector<Position> vWayPoint;
};

class ObjectMgr
{
    public:
        friend class GameContent;
        ~ObjectMgr() = default;

        static ObjectMgr &Instance()
        {
            static ObjectMgr instance;
            return instance;
        }

        typedef std::unordered_map<int32, ItemTemplate>                  ItemTemplateContainer;
        typedef std::unordered_map<int32, uint64>                        SummonLevelBaseContainer;
        typedef std::unordered_map<int32, CreatureStat>                  CreatureBaseStat;
        typedef std::unordered_map<int32, JobLevelBonusTemplate>         JobLevelBonusTemplateContainer;
        typedef std::unordered_map<int32, SummonLevelBonus>              SummonBonusTemplateContainer;
        typedef std::unordered_map<int32, JobResourceTemplate>           JobResourceTemplateContainer;
        typedef std::unordered_map<int32, SummonResourceTemplate>        SummonResourceTemplateContainer;
        typedef std::vector<SkillTreeGroup>                              SkillTreeTemplateContainer;
        typedef std::unordered_map<int32, SkillBase>                     SkillBaseContainer;
        typedef std::unordered_map<int32, MonsterBase>                   MonsterBaseContainer;
        typedef std::unordered_map<int32, LevelResourceTemplate>         LevelTemplateContainer;
        typedef std::unordered_map<std::string, std::vector<MarketInfo>> MarketResourceTemplateContainer;
        typedef std::unordered_map<int, DropGroup>                       DropGroupTemplateContainer;
        typedef std::unordered_map<int, std::string>                     StringContainer;
        typedef std::unordered_map<int, QuestBaseServer>                 QuestResourceTemplateContainer;
        typedef std::unordered_map<int, FieldPropTemplate>               FieldPropTemplateContainer;
        typedef std::vector<QuestLink>                                   QuestLinkTemplateContainer;
        typedef std::unordered_map<int, NPCTemplate>                     NPCTemplateContainer;
        typedef std::unordered_map<int, StateTemplate>                   StateTemplateContainer;

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
        void InitGameContent();

        void UnloadAll();

        const std::string &GetValueFromNameID(int name_id);
        CreatureStat *GetStatInfo(int stat_id);
        ItemTemplate *GetItemBase(int item_id);
        SkillBase *GetSkillBase(int);
        CreatureStat GetJobLevelBonus(int depth, int jobs[], const int levels[]);
        CreatureStat GetSummonLevelBonus(int summon_code, int growth_depth, int level);
        JobResourceTemplate *GetJobInfo(int job_id);
        SummonResourceTemplate *GetSummonBase(int idx);
        MonsterBase *GetMonsterInfo(int idx);
        FieldPropTemplate *GetFieldPropBase(int idx);
        std::vector<MarketInfo> *GetMarketInfo(const std::string &);
        QuestBaseServer *GetQuestBase(int code);
        QuestLink *GetQuestLink(int code, int start_id);
        StateTemplate *GetStateInfo(int code);
        bool checkQuestTypeFlag(QuestType type, int flag);
        std::string GetSummonName();
        int GetNeedJpForJobLevelUp(int, int);
        int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
        int64 GetNeedExp(int level);
        int64 GetNeedSummonExp(int level);
        WayPointInfo *GetWayPoint(int waypoint_id);
        DropGroup *GetDropGroupInfo(int drop_group_id);

        void RegisterMonsterRespawnInfo(MonsterRespawnInfo info);
        void AddWayPoint(int waypoint_id, float x, float y);
        void SetWayPointType(int waypoint_id, int type);

        int g_currentLocationId{0};

        std::unordered_map<int, WayPointInfo> g_vWayPoint{ };
        std::vector<MonsterRespawnInfo>       g_vRespawnInfo{ };
        X2D::QuadTreeMapInfo                  g_qtBlockInfo;
    private:
        JobResourceTemplateContainer    _jobTemplateStore;
        ItemTemplateContainer           _itemTemplateStore;
        CreatureBaseStat                _creatureBaseStore;
        JobLevelBonusTemplateContainer  _jobBonusStore;
        SummonResourceTemplateContainer _summonResourceStore;
        MarketResourceTemplateContainer _marketResourceStore;
        SkillTreeTemplateContainer      _skillTreeResourceStore;
        SummonLevelBaseContainer        _summonLevelStore;
        StringContainer                 _stringResourceStore;
        LevelTemplateContainer          _levelResourceStore;
        SkillBaseContainer              _skillBaseStore;
        MonsterBaseContainer            _monsterBaseStore;
        DropGroupTemplateContainer      _dropTemplateStore;
        QuestResourceTemplateContainer  _questTemplateStore;
        QuestLinkTemplateContainer      _questLinkStore;
        NPCTemplateContainer            _npcTemplateStore;
        FieldPropTemplateContainer      _fieldPropTemplateStore;
        std::vector<int>                _summonPrefixStore;
        std::vector<int>                _summonPostfixStore;
        SummonBonusTemplateContainer    _summonBonusStore;
        StateTemplateContainer          _stateTemplateStore;

        void RegisterSkillTree(SkillTreeBase base);
        std::vector<SkillTreeBase> getSkillTree(int job_id);
    protected:
        ObjectMgr();
};

#define sObjectMgr ObjectMgr::Instance()