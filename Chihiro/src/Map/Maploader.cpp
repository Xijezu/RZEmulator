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

#include "Maploader.h"

#include <fstream>

#include "FieldPropManager.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Scripting/XLua.h"

bool Maploader::LoadMapContent()
{
    if (!seamlessWorldInfo.Initialize("terrainseamlessworld.cfg", false)) {
        NG_LOG_FATAL("server.worldserver", "TerrainSeamlessWorld.cfg read error !");
        return false;
    }

    if (!propInfo.Initialize("terrainpropinfo.cfg")) {
        NG_LOG_FATAL("server.worldserver", "TerrainPropInfo.cfg read error !");
        return false;
    }

    int32_t y = 0;
    fTileSize = seamlessWorldInfo.m_fTileLength;
    fMapLength = seamlessWorldInfo.m_nSegmentCountPerMap * seamlessWorldInfo.m_fTileLength * seamlessWorldInfo.m_nTileCountPerSegment;
    for (float_t fAttrLen = seamlessWorldInfo.m_fTileLength * 0.125f; y < seamlessWorldInfo.m_sizMapCount.height; ++y) {
        for (int32_t i = 0; i < seamlessWorldInfo.m_sizMapCount.width; ++i) {
            std::string strLocationFileName = seamlessWorldInfo.GetLocationFileName(i, y);

            if (strLocationFileName.length() != 0) {
                int32_t wid = seamlessWorldInfo.GetWorldID(i, y);
                if (wid != -1) {
                    SetDefaultLocation(i, y, fMapLength, wid);
                }

                std::transform(strLocationFileName.begin(), strLocationFileName.end(), strLocationFileName.begin(), tolower);
                LoadLocationFile(("Resource/NewMap/"s + strLocationFileName), i, y, fAttrLen, fMapLength);

                std::string strScriptFileName = seamlessWorldInfo.GetScriptFileName(i, y);
                if (!strScriptFileName.empty()) {
                    std::transform(strScriptFileName.begin(), strScriptFileName.end(), strScriptFileName.begin(), tolower);
                    LoadScriptFile(("Resource/NewMap/"s + strScriptFileName), i, y, fMapLength);

                    std::string strAttributeFileName = seamlessWorldInfo.GetAttributePolygonFileName(i, y);

                    if (!strAttributeFileName.empty()) {
                        std::transform(strAttributeFileName.begin(), strAttributeFileName.end(), strAttributeFileName.begin(), tolower);
                        LoadAttributeFile(("Resource/NewMap/"s + strAttributeFileName), i, y, fAttrLen, fMapLength);
                    }
                }

                std::string strPropFileName = seamlessWorldInfo.GetFieldPropFileName(i, y);
                if (!strPropFileName.empty()) {
                    std::transform(strPropFileName.begin(), strPropFileName.end(), strPropFileName.begin(), tolower);
                    LoadFieldPropFile(("Resource/NewMap/"s + strPropFileName), i, y, fAttrLen, fMapLength);
                }
            }
        }
    }
    return true;
}

void Maploader::SetDefaultLocation(int32_t x, int32_t y, float_t fMapLength, int32_t LocationId)
{
    X2D::Pointf begin{};
    X2D::Pointf end{};

    begin.x = (x * fMapLength);
    begin.y = (y * fMapLength);
    end.x = ((x + 1) * fMapLength);
    end.y = ((y + 1) * fMapLength);
    MapLocationInfo li(begin, end, LocationId, 2147483646);
    RegisterMapLocationInfo(li);
}

void Maploader::RegisterMapLocationInfo(MapLocationInfo location_info)
{
    if (g_qtLocationInfo == nullptr) {
        g_qtLocationInfo = new X2D::QuadTreeMapInfo(g_nMapWidth, g_nMapHeight);
    }
    g_qtLocationInfo->Add(std::move(location_info));
}

