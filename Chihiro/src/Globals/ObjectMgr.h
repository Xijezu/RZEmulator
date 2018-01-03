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

    bool LoadStatResource();
    bool LoadJobResource();
    bool LoadJobLevelBonus();
    bool LoadNPCResource();
    bool LoadMonsterResource();
    bool LoadItemResource();
    bool LoadSummonLevelResource();
    bool LoadSummonResource();
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

    CreatureStat* const GetStatInfo(uint stat_id);
    ItemTemplate* const GetItemBase(uint item_id);
    uint64 GetItemSellPrice(uint64 price, int rank, int lv, bool same_price_for_buying);
    SkillBase* const GetSkillBase(uint);
    CreatureStat GetJobLevelBonus(int depth, int jobs[], const int levels[]);
    JobResourceTemplate* const GetJobInfo(uint job_id);
    SummonResourceTemplate* const GetSummonBase(uint idx);
    MonsterBase* const GetMonsterInfo(uint idx);
    std::vector<MarketInfo>* const GetMarketInfo(const std::string&);

    int GetNeedJpForJobLevelUp(uint, uint);
    int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
    long GetNeedExp(int level);
    uint64 GetNeedSummonExp(int level);
    Monster* RespawnMonster(uint x, uint y, uint8_t layer, uint id, bool is_wandering, int way_point_id, /*IMonsterDeleteHandler pDeleteHandler,*/ bool bNeedLock);

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

    std::vector<NPC*>               _npcStore;

    ushort isLearnableSkill(Unit *pUnit, uint skill_id, int skill_level, int nJobID, int unit_job_level);
    void RegisterSkillTree(SkillTreeBase base);

    std::vector<SkillTreeBase> getSkillTree(int job_id);
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // _OBJECTMGR_H_