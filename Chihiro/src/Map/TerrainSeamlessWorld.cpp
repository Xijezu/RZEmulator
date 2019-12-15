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

#include "TerrainSeamlessWorld.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include "Config.h"
#include "Util.h"

namespace fs = boost::filesystem;

bool TerrainSeamlessWorldInfo::Initialize(std::string szFilename, bool bMapFileCheck)
{
    int32_t   nWorldMapID          = 0;
    int32_t   nMapIndex            = 0;
    int32_t   nMapLayer            = 0;
    int32_t   nTileCountPerSegment = 0;
    int32_t   nSegmentCountPerMap  = 0;
    float fTileLength          = 0.0f;
    float pStream              = 0.0f;
    int32_t   nMapCountX           = 0;
    int32_t   nMapCountY           = 0;

    std::vector<std::string> TextLines{ };

    auto configFile = boost::filesystem::path(ConfigMgr::instance()->GetCorrectPath("Resource/NewMap/"+szFilename));

    std::ifstream ifstream(configFile.c_str(), std::ios::in);
    std::string   row;
    while (std::getline(ifstream, row))
    {
        if (!row.empty() || row[0] == ';')
            TextLines.push_back(row);
    }
    ifstream.close();

    if (TextLines.empty())
        return false;

    for (auto &s : TextLines)
    {
        Tokenizer lines(s, '=');
        if (lines.size() < 2)
            continue;

        if (lines[0] == "TILE_LENGTH"s)
        {
            fTileLength = std::stof(lines[1]);
        }
        else if (lines[0] == "TILECOUNT_PER_SEGMENT"s)
        {
            nTileCountPerSegment = std::stoi(lines[1]);
        }
        else if (lines[0] == "SEGMENTCOUNT_PER_MAP"s)
        {
            nSegmentCountPerMap = std::stoi(lines[1]);
        }
        else if (lines[0] == "SETFOV"s)
        {
            pStream = std::stof(lines[1]);
        }
        else if (lines[0] == "MAPLAYER"s)
        {
            nMapLayer = std::stoi(lines[1]);
        }
        else if (lines[0] == "MAPSIZE"s)
        {
            Tokenizer tokens(std::string(lines[1]), ',');
            if (tokens.size() != 2)
                return false;
            nMapCountX = std::stoi(tokens[0]);
            nMapCountY = std::stoi(tokens[1]);
        }
        else if (lines[0] == "MAPFILE"s)
        {
            Tokenizer vars(std::string(lines[1]), ',');
            if (vars.size() != 5)
                return false;

            nWorldMapID = std::stoi(vars[4]);
            nMapLayer   = std::stoi(vars[2]);
            nMapIndex   = std::stoi(vars[0]) + (nMapCountX * std::stoi(vars[1]));
            FilenameMapInfo fnmi{ };
            fnmi.m_nWorldID       = nWorldMapID;
            fnmi.m_strMapFileName = std::string(vars[3]);
            if (!m_FileNameMap.count(nMapIndex))
                m_FileNameMap.emplace(nMapIndex, fnmi);
        }
    }

    if (fTileLength <= 0.0f || nTileCountPerSegment <= 0 || nSegmentCountPerMap <= 0 || m_FileNameMap.empty())
        return false;

    m_fTileLength          = fTileLength;
    m_nTileCountPerSegment = nTileCountPerSegment;
    m_nSegmentCountPerMap  = nSegmentCountPerMap;
    m_nMapLayer            = nMapLayer;
    m_sizMapCount.width  = nMapCountX;
    m_sizMapCount.height = nMapCountY;
    if (pStream > 0.0f)
        m_fFOV = pStream;

    return !bMapFileCheck || (nMapCountY <= 0);
}

int32_t TerrainSeamlessWorldInfo::GetWorldID(int32_t nMapPosX, int32_t nMapPosY)
{
    int32_t nMapIndex = nMapPosX + (m_sizMapCount.width * nMapPosY);
    if (m_FileNameMap.count(nMapIndex) == 1)
        return m_FileNameMap[nMapIndex].m_nWorldID;
    return -1;
}

float TerrainSeamlessWorldInfo::GetFOV()
{
    return m_fFOV;
}

std::string TerrainSeamlessWorldInfo::GetLQWaterFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "nfw");
}

std::string TerrainSeamlessWorldInfo::GetMinimapImageFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "bmp");
}

std::string TerrainSeamlessWorldInfo::GetFieldPropFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "qpf");
}

std::string TerrainSeamlessWorldInfo::GetAttributePolygonFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "nfa");
}

std::string TerrainSeamlessWorldInfo::GetLocationFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "nfc");
}

std::string TerrainSeamlessWorldInfo::GetScriptFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "nfs");
}

std::string TerrainSeamlessWorldInfo::GetMapFileName(int32_t nMapPosX, int32_t nMapPosY)
{
    return GetFileNameWithExt(nMapPosX, nMapPosY, "nfm");
}

std::string TerrainSeamlessWorldInfo::GetFileNameWithExt(int32_t nMapPosX, int32_t nMapPosY, std::string ext)
{
    int32_t nMapIndex = nMapPosX + (m_sizMapCount.width * nMapPosY);
    if (m_FileNameMap.count(nMapIndex) == 0)
        return "";
    return m_FileNameMap[nMapIndex].m_strMapFileName + "."s + ext;
}
