#ifndef _OBJECTMGR_H_
#define _OBJECTMGR_H_

#include "Common.h"
#include "DatabaseTemplates.h"
#include <unordered_map>
#include <Item/Item.h>
#include <Player/Player.h>
#include "QuadTreeMapInfo.h"
#include "Dynamic/UnorderedMap.h"

class ObjectMgr {
public:
    ObjectMgr()
    {};

    ~ObjectMgr()
    {};

    typedef UNORDERED_MAP<uint32, ItemTemplate>           ItemTemplateContainer;
    typedef UNORDERED_MAP<uint32, CreatureStat>           CreatureBaseStat;
    typedef UNORDERED_MAP<uint32, JobLevelBonusTemplate>  JobLevelBonusTemplateContainer;
    typedef UNORDERED_MAP<uint32, JobResourceTemplate>    JobResourceTemplateContainer;
    typedef UNORDERED_MAP<uint32, SummonResourceTemplate> SummonResourceTemplateContainer;
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
    bool InitGameContent();

    CreatureStat GetStatInfo(int stat_id);
    ItemTemplate GetItemBase(int item_id);
    CreatureStat GetJobLevelBonus(int depth, int jobs[], int levels[]);
    JobResourceTemplate GetJobInfo(int job_id);
    SummonResourceTemplate GetSummonBase(int idx);
    std::vector<MarketInfo> GetMarketInfo(std::string);
    int GetLocationID(float x, float y);
    int g_currentLocationId{0};

private:
    JobResourceTemplateContainer    _jobTemplateStore;
    ItemTemplateContainer           _itemTemplateStore;
    CreatureBaseStat                _creatureBaseStore;
    JobLevelBonusTemplateContainer  _jobBonusStore;
    SummonResourceTemplateContainer _summonResourceStore;
    MarketResourceTemplateContainer _marketResourceStore;

    void SetDefaultLocation(int x, int y, float fMapLength, int LocationId);
    void RegisterMapLocationInfo(MapLocationInfo location_info);
    void LoadLocationFile(std::string szFilename,int x, int y, float fAttrLen, float fMapLength);

    X2D::QuadTreeMapInfo *g_qtLocationInfo{nullptr};

    const int g_nMapWidth = 700000;
    const int g_nMapHeight = 1000000;

    float fTileSize{};
    float fMapLength{};
};

#define sObjectMgr ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance()

#endif // _OBJECTMGR_H_