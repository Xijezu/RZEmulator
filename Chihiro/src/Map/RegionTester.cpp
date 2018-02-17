/*
  *  Copyright (C) 2018 Xijezu <http://xijezu.com/>
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
    ACE_NOTREACHED(return false);
}
