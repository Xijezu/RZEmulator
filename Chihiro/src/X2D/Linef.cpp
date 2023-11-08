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

#include "Linef.h"

X2D::Linef::Linef(X2D::Pointf p1, X2D::Pointf p2)
{
    begin = Pointf(p1.x, p1.y);
    end = Pointf(p2.x, p2.y);
}

X2D::CcwResult X2D::Linef::CheckClockWisef(float_t x1, float_t y1, float_t x2, float_t y2, float_t x3, float_t y3)
{
    CcwResult result{};
    float_t l = ((y3 - y1) * (x2 - x1)) - ((x3 - x1) * (y2 - y1));
    if (l <= 0.0f) {
        if (l >= 0.0f)
            result = Parallelism;
        else
            result = CounterClockWise;
    }
    else {
        result = ClockWise;
    }
    return result;
}

X2D::CcwResult X2D::Linef::CheckClockWisef(X2D::Pointf pt1, X2D::Pointf pt2, X2D::Pointf pt3)
{
    return CheckClockWisef(pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y);
}

X2D::Linef::IntersectResult X2D::Linef::IntersectCCW(X2D::Pointf p1, X2D::Pointf p2, X2D::Pointf p3, X2D::Pointf p4)
{
    if (std::max(p3.y, p4.y) < std::min(p1.y, p2.y) || std::max(p1.y, p2.y) < std::min(p3.y, p4.y) || std::max(p3.x, p4.x) < std::min(p1.x, p2.x) || std::max(p1.x, p2.x) < std::min(p3.x, p4.x))
        return Seperate;

    Pointf tp1{};
    Pointf tp2{};
    Pointf tp3{};
    Pointf tp4{};
    Pointf tp5{};
    Pointf tp6{};
    Pointf tp7{};
    Pointf tp8{};
    float_t f1{};
    float_t f2{};

    if (p2.x >= p1.x) {
        tp1 = p1;
        tp2 = p2;
    }
    else {
        tp1 = p2;
        tp2 = p1;
    }
    if (p2.x >= p3.x) {
        tp3 = p3;
        tp4 = p4;
    }
    else {
        tp3 = p4;
        tp4 = p3;
    }
    CcwResult ccw123 = CheckClockWisef(tp1, tp2, tp3);
    CcwResult ccw124 = CheckClockWisef(tp1, tp2, tp4);
    CcwResult ccw341 = CheckClockWisef(tp3, tp4, tp1);
    CcwResult ccw342 = CheckClockWisef(tp3, tp4, tp2);

    if ((int32_t)ccw123 * (int32_t)ccw124 < 0 && (int32_t)ccw341 * (int32_t)ccw342 < 0)
        return Intersect;
    if (ccw123 != Parallelism || ccw124 != Parallelism) {
        if (ccw123 != Parallelism && ccw124 != Parallelism && ccw341 != Parallelism && ccw342 != Parallelism)
            return Seperate;

        tp5 = tp2;
        if (tp2.y >= tp1.y) {
            tp6 = tp1;
        }
        else {
            tp6 = tp2;
            tp2 = tp1;
        }
        if (ccw123 != Parallelism) {
            if (tp3.x < tp1.x || tp5.x < tp3.x)
                return Seperate;
            if (tp1.x != tp3.x || tp5.x != tp1.x)
                return Touch;
            if (tp3.y >= tp6.y) {
                f1 = tp2.y;
                f2 = tp3.y;
                if (f2 <= f1)
                    return Touch;
            }
            return Seperate;
        }
        if (ccw124 != Parallelism) {
            if (tp3.y >= tp4.y) {
                tp7 = tp3;
                tp8 = tp4;
            }
            else {
                tp7 = tp4;
                tp8 = tp3;
            }
            if (ccw341 != Parallelism) {
                if (ccw342 != Parallelism || tp5.x < tp3.x || tp4.x < tp5.x)
                    return Seperate;
                if (tp3.x != tp5.x || tp4.x != tp3.x)
                    return Touch;
                if (tp5.y < tp7.y)
                    return Seperate;
                f1 = tp8.y;
                f2 = tp5.y;
            }
            else {
                if (tp1.x < tp3.x || tp4.x < tp1.x)
                    return Seperate;
                if (tp3.x != tp1.x || tp4.x != tp3.x)
                    return Touch;
                if (tp1.y < tp7.y)
                    return Seperate;
                f1 = tp8.y;
                f2 = tp1.y;
            }
        }
        else {
            if (tp4.x < tp1.x || tp5.x < tp4.x)
                return Seperate;
            if (tp1.x != tp4.x || tp5.x != tp1.x)
                return Touch;
            if (tp4.y < tp6.y)
                return Seperate;
            f1 = tp2.y;
            f2 = tp4.y;
        }
        if (f2 <= f1)
            return Touch;
        return Seperate;
    }

    if (tp2.x < tp4.x || tp3.x < tp1.x)
        return Seperate;
    return Touch;
}
