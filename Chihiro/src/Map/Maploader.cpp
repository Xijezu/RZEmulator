/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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

#include "Maploader.h"
#include "ObjectMgr.h"
#include "Scripting/XLua.h"
#include <fstream>

bool Maploader::LoadMapContent()
{
    if (!seamlessWorldInfo.Initialize("terrainseamlessworld.cfg", false)) {
        MX_LOG_FATAL("server.worldserver", "TerrainSeamlessWorld.cfg read error !");
        return false;
    }

    if(!propInfo.Initialize("terrainpropinfo.cfg")) {
        MX_LOG_FATAL("server.worldserver", "TerrainPropInfo.cfg read error !");
        return false;
    }

    int y = 0;
    fTileSize  = seamlessWorldInfo.m_fTileLength;
    fMapLength = seamlessWorldInfo.m_nSegmentCountPerMap * seamlessWorldInfo.m_fTileLength * seamlessWorldInfo.m_nTileCountPerSegment;
    for (float fAttrLen = seamlessWorldInfo.m_fTileLength * 0.125f; y < seamlessWorldInfo.m_sizMapCount.height; ++y) {
        for (int i = 0; i < seamlessWorldInfo.m_sizMapCount.width; ++i) {
            std::string strLocationFileName = seamlessWorldInfo.GetLocationFileName(i, y);

            if (strLocationFileName.length() != 0) {
                int wid = seamlessWorldInfo.GetWorldID(i, y);
                if (wid != -1) {
                    SetDefaultLocation(i, y, fMapLength, wid);
                }
                LoadLocationFile(("Resource/NewMap/"s+strLocationFileName), i, y, fAttrLen, fMapLength);

                std::string strScriptFileName = seamlessWorldInfo.GetScriptFileName(i, y);

                if(!strLocationFileName.empty()) {
                    MX_LOG_INFO("server.worldserver", "Loading Script: %s", strScriptFileName.c_str());

                    LoadScriptFile(("Resource/NewMap/"s+strScriptFileName), i, y, fMapLength);
                }
            }
        }
    }
    return true;
}

void Maploader::SetDefaultLocation(int x, int y, float fMapLength, int LocationId)
{
    X2D::Pointf begin{};
    X2D::Pointf end{};

    begin.x = (x * fMapLength);
    begin.y = (y * fMapLength);
    end.x = ((x + 1) * fMapLength);
    end.y = ((y + 1) * fMapLength);
    MapLocationInfo li(begin,end, LocationId, 2147483646);
    RegisterMapLocationInfo(li);
}

void Maploader::RegisterMapLocationInfo(MapLocationInfo location_info)
{
    if(g_qtLocationInfo == nullptr)
    {
        g_qtLocationInfo = new X2D::QuadTreeMapInfo(g_nMapWidth, g_nMapHeight);
    }
    g_qtLocationInfo->Add(std::move(location_info));
}

void Maploader::LoadLocationFile(std::string szFilename, int x, int y, float fAttrLen, float fMapLength)
{
    int nCharSize;
    LocationInfoHeader lih{};
    int nPolygonCount;
    int nPointCount;

    std::transform(szFilename.end()-12, szFilename.end(), szFilename.end()-12, tolower);

    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0,std::ios::end);
    int size = infile.tellg();
    infile.seekg(0,std::ios::beg);
    if(size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize(size);
    infile.read(reinterpret_cast<char*>(&buffer[0]), size);
    infile.close();

    auto total_entries = buffer.read<int>();
    for(int i = 0; i < total_entries; ++i) {
        lih.nPriority = buffer.read<int>();
        lih.x = buffer.read<float>();
        lih.y = buffer.read<float>();
        lih.z = buffer.read<float>();
        lih.fRadius = buffer.read<float>();

        nCharSize = buffer.read<int>();
        if(nCharSize > 1) {
            buffer.read_skip(nCharSize);
        }
        nCharSize = buffer.read<int>();
        sObjectMgr->g_currentLocationId = 0;
        if(nCharSize <= 1)
            continue;

        auto script = buffer.ReadString(nCharSize);
        sScriptingMgr->RunString(script);
        if(sObjectMgr->g_currentLocationId == 0)
            continue;

        nPolygonCount = buffer.read<int>();
        for(int cp = 0; cp < nPolygonCount; ++cp) {
            nPointCount = buffer.read<int>();
            float sx = x * fMapLength;
            float sy = y * fMapLength;
            std::vector<X2D::Pointf> points{};

            for(int p = 0; p < nPointCount; ++p) {
                X2D::Pointf pt{ };
                pt.x = sx + ((float) buffer.read<int>() * fAttrLen);
                pt.x = sy + ((float) buffer.read<int>() * fAttrLen);
                points.emplace_back(pt);
                auto location_info = MapLocationInfo(points, sObjectMgr->g_currentLocationId, 0);
                RegisterMapLocationInfo(location_info);
            }
        }
    }
}

