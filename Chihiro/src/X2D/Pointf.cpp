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

#include "Pointf.h"

X2D::Pointf::Pointf(float _x, float _y)
{
    x = _x;
    y = _y;
}

float X2D::Pointf::GetAlternativeDistance(X2D::Pointf rh)
{
    float v2;
    float v3;

    v2     = x - rh.x;
    v3     = y - rh.y;
    if (v2 < 0)
        v2 = -v2;
    if (v3 < 0)
        v3 = -v3;
    return v3 + v2;
}

void X2D::Pointf::Set(float _x, float _y)
{
    x = _x;
    y = _y;
}

float X2D::Pointf::GetDistance(X2D::Pointf rh)
{
    return (float)sqrt((x - rh.x) * (x - rh.x) + (y - rh.y) * (y - rh.y));
}
