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
    int left;
    int top;
    int right;
    int bottom;
};

struct KPoint
{
    int x;
    int y;
};

struct KSize
{
    int width;
    int cx;
    int height;
    int cy;
};

struct FilenameMapInfo
{
    std::string m_strMapFileName;
    int         m_nWorldID;
};

class TerrainSeamlessWorldInfo
{
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