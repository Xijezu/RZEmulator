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

enum PropType : int
{
    PT_Unused = 0,
    PT_UseNAF = 1,
    PT_UseNX3 = 2,
    PT_SpeedTree = 3,
    PT_NPC = 4
};

enum RenderType : int
{
    RT_General = 0,
    RT_Building = 1
};

struct PropInfo
{
    int nLineIndex;
    PropType Type;
    RenderType mRenderType;
    int nCategory;
    float fVisibleRatio;
    int nShadowFlag;
    std::string strName;
};

class TerrainPropInfo
{
  public:
    TerrainPropInfo() = default;
    ~TerrainPropInfo() = default;

    bool Initialize(std::string szFileName);
    void Release();

    std::vector<PropInfo> m_pPropInfo{};
    std::vector<std::string> m_CategoryNames{};

  private:
    int GetShadowFlag(std::string str);
    bool CheckPropFileType(std::string rname, std::string szTail);
    void SetPropInfo(int nLineIndex, uint16_t wPropNum, PropType propType, RenderType renderType, int nCategory, float fVisibleRatio, std::string rName, int nShadowFlag);
};