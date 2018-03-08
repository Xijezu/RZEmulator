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

#ifndef NGEMITY_OBJECTMGR_H_
#define NGEMITY_OBJECTMGR_H_

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
#include "ObjectRegistry.h"

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
        ObjectMgr();
        ~ObjectMgr() = default;

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
        const CreatureStat *const GetStatInfo(int stat_id);
        const ItemTemplate *const GetItemBase(int item_id);
        const SkillBase *const GetSkillBase(int);
        CreatureStat GetJobLevelBonus(int depth, int jobs[], const int levels[]);
        CreatureStat GetSummonLevelBonus(int summon_code, int growth_depth, int level);
        const JobResourceTemplate *const GetJobInfo(int job_id);
        const SummonResourceTemplate *const GetSummonBase(int idx);
        const MonsterBase *const GetMonsterInfo(int idx);
        const FieldPropTemplate *const GetFieldPropBase(int idx);
        const std::vector<MarketInfo> *const GetMarketInfo(const std::string &);
        const QuestBaseServer *const GetQuestBase(int code);
        const QuestLink *const GetQuestLink(int code, int start_id);
        const StateTemplate *const GetStateInfo(int code);
        bool checkQuestTypeFlag(QuestType type, int flag);
        std::string GetSummonName();
        int GetNeedJpForJobLevelUp(int, int);
        int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
        int64 GetNeedExp(int level);
        int64 GetNeedSummonExp(int level);
        WayPointInfo *GetWayPoint(int waypoint_id);
        const DropGroup *GetDropGroupInfo(int drop_group_id);

        void RegisterMonsterRespawnInfo(MonsterRespawnInfo info);
        void AddWayPoint(int waypoint_id, float x, float y);
        void SetWayPointType(int waypoint_id, int type);

        int g_currentLocationId{0};

        std::unordered_map<int, WayPointInfo> g_vWayPoint{ };
        std::vector<MonsterRespawnInfo>       g_vRespawnInfo{ };
        X2D::QuadTreeMapInfo                  g_qtBlockInfo;
    private:
        ObjectRegistry<int, JobResourceTemplate>                 _jobTemplateStore;
        ObjectRegistry<int, ItemTemplate>                        _itemTemplateStore;
        ObjectRegistry<int, CreatureStat>                        _creatureBaseStore;
        ObjectRegistry<int, JobLevelBonusTemplate>               _jobBonusStore;
        ObjectRegistry<int, SummonResourceTemplate>              _summonResourceStore;
        ObjectRegistry<int, LevelResourceTemplate>               _levelResourceStore;
        ObjectRegistry<int, SkillBase>                           _skillBaseStore;
        ObjectRegistry<int, MonsterBase>                         _monsterBaseStore;
        ObjectRegistry<int, DropGroup>                           _dropTemplateStore;
        ObjectRegistry<int, QuestBaseServer>                     _questTemplateStore;
        ObjectRegistry<int, NPCTemplate>                         _npcTemplateStore;
        ObjectRegistry<int, FieldPropTemplate>                   _fieldPropTemplateStore;
        ObjectRegistry<int, SummonLevelBonus>                    _summonBonusStore;
        ObjectRegistry<int, StateTemplate>                       _stateTemplateStore;
        std::vector<int>                                         _summonPrefixStore;
        std::vector<int>                                         _summonPostfixStore;
        std::unordered_map<std::string, std::vector<MarketInfo>> _marketResourceStore;
        std::unordered_map<int32, uint64>                        _summonLevelStore;
        std::unordered_map<int, std::string>                     _stringResourceStore;
        std::vector<QuestLink *>                                 _questLinkStore;
        std::vector<SkillTreeGroup>                              _skillTreeResourceStore;

        void RegisterSkillTree(SkillTreeBase base);
        std::vector<SkillTreeBase> getSkillTree(int job_id);
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // NGEMITY_OBJECTMGR_H_