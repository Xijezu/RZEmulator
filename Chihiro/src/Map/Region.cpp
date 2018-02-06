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

#include "Region.h"
#include "Object.h"

void Region::AddObject(WorldObject *obj)
{
    RegionType* lbo{nullptr};
    switch(obj->GetObjType())
    {
        case OBJ_CLIENT:
            lbo = &m_vClientObjects;
            break;
        case OBJ_MOVABLE:
            lbo = &m_vMovableObjects;
            break;
        case OBJ_STATIC:
            lbo = &m_vStaticObjects;
            break;
        default:
            return;
    }
    addObject(obj, lbo);
}

void Region::RemoveObject(WorldObject *obj)
{
    RegionType* lbo{nullptr};
    switch(obj->GetObjType())
    {
        case OBJ_CLIENT:
            lbo = &m_vClientObjects;
            break;
        case OBJ_MOVABLE:
            lbo = &m_vMovableObjects;
            break;
        case OBJ_STATIC:
            lbo = &m_vStaticObjects;
            break;
        default:
            return;
    }
    removeObject(obj, lbo);
}

uint Region::DoEachClient(WorldObjectFunctor &fn)
{
    MX_UNIQUE_GUARD lock(i_lock);
    for(auto& obj : m_vClientObjects)
    {
        fn.Run(obj);
    }
    return (uint)m_vClientObjects.size();
}

uint Region::DoEachStaticObject(WorldObjectFunctor &fn)
{
    MX_UNIQUE_GUARD lock(i_lock);
    for(auto& obj : m_vStaticObjects)
    {
        fn.Run(obj);
    }
    return (uint)m_vStaticObjects.size();
}

uint Region::DoEachMovableObject(WorldObjectFunctor &fn)
{
    MX_UNIQUE_GUARD lock(i_lock);
    for(auto& obj : m_vMovableObjects)
    {
        fn.Run(obj);
    }
    return (uint)m_vMovableObjects.size();
}

void Region::addObject(WorldObject *obj, std::vector<WorldObject *> *v)
{
    MX_UNIQUE_GUARD writeGuard(i_lock);
    // The region has to be correct
    // No matter what. So we're ASSERTing
    // to just crash if it isn't.
    // If this ASSERT fails, fix your fricking code
    // Having invalid region pointers
    // is an absolute no go
    ASSERT(obj->pRegion == nullptr);
    v->emplace_back(obj);
    obj->AddToWorld();
    obj->pRegion = this;
    obj->region_index = (int)(v->size() - 1);
}

void Region::removeObject(WorldObject *obj, std::vector<WorldObject *> *v)
{
    MX_UNIQUE_GUARD writeGuard(i_lock);
    // The region has to be correct
    // No matter what. So we're ASSERTing
    // to just crash if it isn't.
    // If this ASSERT fails, fix your fricking code
    // Having invalid region pointers
    // is an absolute no go
    ASSERT(!v->empty());
    ASSERT((*v)[obj->region_index]->GetHandle() == obj->GetHandle());
    ASSERT(obj->pRegion == this);

    if(v->back()->GetHandle() != obj->GetHandle())
    {
        auto lbo = v->back();
        (*v)[obj->region_index] = lbo;
        lbo->region_index = obj->region_index;
    }
    v->erase(v->end() - 1);
    obj->region_index = -1;
    obj->pRegion = nullptr;
    obj->RemoveFromWorld();
}
