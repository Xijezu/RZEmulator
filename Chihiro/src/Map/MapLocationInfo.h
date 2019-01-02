#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "PolygonF.h"

class MapLocationInfo : public X2D::PolygonF
{
    public:
        MapLocationInfo(X2D::Pointf p1, X2D::Pointf p2, int id, int _pri) : X2D::PolygonF(p1, p2)
        {
            location_id = id;
            priority    = _pri;
        }

        MapLocationInfo(std::vector<X2D::Pointf> points, int id, int _pri) : X2D::PolygonF(std::move(points))
        {
            location_id = id;
            priority    = _pri;
        }

        std::string ToString()
        {
            return std::to_string(location_id);
        }

        int location_id;
        int priority;
};