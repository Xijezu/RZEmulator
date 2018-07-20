#pragma once
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
#include "Common.h"
#include "Pointf.h"
#include "Linef.h"

namespace X2D
{
    class Rectf
    {
        public:
            Rectf() = default;
            ~Rectf() = default;
            virtual bool IsInclude(float x, float y);
            virtual bool IsInclude(Pointf p);
            virtual bool Set(Pointf begin, Pointf end);

            X2D::Pointf pos{ }, size{ };
    };

    class RectangleF
    {
        public:
            RectangleF() = default;
            ~RectangleF() = default;
            RectangleF(Pointf p1, Pointf p2);
            explicit RectangleF(std::vector<Pointf> points);
            virtual bool IsInclude(float x, float y);
            virtual bool IsInclude(Pointf p);
            bool IsCollision(Linef line);

            Pointf m_TopLeft;
            Pointf m_BottomRight;
    };
}