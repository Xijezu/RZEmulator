#ifndef _OBJECTMGR_H_
#define _OBJECTMGR_H_

#include "Common.h"
#include "DatabaseTemplates.h"
#include <unordered_map>
#include "QuadTreeMapInfo.h"
#include "Dynamic/UnorderedMap.h"
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
        ObjectMgr();
        ~ObjectMgr() = default;

        typedef UNORDERED_MAP<int32, ItemTemplate>                  ItemTemplateContainer;
        typedef UNORDERED_MAP<int32, uint64>                        SummonLevelBaseContainer;
        typedef UNORDERED_MAP<int32, CreatureStat>                  CreatureBaseStat;
        typedef UNORDERED_MAP<int32, JobLevelBonusTemplate>         JobLevelBonusTemplateContainer;
        typedef UNORDERED_MAP<int32, SummonLevelBonus>              SummonBonusTemplateContainer;
        typedef UNORDERED_MAP<int32, JobResourceTemplate>           JobResourceTemplateContainer;
        typedef UNORDERED_MAP<int32, SummonResourceTemplate>        SummonResourceTemplateContainer;
        typedef std::vector<SkillTreeGroup>                         SkillTreeTemplateContainer;
        typedef UNORDERED_MAP<int32, SkillBase>                     SkillBaseContainer;
        typedef UNORDERED_MAP<int32, MonsterBase>                   MonsterBaseContainer;
        typedef UNORDERED_MAP<int32, LevelResourceTemplate>         LevelTemplateContainer;
        typedef UNORDERED_MAP<std::string, std::vector<MarketInfo>> MarketResourceTemplateContainer;
        typedef UNORDERED_MAP<int, DropGroup>                       DropGroupTemplateContainer;
        typedef UNORDERED_MAP<int, std::string>                     StringContainer;
        typedef UNORDERED_MAP<int, QuestBaseServer>                 QuestResourceTemplateContainer;
        typedef UNORDERED_MAP<int, FieldPropTemplate>               FieldPropTemplateContainer;
        typedef std::vector<QuestLink>                              QuestLinkTemplateContainer;
        typedef UNORDERED_MAP<int, NPCTemplate>                     NPCTemplateContainer;
        typedef UNORDERED_MAP<int, StateTemplate>                   StateTemplateContainer;

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

        void AddWayPoint(int waypoint_id, float x, float y);
        void SetWayPointType(int waypoint_id, int type);
        WayPointInfo *GetWayPoint(int waypoint_id);
        void RegisterMonsterRespawnInfo(MonsterRespawnInfo info);

        NPC *GetNewNPC(NPCTemplate *npc_info, uint8 layer);
        void AddNPCToWorld();

        const std::string &GetValueFromNameID(int name_id);

        CreatureStat *const GetStatInfo(int stat_id);
        ItemTemplate *const GetItemBase(int item_id);
        int64 GetItemSellPrice(int64 price, int rank, int lv, bool same_price_for_buying);
        SkillBase *const GetSkillBase(int);
        CreatureStat GetJobLevelBonus(int depth, int jobs[], const int levels[]);
        CreatureStat GetSummonLevelBonus(int summon_code, int growth_depth, int level);
        JobResourceTemplate *const GetJobInfo(int job_id);
        SummonResourceTemplate *const GetSummonBase(int idx);
        MonsterBase *const GetMonsterInfo(int idx);
        FieldPropTemplate *const GetFieldPropBase(int idx);
        std::vector<MarketInfo> *const GetMarketInfo(const std::string &);
        QuestBaseServer *const GetQuestBase(int code);
        QuestLink *const GetQuestLink(int code, int start_id);
        StateTemplate *const GetStateInfo(int code);
        bool checkQuestTypeFlag(QuestType type, int flag);
        std::string GetSummonName();

        int GetNeedJpForJobLevelUp(int, int);
        int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
        int64 GetNeedExp(int level);
        uint64 GetNeedSummonExp(int level);
        Monster *RespawnMonster(float x, float y, uint8_t layer, int id, bool is_wandering, int way_point_id, MonsterDeleteHandler *pDeleteHandler, bool bNeedLock);
        bool IsInRandomPoolMonster(int group_id, int monster_id);
        bool LearnAllSkill(Player *pPlayer);

        DropGroup *GetDropGroupInfo(int drop_group_id);
        bool SelectItemIDFromDropGroup(int nDropGroupID, int &nItemID, int64 &nItemCount);

        ushort IsLearnableSkill(Unit *, int, int, int &);
        int GetLocationID(float x, float y) const;
        bool IsBlocked(float x, float y);
        bool CollisionToLine(float x1, float y1, float x2, float y2);

        int                             g_currentLocationId{0};

        UNORDERED_MAP<int, WayPointInfo> g_vWayPoint{ };
        //UNORDERED_MAP<int,MonsterRespawnInfo> g_vRespawnInfo{};
        std::vector<MonsterRespawnInfo> g_vRespawnInfo{ };
        X2D::QuadTreeMapInfo            g_qtBlockInfo;
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

        ushort isLearnableSkill(Unit *pUnit, int skill_id, int skill_level, int nJobID, int unit_job_level);
        void RegisterSkillTree(SkillTreeBase base);

        std::vector<SkillTreeBase> getSkillTree(int job_id);
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // _OBJECTMGR_H_