void Maploader::LoadScriptFile(std::string szFilename, int x, int y, float fMapLength)
{
    NfsHeader header{};

    std::transform(szFilename.end()-12, szFilename.end(), szFilename.end()-12, tolower);
    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0,std::ios::end);
    int size = infile.tellg();
    infile.seekg(0,std::ios::beg);
    if(size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize(size);
    infile.read(reinterpret_cast<char*>(&buffer[0]), size);
    infile.close();

    header.szSign = buffer.ReadString(16);
    header.dwVersion = buffer.read<uint>();
    header.dwEventLocationOffset = buffer.read<uint>();
    header.dwEventScriptOffset = buffer.read<uint>();
    header.dwPropScriptOffset = buffer.read<uint>();

    if(header.szSign != "nFlavor Script"s) {
        MX_LOG_ERROR("server.worldserver", "[%s] Invalid Script Header: Sign: %s", szFilename.c_str(), header.szSign.c_str());
        return;
    }
    if(header.dwVersion != 2) {
        MX_LOG_ERROR("server.worldserver", "[%s] Invalid Script Header: Version: %d", szFilename.c_str(), header.dwVersion);
        return;
    }

    buffer.rpos(header.dwEventLocationOffset);
    LoadRegionInfo(buffer, x, y, fMapLength);
    buffer.rpos(header.dwEventScriptOffset);
    LoadRegionScriptInfo(buffer);
    m_vRegionList.clear();
}

void Maploader::LoadRegionInfo(ByteBuffer &buffer, int x, int y, float fMapLength)
{
    auto nLocationCount = buffer.read<int>();
    float sx = x * fMapLength;
    float sy = y * fMapLength;

    for(int i = 0; i < nLocationCount; ++i) {
        ScriptRegion sr{};
        sr.left = (fTileSize * (float)buffer.read<int>()) + sx;
        sr.top = (fTileSize * (float)buffer.read<int>()) + sy;
        sr.right = (fTileSize * (float)buffer.read<int>()) + sx;
        sr.bottom = (fTileSize * (float)buffer.read<int>()) + sy;
        auto nLength = buffer.read<int>();
        if(nLength > 0)
            sr.szName = buffer.ReadString(nLength);

        m_vRegionList.emplace_back(sr);
    }
}

void Maploader::LoadRegionScriptInfo(ByteBuffer &buffer)
{
    auto nScriptCount = buffer.read<int>();
    for(int i = 0; i < nScriptCount; ++i) {
        std::string szBox{};
        std::string szRight{};
        std::string szTop{};
        std::string szLeft{};
        std::string szBottom{};
        ScriptRegionInfo ri{};
        ri.nRegionIndex = buffer.read<int>();
        ri.nRegionIndex += nCurrentRegionIdx;
        ScriptRegion sr = m_vRegionList[ri.nRegionIndex];
        szRight = std::to_string(sr.right);
        szTop = std::to_string(sr.top);
        szLeft = std::to_string(sr.left);
        szBottom = std::to_string(sr.bottom);
        szBox = string_format("%s,%s,%s,%s", szLeft.c_str(),szTop.c_str(),szRight.c_str(),szBottom.c_str());
        auto nFunctionCount = buffer.read<int>();
        for(int x = 0; x < nFunctionCount; ++x) {
            ScriptTag nt{};
            nt.nTrigger = buffer.read<int>();
            auto cc = buffer.read<int>();
            if(cc > 0) {
                nt.strFunction = buffer.ReadString(cc);
                string_replace(nt.strFunction, "#LEFT", szLeft);
                string_replace(nt.strFunction, "#TOP", szTop);
                string_replace(nt.strFunction, "#RIGHT", szRight);
                string_replace(nt.strFunction, "#BOTTOM", szBottom);
                string_replace(nt.strFunction, "#BOX", szBox);
                string_replace(nt.strFunction, "#box", szBox);
            }
            ri.vInfoList.emplace_back(nt);
        }
        m_vScriptEvent.emplace_back(ri);
    }
}

bool Maploader::InitMapInfo()
{
    for(auto& ri : m_vScriptEvent) {
        for(auto& tag : ri.vInfoList) {
            if(tag.nTrigger == 0) {
                sScriptingMgr->RunString(tag.strFunction);
            }
        }
    }
    return true;
}
