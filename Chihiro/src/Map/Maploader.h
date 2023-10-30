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
#include "Common.h"
#include "MapLocationInfo.h"
#include "QuadTreeMapInfo.h"
#include "TerrainPropInfo.h"
#include "TerrainSeamlessWorld.h"

struct LocationInfoHeader {
    int32_t nPriority;
    float x;
    float y;
    float z;
    float fRadius;
};

struct NfsHeader {
    std::string szSign; // Data           :   this+0x0, Member, Type: char[0x10], szSign
    uint32_t dwVersion;
    uint32_t dwEventLocationOffset;
    uint32_t dwEventScriptOffset;
    uint32_t dwPropScriptOffset;
};

struct ScriptRegion {
    float left;
    float top;
    float right;
    float bottom;
    std::string szName;
};

struct ScriptTag {
    int32_t nTrigger;
    std::string strFunction;
};

struct ScriptRegionInfo {
    int32_t nRegionIndex;
    std::vector<ScriptTag> vInfoList;
};

class ByteBuffer;

class Maploader {
public:
    static Maploader &Instance()
    {
        static Maploader instance;
        return instance;
    }

    ~Maploader() = default;

    /// Initial map loadig
    bool LoadMapContent();

    void UnloadAll() { delete g_qtLocationInfo; }

    bool InitMapInfo();
    X2D::QuadTreeMapInfo *g_qtLocationInfo{nullptr};

    std::vector<ScriptRegion> m_vRegionList{};
    std::vector<ScriptRegionInfo> m_vScriptEvent{};
    int32_t nCurrentRegionIdx{0};

private:
    void SetDefaultLocation(int32_t x, int32_t y, float fMapLength, int32_t LocationId);
    void RegisterMapLocationInfo(MapLocationInfo location_info);
    void LoadLocationFile(const std::string &szFilename, int32_t x, int32_t y, float fAttrLen, float fMapLength);
    void LoadScriptFile(const std::string &szFilename, int32_t x, int32_t y, float fMapLength);
    void LoadAttributeFile(const std::string &szFileName, int32_t x, int32_t y, float fAttrLen, float fMapLength);
    void LoadFieldPropFile(const std::string &szFileName, int32_t x, int32_t y, float fAttrLen, float fMapLength);
    void LoadRegionInfo(ByteBuffer &buffer, int32_t x, int32_t y, float fMapLength);
    void LoadRegionScriptInfo(ByteBuffer &buffer);

    TerrainSeamlessWorldInfo seamlessWorldInfo{};
    TerrainPropInfo propInfo{};

    const int32_t g_nMapWidth = 700000;
    const int32_t g_nMapHeight = 1000000;

    float fTileSize{0};
    float fMapLength{0};

protected:
    Maploader() = default;
};

#define sMapContent Maploader::Instance()