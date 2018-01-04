#ifndef _OBJECTMGR_H_
#define _OBJECTMGR_H_

#include "Common.h"
#include "DatabaseTemplates.h"
#include <unordered_map>
#include <Item/Item.h>
#include <Player/Player.h>
#include "QuadTreeMapInfo.h"
#include "Dynamic/UnorderedMap.h"
#include "SkillBase.h"
#include "Monster.h"
#include "NPCBase.h"
#include "QuestBase.h"

class NPC;

struct Waypoint {
    int                   way_point_speed;
    int                   way_point_type;
    int                   way_point_id;
    std::vector<Position> vWayPoint;
};

class ObjectMgr {
public:
    ObjectMgr() = default;
    ~ObjectMgr() = default;

    typedef UNORDERED_MAP<uint32, ItemTemplate>                 ItemTemplateContainer;
    typedef UNORDERED_MAP<uint32, uint64>                       SummonLevelBaseContainer;
    typedef UNORDERED_MAP<uint32, CreatureStat>                 CreatureBaseStat;
    typedef UNORDERED_MAP<uint32, JobLevelBonusTemplate>        JobLevelBonusTemplateContainer;
    typedef UNORDERED_MAP<uint32, JobResourceTemplate>          JobResourceTemplateContainer;
    typedef UNORDERED_MAP<uint32, SummonResourceTemplate>       SummonResourceTemplateContainer;
    typedef std::vector<SkillTreeGroup>                         SkillTreeTemplateContainer;
    typedef UNORDERED_MAP<uint32, SkillBase>                    SkillBaseContainer;
    typedef UNORDERED_MAP<uint32, MonsterBase>                  MonsterBaseContainer;
    typedef UNORDERED_MAP<uint32, LevelResourceTemplate>        LevelTemplateContainer;
    typedef UNORDERED_MAP<std::string, std::vector<MarketInfo>> MarketResourceTemplateContainer;
    typedef UNORDERED_MAP<int, DropGroup>                       DropGroupTemplateContainer;
    typedef UNORDERED_MAP<int, QuestBaseServer>                 QuestResourceTemplateContainer;
    typedef std::vector<QuestLink>                              QuestLinkTemplateContainer;
    typedef UNORDERED_MAP<int, NPCTemplate>                     NPCTemplateContainer;

    bool LoadStatResource();
    bool LoadJobResource();
    bool LoadJobLevelBonus();
    bool LoadNPCResource();
    bool LoadMonsterResource();
    bool LoadItemResource();
    bool LoadSummonLevelResource();
    bool LoadSummonResource();
    bool LoadQuestResource();
    bool LoadQuestLinkResource();
    bool LoadDropGroupResource();
    bool LoadMarketResource();
    bool LoadWorldLocation();
    bool LoadSkillResource();
    bool LoadLevelResource();
    bool LoadSkillTreeResource();
    bool LoadSkillJP();
    bool InitGameContent();

    void UnloadAll();

    void AddWayPoint(int waypoint_id, float x, float y);
    void SetWayPointType(int waypoint_id, int type);
    void RegisterMonsterRespawnInfo(MonsterRespawnInfo info);

    NPC* GetNewNPC(NPCTemplate* npc_info, uint8 layer);
    void AddNPCToWorld();

    CreatureStat* const GetStatInfo(uint stat_id);
    ItemTemplate* const GetItemBase(uint item_id);
    uint64 GetItemSellPrice(uint64 price, int rank, int lv, bool same_price_for_buying);
    SkillBase* const GetSkillBase(uint);
    CreatureStat GetJobLevelBonus(int depth, int jobs[], const int levels[]);
    JobResourceTemplate* const GetJobInfo(uint job_id);
    SummonResourceTemplate* const GetSummonBase(uint idx);
    MonsterBase* const GetMonsterInfo(uint idx);
    std::vector<MarketInfo>* const GetMarketInfo(const std::string&);
    QuestBaseServer* const GetQuestBase(int code);
    QuestLink* const GetQuestLink(int code, int start_id);
    bool checkQuestTypeFlag(QuestType type, int flag);

    int GetNeedJpForJobLevelUp(uint, uint);
    int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
    long GetNeedExp(int level);
    uint64 GetNeedSummonExp(int level);
    Monster* RespawnMonster(uint x, uint y, uint8_t layer, uint id, bool is_wandering, int way_point_id, MonsterDeleteHandler* pDeleteHandler, bool bNeedLock);
    bool IsInRandomPoolMonster(int group_id, int monster_id);

    DropGroup* GetDropGroupInfo(int drop_group_id);
    bool SelectItemIDFromDropGroup(int nDropGroupID, int &nItemID, long &nItemCount);

    ushort IsLearnableSkill(Unit *, uint, int, int &);
    int GetLocationID(float x, float y) const;

    int g_currentLocationId{0};

    UNORDERED_MAP<int,Waypoint> g_vWayPoint{};
    //UNORDERED_MAP<int,MonsterRespawnInfo> g_vRespawnInfo{};
    std::vector<MonsterRespawnInfo> g_vRespawnInfo{};
private:
    JobResourceTemplateContainer    _jobTemplateStore;
    ItemTemplateContainer           _itemTemplateStore;
    CreatureBaseStat                _creatureBaseStore;
    JobLevelBonusTemplateContainer  _jobBonusStore;
    SummonResourceTemplateContainer _summonResourceStore;
    MarketResourceTemplateContainer _marketResourceStore;
    SkillTreeTemplateContainer      _skillTreeResourceStore;
    SummonLevelBaseContainer        _summonLevelStore;
    LevelTemplateContainer          _levelResourceStore;
    SkillBaseContainer              _skillBaseStore;
    MonsterBaseContainer            _monsterBaseStore;
    DropGroupTemplateContainer      _dropTemplateStore;
    QuestResourceTemplateContainer  _questTemplateStore;
    QuestLinkTemplateContainer      _questLinkStore;
    NPCTemplateContainer            _npcTemplateStore;

    ushort isLearnableSkill(Unit *pUnit, uint skill_id, int skill_level, int nJobID, int unit_job_level);
    void RegisterSkillTree(SkillTreeBase base);

    std::vector<SkillTreeBase> getSkillTree(int job_id);
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // _OBJECTMGR_H_