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

class ObjectMgr {
public:
    ObjectMgr()
    {};

    ~ObjectMgr()
    {};

    typedef UNORDERED_MAP<uint32, ItemTemplate>                 ItemTemplateContainer;
    typedef UNORDERED_MAP<uint32, CreatureStat>                 CreatureBaseStat;
    typedef UNORDERED_MAP<uint32, JobLevelBonusTemplate>        JobLevelBonusTemplateContainer;
    typedef UNORDERED_MAP<uint32, JobResourceTemplate>          JobResourceTemplateContainer;
    typedef UNORDERED_MAP<uint32, SummonResourceTemplate>       SummonResourceTemplateContainer;
    typedef std::vector<SkillTreeGroup>                         SkillTreeTemplateContainer;
    typedef UNORDERED_MAP<uint32, SkillBase>                    SkillBaseContainer;
    typedef UNORDERED_MAP<uint32, LevelResourceTemplate>        LevelTemplateContainer;
    typedef UNORDERED_MAP<std::string, std::vector<MarketInfo>> MarketResourceTemplateContainer;

    bool LoadStatResource();
    bool LoadJobResource();
    bool LoadJobLevelBonus();
    bool LoadNPCResource();
    bool LoadMonsterResource();
    bool LoadItemResource();
    bool LoadSummonResource();
    bool LoadMarketResource();
    bool LoadWorldLocation();
    bool LoadMapContent();
    bool LoadSkillResource();
    bool LoadLevelResource();
    bool LoadSkillTreeResource();
    bool LoadSkillJP();
    bool InitGameContent();

    CreatureStat GetStatInfo(int stat_id);
    ItemTemplate GetItemBase(int item_id);
    SkillBase GetSkillBase(int);
    CreatureStat GetJobLevelBonus(int depth, int jobs[], int levels[]);
    JobResourceTemplate GetJobInfo(int job_id);
    SummonResourceTemplate GetSummonBase(int idx);

    int GetNeedJpForJobLevelUp(int,int);
    int GetNeedJpForSkillLevelUp(int skill_id, int skill_level, int nJobID);
    long GetNeedExp(int level);

    std::vector<MarketInfo> GetMarketInfo(std::string);
    ushort IsLearnableSkill(Unit*,int,int, int&);

    int GetLocationID(float x, float y);
    int g_currentLocationId{0};

private:
    JobResourceTemplateContainer    _jobTemplateStore;
    ItemTemplateContainer           _itemTemplateStore;
    CreatureBaseStat                _creatureBaseStore;
    JobLevelBonusTemplateContainer  _jobBonusStore;
    SummonResourceTemplateContainer _summonResourceStore;
    MarketResourceTemplateContainer _marketResourceStore;
    SkillTreeTemplateContainer      _skillTreeResourceStore;
    LevelTemplateContainer          _levelResourceStore;
    SkillBaseContainer              _skillBaseStore;

    void SetDefaultLocation(int x, int y, float fMapLength, int LocationId);
    void RegisterMapLocationInfo(MapLocationInfo location_info);
    void LoadLocationFile(std::string szFilename,int x, int y, float fAttrLen, float fMapLength);
    ushort isLearnableSkill(Unit* pUnit, int skill_id, int skill_level,int nJobID, int unit_job_level);
    void RegisterSkillTree(SkillTreeBase base);

    std::vector<SkillTreeBase> getSkillTree(int job_id);

    X2D::QuadTreeMapInfo *g_qtLocationInfo{nullptr};

    const int g_nMapWidth = 700000;
    const int g_nMapHeight = 1000000;

    float fTileSize{};
    float fMapLength{};
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // _OBJECTMGR_H_