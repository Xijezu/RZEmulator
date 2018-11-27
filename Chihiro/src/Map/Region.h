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
#include "SharedMutex.h"
#include "Functors.h"

class WorldObject;
using RegionType = std::vector<WorldObject *>;

class Region
{
    public:
        friend class RegionBlock;
        Region() = default;
        ~Region() = default;
        // Deleting the copy & assignment operators
        // Better safe than sorry
        Region(const Region &) = delete;
        Region &operator=(const Region &) = delete;

        void AddObject(WorldObject *obj);
        void RemoveObject(WorldObject *obj);
        /* Deprecated*/
        uint DoEachClient(WorldObjectFunctor &fn);
        uint DoEachStaticObject(WorldObjectFunctor &fn);
        uint DoEachMovableObject(WorldObjectFunctor &fn);

        template<typename Visitor>
        void DoEachClient2(Visitor &&visitor)
        {
            {
                NG_UNIQUE_GUARD lock(i_lock);
                visitor(m_vClientObjects);
            }
        }

        template<typename Visitor>
        void DoEachStaticObject2(Visitor &&visitor)
        {
            {
                NG_UNIQUE_GUARD lock(i_lock);
                visitor(m_vStaticObjects);
            }
        }

        template<typename Visitor>
        void DoEachMovableObject2(Visitor &&visitor)
        {
            {
                NG_UNIQUE_GUARD lock(i_lock);
                visitor(m_vMovableObjects);
            }
        }

    private:
        void addObject(WorldObject *obj, RegionType *v);
        void removeObject(WorldObject *obj, RegionType *v);

        RegionType m_vStaticObjects;
        RegionType m_vMovableObjects;
        RegionType m_vClientObjects;

        NG_SHARED_MUTEX i_lock;
        uint       x;
        uint       y;
        uint8      layer;
};