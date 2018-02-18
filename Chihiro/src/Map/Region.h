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

#ifndef NGEMITY_REGION_H
#define NGEMITY_REGION_H

#include "Common.h"
#include "SharedMutex.h"
#include "Functors.h"

class WorldObject;
class Region
{
    public:
        friend class RegionBlock;
        Region() = default;
        ~Region() = default;

        void AddObject(WorldObject* obj);
        void RemoveObject(WorldObject* obj);
        uint DoEachClient(WorldObjectFunctor& fn);
        uint DoEachStaticObject(WorldObjectFunctor& fn);
        uint DoEachMovableObject(WorldObjectFunctor& fn);

    private:
        typedef std::vector<WorldObject*> RegionType;
        void addObject(WorldObject* obj, RegionType* v);
        void removeObject(WorldObject* obj, RegionType* v);


        RegionType m_vStaticObjects;
        RegionType m_vMovableObjects;
        RegionType m_vClientObjects;

        NG_SHARED_MUTEX i_lock;
        uint x;
        uint y;
        uint8 layer;
};

#endif // NGEMITY_REGION_H
