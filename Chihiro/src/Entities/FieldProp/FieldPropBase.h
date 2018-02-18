/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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

#ifndef PROJECT_FIELDPROPBASE_H
#define PROJECT_FIELDPROPBASE_H

#include "Common.h"

enum ACTIVATE_CHECK_TYPE : int
{
    CHECK_TYPE_ITEM                  = 1,
    CHECK_TYPE_QUEST                 = 2,
    CHECK_TYPE_SKILL                 = 3,
    CHECK_TYPE_WEAR                  = 4,
    CHECK_TYPE_SUMMON                = 5,
    CHECK_TYPE_PROP                  = 6,
    CHECK_TYPE_MEMBER                = 7,
    CHECK_TYPE_EXIST_NEAR_MOB        = 11,
    CHECK_TYPE_NON_EXIST_NEAR_MOB    = 12,
    CHECK_TYPE_OWN_TACTICAL_POSITION = 13,
};

struct DropItemInfo
{
    int code;                                        // 0x0
    int ratio;                                       // 0x4
    int min_count;                                   // 0x8
    int max_count;                                   // 0xC
    int min_level;                                   // 0x10
    int max_level;                                   // 0x14
};

struct FieldPropRespawnInfo
{
    FieldPropRespawnInfo() = default;

    FieldPropRespawnInfo(const FieldPropRespawnInfo &_src)
    {
        nPropID     = _src.nPropID;
        x           = _src.x;
        y           = _src.y;
        layer       = _src.layer;
        fZOffset    = _src.fZOffset;
        fRotateX    = _src.fRotateX;
        fRotateY    = _src.fRotateY;
        fRotateZ    = _src.fRotateZ;
        fScaleX     = _src.fScaleX;
        fScaleY     = _src.fScaleY;
        fScaleZ     = _src.fScaleZ;
        bLockHeight = _src.bLockHeight;
        fLockHeight = _src.fLockHeight;
        bOnce       = _src.bOnce;
    }

    int   nPropID;
    float x;
    float y;
    uint8 layer;
    float fZOffset;
    float fRotateX;
    float fRotateY;
    float fRotateZ;
    float fScaleX;
    float fScaleY;
    float fScaleZ;
    bool  bLockHeight;
    float fLockHeight;
    bool  bOnce;
};

struct FieldPropTemplate
{
    uint         nPropID;
    int          nPropTextID;
    int          nType;
    int          nLocalFlag;
    uint         nCastingTime;
    int          nUseCount;
    uint         nRegenTime;
    uint         nLifeTime;
    int          nMinLevel;
    int          nMaxLevel;
    int          nLimit;
    int          nLimitJobID;
    int          nActivateID[2];
    int          nActivateValue[2][2];
    int          nActivateSkillID;
    DropItemInfo drop_info[2];
    std::string  strScript;
};

#endif // PROJECT_FIELDPROPBASE_H
