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

#include "RegionTester.h"

void DirectionRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    dx = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    dy = TargetPos.GetPositionY() - OriginalPos.GetPositionY();
    c = OriginalPos.GetPositionX() * dy - OriginalPos.GetPositionY() * dx;
    denominator = std::sqrt(dy * dy + dx * dx);
    thickness = RegionProperty * 12.0f * 0.5f;
    V1x = dx / denominator;
    V1y = dy / denominator;
    ori_x = OriginalPos.GetPositionX();
    ori_y = OriginalPos.GetPositionY();
}

bool DirectionRegionTester::IsInRegion(Position pos)
{
    auto _V2ya = std::abs(dx * pos.GetPositionY() - dy * pos.GetPositionX() + c);
    auto _V2yb = _V2ya / denominator;
    if(_V2yb >= thickness)
    {
        return false;
    }
    else
    {
        auto _x = pos.GetPositionX() - ori_x;
        auto _y = pos.GetPositionY() - ori_y;
        auto v6 = std::sqrt(_y * _y + _x * _x);
        auto _V2yd = _y / v6;
        auto v7 = _V2yd * V1y;
        auto _V2ye = _x / v6;
        return v7 + _V2ye * V1x >= 0.0f;
    }
}

void CrossRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    /*
     * I dont know either
       this->x1 = TargetPos->y - OriginalPos->y;
       *(float *)&OriginalPosa = OriginalPos->x - TargetPos->x;
       this->y1 = *(float *)&OriginalPosa;
     */
    y1 = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    x1 = TargetPos.GetPositionY() - OriginalPos.GetPositionY();


    c1 = 0.0f - x1 * OriginalPos.GetPositionX() - y1 - OriginalPos.GetPositionY();
    x2 = y1;
    y2 = x1;

    c1 = 0.0f - OriginalPos.GetPositionX() * x2 - x1 * OriginalPos.GetPositionY();
    denominator = std::sqrt(y1 * y1 + x1 * x1);
    thickness = RegionProperty * 12.0f * 0.5f;
}

bool CrossRegionTester::IsInRegion(Position pos)
{
    auto posa = std::abs(x1 * pos.GetPositionX() + y1 * pos.GetPositionY() + c1);

    if(posa / denominator >= thickness)
    {
        auto posb = std::abs(x2 * pos.GetPositionX() + y2 * pos.GetPositionY() + c2);
        if(posb / denominator >= thickness)
            return false;
    }
    return true;
}

void ArcCircleRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    auto _V1x = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    auto _V1y = TargetPos.GetPositionY() - OriginalPos.GetPositionY();
    float m = std::sqrt(_V1y * _V1y + _V1x * _V1x);
    if(m == 0.0f)
    {
        V1x = 1.0f;
        V1y = 0.0f;
    }
    else
    {
        V1x = _V1x / m;
        V1y = _V1y / m;
    }
    x = OriginalPos.GetPositionX();
    y = OriginalPos.GetPositionY();
    fCos = std::cos(RegionProperty);
}

bool ArcCircleRegionTester::IsInRegion(Position pos)
{
    auto  _V2x = pos.GetPositionX() - x;
    auto  _V2y = pos.GetPositionY() - y;
    float m    = std::sqrt(_V2y * _V2y + _V2x * _V2x);
    if (m == 0.0f)
    {
        return true;
    }
    auto  v4 = _V2y / m;
    float mb = _V2x / m;
    return v4 * V1y + mb * V1x >= fCos;
}
