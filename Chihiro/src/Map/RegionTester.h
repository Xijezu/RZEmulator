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

#ifndef NGEMITY_REGIONTESTER_H
#define NGEMITY_REGIONTESTER_H

#include "Object.h"

class RegionTester
{
    public:
        virtual void Init(Position OriginalPos, Position TargetPos, float RegionProperty) = 0;
        virtual bool IsInRegion(Position pos) = 0;
};

class DirectionRegionTester : public RegionTester
{
    public:
        DirectionRegionTester() : V1x(), V1y(), ori_x(), ori_y(), dx(), dy(), c(), thickness(), denominator() { };
        ~DirectionRegionTester() = default;
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
    private:
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

class CrossRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
    private:
};

class ArcCircleRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
};

class CircleRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
};



#endif // NGEMITY_REGIONTESTER_H
