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
#include <memory>

#include "Object.h"

struct RegionTester {
    virtual void Init(Position OriginalPos, Position TargetPos, float_t RegionProperty) = 0;
    virtual bool IsInRegion(Position pos) = 0;
    virtual ~RegionTester() = default;
};

struct DirectionRegionTester : public RegionTester {
    DirectionRegionTester()
        : V1x()
        , V1y()
        , ori_x()
        , ori_y()
        , dx()
        , dy()
        , c()
        , thickness()
        , denominator(){

          };
    ~DirectionRegionTester() = default;
    void Init(Position OriginalPos, Position TargetPos, float_t RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float_t V1x;
    float_t V1y;
    float_t ori_x;
    float_t ori_y;
    float_t dx;
    float_t dy;
    float_t c;
    float_t thickness;
    float_t denominator;
};

struct CrossRegionTester : public RegionTester {
    CrossRegionTester()
        : x1()
        , y1()
        , c1()
        , x2()
        , y2()
        , c2()
        , thickness()
        , denominator(){};

    void Init(Position OriginalPos, Position TargetPos, float_t RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float_t x1;
    float_t y1;
    float_t c1;
    float_t x2;
    float_t y2;
    float_t c2;
    float_t thickness;
    float_t denominator;
};

struct ArcCircleRegionTester : public RegionTester {
    ArcCircleRegionTester()
        : V1x()
        , V1y()
        , x()
        , y()
        , fCos(){

          };
    ~ArcCircleRegionTester() = default;
    void Init(Position OriginalPos, Position TargetPos, float_t RegionProperty) override;
    bool IsInRegion(Position pos) override;

    float_t V1x;
    float_t V1y;
    float_t x;
    float_t y;
    float_t fCos;
};

struct CircleRegionTester : public RegionTester {
    void Init(Position OriginalPos, Position TargetPos, float_t RegionProperty) override {}

    bool IsInRegion(Position pos) override { return true; }
};