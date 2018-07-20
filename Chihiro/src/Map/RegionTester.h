#pragma once
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
#include "Object.h"
#include <memory>

struct RegionTester
{
    virtual void Init(Position OriginalPos, Position TargetPos, float RegionProperty) = 0;
    virtual bool IsInRegion(Position pos) = 0;
    virtual ~RegionTester() = default;
};

struct DirectionRegionTester : public RegionTester
{
    DirectionRegionTester() : V1x(), V1y(), ori_x(), ori_y(), dx(), dy(), c(), thickness(), denominator()
    {

    };
    ~DirectionRegionTester() = default;
    void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float V1x;
    float V1y;
    float ori_x;
    float ori_y;
    float dx;
    float dy;
    float c;
    float thickness;
    float denominator;
};

struct CrossRegionTester : public RegionTester
{
    CrossRegionTester() : x1(), y1(), c1(), x2(), y2(), c2(), thickness(), denominator()
    {
    };

    void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float x1;
    float y1;
    float c1;
    float x2;
    float y2;
    float c2;
    float thickness;
    float denominator;
};

struct ArcCircleRegionTester : public RegionTester
{
    ArcCircleRegionTester() : V1x(), V1y(), x(), y(), fCos()
    {

    };
    ~ArcCircleRegionTester() = default;
    void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float V1x;
    float V1y;
    float x;
    float y;
    float fCos;
};

struct CircleRegionTester : public RegionTester
{
    void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override
    {

    }

    bool IsInRegion(Position pos) override
    {
        return true;
    }
};