void Maploader::LoadLocationFile(const std::string &szFilename, int32_t x, int32_t y, float_t fAttrLen, float_t fMapLength)
{
    int32_t nCharSize;
    LocationInfoHeader lih;
    int32_t nPolygonCount;
    int32_t nPointCount;

    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0, std::ios::end);
    int64_t size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    if (size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize((size_t)size);
    infile.read(reinterpret_cast<char *>(&buffer[0]), size);
    infile.close();

    auto total_entries = buffer.read<int32_t>();
    for (int32_t i = 0; i < total_entries; ++i) {
        lih.nPriority = buffer.read<int32_t>();
        lih.x = buffer.read<float>();
        lih.y = buffer.read<float>();
        lih.z = buffer.read<float>();
        lih.fRadius = buffer.read<float>();

        nCharSize = buffer.read<int32_t>();
        if (nCharSize > 1) {
            buffer.read_skip((size_t)nCharSize);
        }
        nCharSize = buffer.read<int32_t>();
        sObjectMgr.g_currentLocationId = 0;
        if (nCharSize <= 1)
            continue;

        auto script = buffer.ReadString((uint32_t)nCharSize);
        sScriptingMgr.RunString(script);
        if (sObjectMgr.g_currentLocationId == 0)
            continue;

        nPolygonCount = buffer.read<int32_t>();
        for (int32_t cp = 0; cp < nPolygonCount; ++cp) {
            nPointCount = buffer.read<int32_t>();
            float_t sx = x * fMapLength;
            float_t sy = y * fMapLength;
            std::vector<X2D::Pointf> points{};

            for (int32_t p = 0; p < nPointCount; ++p) {
                X2D::Pointf pt{};
                pt.x = sx + ((float)buffer.read<int32_t>() * fAttrLen);
                pt.x = sy + ((float)buffer.read<int32_t>() * fAttrLen);
                points.emplace_back(pt);
                auto location_info = MapLocationInfo(points, sObjectMgr.g_currentLocationId, lih.nPriority);
                RegisterMapLocationInfo(location_info);
            }
        }
    }
}

void Maploader::LoadAttributeFile(const std::string &szFilename, int32_t x, int32_t y, float_t fAttrLen, float_t fMapLength)
{
    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0, std::ios::end);
    int64_t size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    if (size == -1)
        return;

    ByteBuffer buffer{};
    buffer.resize((size_t)size);
    infile.read(reinterpret_cast<char *>(&buffer[0]), size);
    infile.close();

    auto total_entries = buffer.read<int32_t>();
    float_t sx = x * fMapLength;
    float_t sy = y * fMapLength;

    for (int32_t i = 0; i < total_entries; ++i) {
        auto nPointCount = buffer.read<int32_t>();

        std::vector<X2D::Pointf> points{};
        for (int32_t p = 0; p < nPointCount; ++p) {
            X2D::Pointf pt{};
            pt.x = sx + ((float)buffer.read<int32_t>() * fAttrLen);
            pt.y = sy + ((float)buffer.read<int32_t>() * fAttrLen);
            points.emplace_back(pt);
        }
        X2D::PolygonF pg{points};
        sObjectMgr.g_qtBlockInfo.Add({points, 0, 0});
    }
}

void Maploader::LoadScriptFile(const std::string &szFilename, int32_t x, int32_t y, float_t fMapLength)
{
    NfsHeader header{};

    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0, std::ios::end);
    int64_t size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    if (size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize((size_t)size);
    infile.read(reinterpret_cast<char *>(&buffer[0]), size);
    infile.close();

    header.szSign = buffer.ReadString(16);
    header.dwVersion = buffer.read<uint32_t>();
    header.dwEventLocationOffset = buffer.read<uint32_t>();
    header.dwEventScriptOffset = buffer.read<uint32_t>();
    header.dwPropScriptOffset = buffer.read<uint32_t>();

    if (header.szSign != "nFlavor Script"s) {
        NG_LOG_ERROR("server.worldserver", "[%s] Invalid Script Header: Sign: %s", szFilename.c_str(), header.szSign.c_str());
        return;
    }
    if (header.dwVersion != 2) {
        NG_LOG_ERROR("server.worldserver", "[%s] Invalid Script Header: Version: %d", szFilename.c_str(), header.dwVersion);
        return;
    }

    buffer.rpos(header.dwEventLocationOffset);
    LoadRegionInfo(buffer, x, y, fMapLength);
    buffer.rpos(header.dwEventScriptOffset);
    LoadRegionScriptInfo(buffer);
    m_vRegionList.clear();
}

