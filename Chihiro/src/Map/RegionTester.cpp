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

#include "RegionTester.h"

void DirectionRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    dx = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    dy = TargetPos.GetPositionY() - OriginalPos.GetPositionY();
    c = dy * OriginalPos.GetPositionX() - dx * OriginalPos.GetPositionY();

    denominator = sqrt(dx * dx + dy * dy);
    thickness = RegionProperty * 12.0f / 2;

    float _V1x = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    float _V1y = TargetPos.GetPositionY() - OriginalPos.GetPositionY();

    float m = std::sqrt(_V1x * _V1x + _V1y * _V1y);
    V1x = _V1x / m;
    V1y = _V1y / m;

    ori_x = OriginalPos.GetPositionX();
    ori_y = OriginalPos.GetPositionY();
}

bool DirectionRegionTester::IsInRegion(Position pos)
{
    float dist = abs(-dy * pos.GetPositionX() + dx * pos.GetPositionY() + c) / denominator;
    if (thickness > dist)
    {
        float _V2x = pos.GetPositionX() - ori_x;
        float _V2y = pos.GetPositionY() - ori_y;

        float V2x, V2y;

        float m = std::sqrt(_V2x * _V2x + _V2y * _V2y);
        V2x = _V2x / m;
        V2y = _V2y / m;

        return 0 <= (V1x * V2x + V1y * V2y);
    }
    return false;
}

void CrossRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    x1 = TargetPos.GetPositionY() - OriginalPos.GetPositionY();
    y1 = OriginalPos.GetPositionX() - TargetPos.GetPositionX();
    c1 = 0 - x1 * OriginalPos.GetPositionX() - y1 * OriginalPos.GetPositionY();

    x2 = y1;
    y2 = -x1;
    c1 = 0 - x2 * OriginalPos.GetPositionX() - y2 * OriginalPos.GetPositionY();

    denominator = std::sqrt(x1 * x1 + y1 * y1);
    thickness = RegionProperty * 12.0f / 2;
}

bool CrossRegionTester::IsInRegion(Position pos)
{
    return (thickness > (std::abs(x1 * pos.GetPositionX() + y1 * pos.GetPositionY() + c1) / denominator)) ||
        (thickness > (std::abs(x2 * pos.GetPositionX() + y2 * pos.GetPositionY() + c2) / denominator));
}

void ArcCircleRegionTester::Init(Position OriginalPos, Position TargetPos, float RegionProperty)
{
    float _V1x = TargetPos.GetPositionX() - OriginalPos.GetPositionX();
    float _V1y = TargetPos.GetPositionY() - OriginalPos.GetPositionY();

    float m = std::sqrt(_V1x * _V1x + _V1y * _V1y);

    if (m == 0)
    {
        V1x = 1;
        V1y = 0;
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
    float _V2x = pos.GetPositionX() - x;
    float _V2y = pos.GetPositionY() - y;

    float V2x, V2y;

    float m = std::sqrt(_V2x * _V2x + _V2y * _V2y);

    if (m == 0)
        return true;

    V2x = _V2x / m;
    V2y = _V2y / m;

    return fCos <= (V1x * V2x + V1y * V2y);
}
