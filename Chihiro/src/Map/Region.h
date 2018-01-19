#ifndef PROJECT_REGION_H
#define PROJECT_REGION_H

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

        MX_SHARED_MUTEX i_lock;
        uint x;
        uint y;
        uint8 layer;
};

#endif // PROJECT_REGION_H