void Maploader::LoadRegionInfo(ByteBuffer &buffer, int32_t x, int32_t y, float_t fMapLength)
{
    auto nLocationCount = buffer.read<int32_t>();
    float_t sx = x * fMapLength;
    float_t sy = y * fMapLength;

    for (int32_t i = 0; i < nLocationCount; ++i) {
        ScriptRegion sr{};
        sr.left = (fTileSize * (float)buffer.read<int32_t>()) + sx;
        sr.top = (fTileSize * (float)buffer.read<int32_t>()) + sy;
        sr.right = (fTileSize * (float)buffer.read<int32_t>()) + sx;
        sr.bottom = (fTileSize * (float)buffer.read<int32_t>()) + sy;
        auto nLength = buffer.read<int32_t>();
        if (nLength > 0)
            sr.szName = buffer.ReadString((uint32_t)nLength);

        m_vRegionList.emplace_back(sr);
    }
}

void Maploader::LoadRegionScriptInfo(ByteBuffer &buffer)
{
    auto nScriptCount = buffer.read<int32_t>();
    for (int32_t i = 0; i < nScriptCount; ++i) {
        std::string szBox{};
        std::string szRight{};
        std::string szTop{};
        std::string szLeft{};
        std::string szBottom{};

        ScriptRegionInfo ri{};
        ri.nRegionIndex = buffer.read<int32_t>();
        ri.nRegionIndex += nCurrentRegionIdx;
        ScriptRegion sr = m_vRegionList[ri.nRegionIndex];
        szRight = std::to_string(sr.right);
        szTop = std::to_string(sr.top);
        szLeft = std::to_string(sr.left);
        szBottom = std::to_string(sr.bottom);
        szBox = NGemity::StringFormat("{},{},{},{}", szLeft, szTop, szRight, szBottom);
        auto nFunctionCount = buffer.read<int32_t>();
        for (int32_t x = 0; x < nFunctionCount; ++x) {
            ScriptTag nt{};
            nt.nTrigger = buffer.read<int32_t>();
            auto cc = buffer.read<int32_t>();
            if (cc > 0) {
                nt.strFunction = buffer.ReadString((uint32_t)cc);
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
    for (auto &ri : m_vScriptEvent) {
        for (auto &tag : ri.vInfoList) {
            if (tag.nTrigger == 0) {
                sScriptingMgr.RunString(tag.strFunction);
            }
        }
    }
    return true;
}

void Maploader::LoadFieldPropFile(const std::string &szFilename, int32_t x, int32_t y, float_t /* fAttrLen*/, float_t fMapLength)
{
    std::ifstream infile(szFilename.c_str(), std::ios::in | std::ios::binary);
    infile.seekg(0, std::ios::end);
    int64_t size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    if (size == -1)
        return;
    ByteBuffer buffer{};
    buffer.resize((size_t)size);
    infile.read(reinterpret_cast<char *>(&buffer[0]), size);
    infile.close();

    buffer.read_skip(18); // Sign
    auto version = buffer.read<int32_t>(); // Version

    auto total_entries = buffer.read<int32_t>();
    float_t rx = x * fMapLength;
    float_t ry = y * fMapLength;

    for (int32_t i = 0; i < total_entries; ++i) {
        FieldPropRespawnInfo sr{};
        sr.nPropId = buffer.read<int32_t>();
        sr.x = buffer.read<float>() + rx;
        sr.y = buffer.read<float>() + ry;
        sr.fZOffset = buffer.read<float>();
        sr.fRotateX = buffer.read<float>();
        sr.fRotateY = buffer.read<float>();
        sr.fRotateZ = buffer.read<float>();
        sr.fScaleX = buffer.read<float>();
        sr.fScaleY = buffer.read<float>();
        sr.fScaleZ = buffer.read<float>();

        sr.layer = 0;
        if (version == 2)
            buffer.read_skip(7);
        else if (version == 3)
            buffer.read_skip(9);
        else
            buffer.read_skip(2);
        sFieldPropManager.RegisterFieldProp(sr);
    }
}
