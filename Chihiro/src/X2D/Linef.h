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
#include "Pointf.h"

namespace X2D
{
    class Linef
    {
        public:
            enum IntersectResult : int
            {
                None      = 157,
                Intersect = 1,
                Seperate  = -1,
                Touch     = 0,
            };

            Linef() = default;
            ~Linef() = default;
            Linef(Pointf p1, Pointf p2);

            static CcwResult CheckClockWisef(float x1, float y1, float x2, float y2, float x3, float y3);
            static CcwResult CheckClockWisef(Pointf pt1, Pointf pt2, Pointf pt3);
            static IntersectResult IntersectCCW(X2D::Pointf p1, X2D::Pointf p2, X2D::Pointf p3, X2D::Pointf p4);

            Pointf begin{ }, end{ };
    };
}