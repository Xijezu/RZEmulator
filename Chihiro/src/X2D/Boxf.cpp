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

#include "Boxf.h"

bool X2D::Boxf::IsInclude(float x, float y)
{
    return x >= begin.x
           && x <= end.x
           && y >= begin.y
           && y <= end.y;
}

void X2D::Boxf::SetLeft(float x)
{
    begin.x = x;
    if (end.x < x) {
        begin.x = end.x;
        end.x   = x;
    }
}

void X2D::Boxf::SetTop(float y)
{
    begin.y = y;
    if (end.y < y) {
        begin.y = end.y;
        end.y   = y;
    }
}

void X2D::Boxf::SetRight(float x)
{
    end.x = x;
    if (x < begin.x) {
        end.x   = begin.x;
        begin.x = x;
    }
}

void X2D::Boxf::SetBottom(float y)
{
    end.y = y;
    if (y < begin.y) {
        end.y   = begin.y;
        begin.y = y;
    }
}

void X2D::Boxf::Set(X2D::Pointf _begin, X2D::Pointf _end)
{
    begin.x = _begin.x;
    begin.y = _begin.y;
    end.x   = _end.x;
    end.y   = _end.y;
    this->normalize();
}

void X2D::Boxf::normalize()
{
    float t;

    if (end.x < begin.x) {
        t = begin.x;
        begin.x = end.x;
        end.x   = t;
    }
    if (end.y < begin.y) {
        t = begin.y;
        begin.y = end.y;
        end.y   = t;
    }
}
