#ifndef __AR_REGION_H
#define __AR_REGION_H

#include "Common.h"

class WorldObject;
class Player;
class Unit;
class NPC;

#define REGION_BLOCK_COUNT  100
#define AR_REGION_SIZE      180

typedef UNORDERED_MAP<uint32, WorldObject *> svObjects;

// The actual ArRegion, everything else is just a container class
//////////////////////////////////////////////////////////////////////////
// DO NOT USE OUTSIDE OF ArRegionContainer!!!
//////////////////////////////////////////////////////////////////////////
class ArRegion {
public:
    ArRegion() = default;
    ~ArRegion() = default;

    void AddObject(WorldObject *obj);
    void RemoveObject(WorldObject *obj);

    uint DoEachClient(const std::function<void (Unit*)>& fn);
    void DoEachMovableObject(const std::function<void (WorldObject*)>& fn);
    void DoEachStaticObject(const std::function<void (WorldObject*)>& fn);
protected:
    svObjects m_vStatic;
    svObjects m_vMovable;
    svObjects m_vClient;
};

//////////////////////////////////////////////////////////////////////////
// DO NOT USE OUTSIDE OF ArRegionContainer!!!
//////////////////////////////////////////////////////////////////////////
class ArRegionBase {
public:
    ArRegionBase() = default;
    ~ArRegionBase() = default;

    ArRegion* GetRegion(uint idx);
    UNORDERED_MAP<uint32, ArRegion*> m_nRegions;
};

//////////////////////////////////////////////////////////////////////////
// DO NOT USE OUTSIDE OF ArRegionContainer!!!
//////////////////////////////////////////////////////////////////////////
class ArRegionBlock {
public:
    ArRegionBlock() = default;
    ~ArRegionBlock() = default;

    ArRegion *getRegion(uint32 rx, uint32 ry, uint32 layer);
private:
    UNORDERED_MAP<uint32, ArRegionBase*> m_RegionBases;
};

//////////////////////////////////////////////////////////////////////////
class ArRegionContainer {

public :
    ArRegionContainer() = default;
    ~ArRegionContainer() = default;

    void InitRegionSystem(uint32 width, uint32 height);
    bool IsValidRegion(uint32 rx, uint32 ry, uint32 layer);
    uint32 IsVisibleRegion(uint32 rx, uint32 ry, uint32 _rx, uint32 _ry);
    ArRegion *GetRegion(WorldObject *pObject);
    ArRegion *GetRegion(uint32 rx, uint32 ry, uint32 layer);
    void DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, std::function<void (ArRegion*)> fn);
    void DoEachVisibleRegion(uint rx1, uint ry1, uint rx2, uint ry2, uint8_t layer, std::function<void (ArRegion*)> fn);
    void DoEachNewRegion(uint rx, uint ry, uint prx, uint pry, uint8_t layer, std::function<void (ArRegion*)> fn);

    // needed for IsvisibleRegion
    uint32 s_Matrix[49] = {
            0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 1, 1, 0, 0,
            0, 1, 1, 1, 1, 1, 0,
            0, 1, 1, 1, 1, 1, 0,
            0, 1, 1, 1, 1, 1, 0,
            0, 0, 1, 1, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0
    };
private:
    UNORDERED_MAP<uint32, ArRegionBlock> m_RegionBlock;

    uint32 m_nMapWidth{};
    uint32 m_nMapHeight{};
    uint32 m_nRegionWidth{};
    uint32 m_nRegionHeight{};
    uint32 m_nRegionBlockWidth{};
    uint32 m_nRegionBlockHeight{};
};

#define sArRegion ACE_Singleton<ArRegionContainer, ACE_Null_Mutex>::instance()
#endif // __AR_REGION_H