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

namespace X2D {
    enum CcwResult : int {
        Parallelism = 0,
        ClockWise = 1,
        CounterClockWise = -1,
    };

    struct PointBasef {
        float x;
        float y;
    };

    class Pointf {
    public:
        Pointf() = default;
        ~Pointf() = default;

        Pointf(float _x, float _y);

        float GetX() { return x; }

        float GetY() { return y; }

        void Set(float _x, float _y);
        float GetAlternativeDistance(Pointf rh);
        float GetDistance(Pointf rh);

        float x;
        float y;
    };
}