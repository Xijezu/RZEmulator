#ifndef PROJECT_MAPLOADER_H
#define PROJECT_MAPLOADER_H

#include "Common.h"
#include "TerrainSeamlessWorld.h"
#include "TerrainPropInfo.h"
#include "MapLocationInfo.h"
#include "QuadTreeMapInfo.h"

struct LocationInfoHeader {
    int   nPriority;
    float x;
    float y;
    float z;
    float fRadius;
};

struct NfsHeader {
    std::string szSign;//Data           :   this+0x0, Member, Type: char[0x10], szSign
    uint        dwVersion;
    uint        dwEventLocationOffset;
    uint        dwEventScriptOffset;
    uint        dwPropScriptOffset;
};

struct ScriptRegion {
    float       left;
    float       top;
    float       right;
    float       bottom;
    std::string szName;
};

struct ScriptTag {
    int nTrigger;
    std::string strFunction;
};

struct ScriptRegionInfo {
    int nRegionIndex;
    std::vector<ScriptTag> vInfoList;
};

class ByteBuffer;

class Maploader {
public:
    Maploader() = default;
    ~Maploader() = default;

    /// Initial map loadig
    bool LoadMapContent();
    void UnloadAll() { delete g_qtLocationInfo; }
    bool InitMapInfo();
    X2D::QuadTreeMapInfo *g_qtLocationInfo{nullptr};

    std::vector<ScriptRegion> m_vRegionList{};
    std::vector<ScriptRegionInfo> m_vScriptEvent{};
    int nCurrentRegionIdx{0};
private:
    void SetDefaultLocation(int x, int y, float fMapLength, int LocationId);
    void RegisterMapLocationInfo(MapLocationInfo location_info);
    void LoadLocationFile(std::string szFilename, int x, int y, float fAttrLen, float fMapLength);
    void LoadScriptFile(std::string szFilename, int x, int y, float fMapLength);
    void LoadAttributeFile(std::string szFileName, int x, int y, float fAttrLen, float fMapLength);
    void LoadRegionInfo(ByteBuffer &buffer, int x, int y, float fMapLength);
    void LoadRegionScriptInfo(ByteBuffer &buffer);

    TerrainSeamlessWorldInfo seamlessWorldInfo{ };
    TerrainPropInfo          propInfo{ };

    const int g_nMapWidth  = 700000;
    const int g_nMapHeight = 1000000;

    float fTileSize{0};
    float fMapLength{0};

};

#define sMapContent ACE_Singleton<Maploader,ACE_Thread_Mutex>::instance()

#endif // PROJECT_MAPLOADER_H
