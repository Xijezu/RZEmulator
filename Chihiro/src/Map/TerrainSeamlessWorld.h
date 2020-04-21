#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include <map>

struct KRect
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

struct KPoint
{
    int32_t x;
    int32_t y;
};

struct KSize
{
    int32_t width;
    int32_t cx;
    int32_t height;
    int32_t cy;
};

struct FilenameMapInfo
{
    std::string m_strMapFileName;
    int32_t         m_nWorldID;
};

class TerrainSeamlessWorldInfo
{
    public:
        TerrainSeamlessWorldInfo() = default;
        ~TerrainSeamlessWorldInfo() = default;

        bool Initialize(std::string szFilename, bool bMapFileCheck);

        int32_t GetWorldID(int32_t nMapPosX, int32_t nMapPosY);
        float GetFOV();
        std::string GetLQWaterFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetMinimapImageFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetFieldPropFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetAttributePolygonFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetLocationFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetScriptFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetMapFileName(int32_t nMapPosX, int32_t nMapPosY);
        std::string GetFileNameWithExt(int32_t nMapPosX, int32_t nMapPosY, std::string ext);

        int32_t   m_nTileCountPerSegment{ };
        int32_t   m_nSegmentCountPerMap{ };
        float m_fTileLength{ };
        float m_fFOV{ };
        KSize m_sizMapCount{ };
        int32_t   m_nMapLayer{ };

        std::map<int, FilenameMapInfo> m_FileNameMap{ };
};