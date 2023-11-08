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

enum ACTIVATE_CHECK_TYPE : int32_t {
    CHECK_TYPE_ITEM = 1,
    CHECK_TYPE_QUEST = 2,
    CHECK_TYPE_SKILL = 3,
    CHECK_TYPE_WEAR = 4,
    CHECK_TYPE_SUMMON = 5,
    CHECK_TYPE_PROP = 6,
    CHECK_TYPE_MEMBER = 7,
    CHECK_TYPE_EXIST_NEAR_MOB = 11,
    CHECK_TYPE_NON_EXIST_NEAR_MOB = 12,
    CHECK_TYPE_OWN_TACTICAL_POSITION = 13,
};

struct DropItemInfo {
    int32_t code; // 0x0
    int32_t ratio; // 0x4
    int32_t min_count; // 0x8
    int32_t max_count; // 0xC
    int32_t min_level; // 0x10
    int32_t max_level; // 0x14
};

struct FieldPropRespawnInfo {
    FieldPropRespawnInfo()
    {
        nPropId = 0;
        x = 0;
        y = 0;
        layer = 0;
        fZOffset = 0.0f;
        fRotateX = 0.0f;
        fRotateY = 0.0f;
        fRotateZ = 0.0f;
        fScaleX = 1.0f;
        fScaleY = 1.0f;
        fScaleY = 1.0f;
        bLockHeight = false;
        fLockHeight = 0.0f;
    }
    FieldPropRespawnInfo(const FieldPropRespawnInfo &) = default;
    FieldPropRespawnInfo &operator=(const FieldPropRespawnInfo &) = default;

    int32_t nPropId;
    float_t x, y;
    uint8_t layer;
    float_t fZOffset;
    float_t fRotateX;
    float_t fRotateY;
    float_t fRotateZ;
    float_t fScaleX;
    float_t fScaleY;
    float_t fScaleZ;
    bool bLockHeight;
    float_t fLockHeight;
};

struct RegenInfo {
    RegenInfo(const FieldPropRespawnInfo *_pRespawnInfo, uint32_t _tNextRegen)
        : pRespawnInfo(_pRespawnInfo)
        , tNextRegen(_tNextRegen)
    {
    }

    const FieldPropRespawnInfo *pRespawnInfo;
    uint32_t tNextRegen;
};


struct FieldPropTemplate {
    uint32_t nPropID;
    int32_t nPropTextID;
    int32_t nType;
    int32_t nLocalFlag;
    uint32_t nCastingTime;
    int32_t nUseCount;
    uint32_t nRegenTime;
    uint32_t nLifeTime;
    int32_t nMinLevel;
    int32_t nMaxLevel;
    int32_t nLimit;
    int32_t nLimitJobID;
    int32_t nActivateID[2];
    int32_t nActivateValue[2][2];
    int32_t nActivateSkillID;
    DropItemInfo drop_info[2];
    std::string strScript;
};