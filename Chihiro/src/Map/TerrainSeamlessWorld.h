#ifndef TERRAINSEAMLESSWORLD_H
#define TERRAINSEAMLESSWORLD_H

#include "Common.h"

struct KRect {
    int left;
    int top;
    int right;
    int bottom;
};

struct KPoint {
    int x;
    int y;
};

struct KSize {
    int width;
    int cx;
    int height;
    int cy;
};

struct FilenameMapInfo {
    std::string m_strMapFileName;
    int         m_nWorldID;
};

class TerrainSeamlessWorldInfo {
public:
    TerrainSeamlessWorldInfo() = default;
    ~TerrainSeamlessWorldInfo() = default;

    bool Initialize(std::string szFilename, bool bMapFileCheck);

    int GetWorldID(int nMapPosX, int nMapPosY);
    float GetFOV();
    std::string GetLQWaterFileName(int nMapPosX, int nMapPosY);
    std::string GetMinimapImageFileName(int nMapPosX, int nMapPosY);
    std::string GetFieldPropFileName(int nMapPosX, int nMapPosY);
    std::string GetAttributePolygonFileName(int nMapPosX, int nMapPosY);
    std::string GetLocationFileName(int nMapPosX, int nMapPosY);
    std::string GetScriptFileName(int nMapPosX, int nMapPosY);
    std::string GetMapFileName(int nMapPosX, int nMapPosY);
    std::string GetFileNameWithExt(int nMapPosX, int nMapPosY, std::string ext);

    int   m_nTileCountPerSegment{ };
    int   m_nSegmentCountPerMap{ };
    float m_fTileLength{ };
    float m_fFOV{ };
    KSize m_sizMapCount{ };
    int   m_nMapLayer{ };

    std::map<int, FilenameMapInfo> m_FileNameMap{ };
};

#endif // TERRAINSEAMLESSWORLD_H
