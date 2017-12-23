#ifndef __AR_REGION_H
#define __AR_REGION_H

#include "Common.h"
#include "Object.h"
#include "Player.h"
#include "Unit.h"
#include "NPC.h"

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

    void AddObject(WorldObject *obj)
    {
        if (obj->GetObjType() == OBJ_MOVABLE) {
            m_vMovable[obj->GetHandle()] = obj;
        } else if (obj->GetObjType() == OBJ_CLIENT)
            m_vClient[obj->GetHandle()] = obj;
        else if (obj->GetObjType() == OBJ_STATIC)
            m_vStatic[obj->GetHandle()] = obj;
        obj->_region = this;
        //obj->_bIsInWorld = true;
    }

    void RemoveObject(WorldObject *obj)
    {
        if (obj->GetObjType() == OBJ_MOVABLE)
            m_vMovable.erase(obj->GetHandle());
        else if (obj->GetObjType() == OBJ_CLIENT)
            m_vClient.erase(obj->GetHandle());
        else if (obj->GetObjType() == OBJ_STATIC)
            m_vStatic.erase(obj->GetHandle());
        obj->_region = nullptr;
    }

    uint DoEachClient(const std::function<void (Unit*)>& fn) {
        for(auto& obj : this->m_vClient) {
            if(obj.second != nullptr)
                fn(dynamic_cast<Unit*>(obj.second));
                //fn.run(dynamic_cast<Unit*>(obj.second));
        }
    }

    void DoEachMovableObject(const std::function<void (WorldObject*)>& fn)
    {
        for (auto &obj : m_vMovable) {
            if (obj.second != nullptr)
                fn(obj.second);
        }
    }

    void DoEachStaticObject(const std::function<void (WorldObject*)>& fn)
    {
        for (auto &obj : m_vStatic) {
            if (obj.second != nullptr)
                fn(obj.second);
        }
    }
private:
    uint32 _x;
    uint32 _y;
    uint32 _layer;

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
    ArRegionBase()
    {
// 		for (int i = 0; i < (REGION_BLOCK_COUNT * REGION_BLOCK_COUNT); i++)
// 		{
// 			m_nRegions[i] = new ArRegion();
// 		}
    }

    ~ArRegionBase()
    { /*if (m_Regions) delete[] m_Regions*/; }

    UNORDERED_MAP<uint32, ArRegion> m_nRegions;
};

//////////////////////////////////////////////////////////////////////////
// DO NOT USE OUTSIDE OF ArRegionContainer!!!
//////////////////////////////////////////////////////////////////////////
class ArRegionBlock {
public:
    ArRegionBlock() = default;
//    {
// 		for (int i = 0; i < 256; i++)
// 		{
// 			m_RegionBases[i] = new ArRegionBase();
// 		}
//    }

    ~ArRegionBlock() = default;

    ArRegion *getRegion(uint32 rx, uint32 ry, uint32 layer)
    {
        if (layer > 256)
            return nullptr;

        return &m_RegionBases[layer].m_nRegions[rx + (REGION_BLOCK_COUNT * ry)];

    }

private:
    UNORDERED_MAP<uint32, ArRegionBase> m_RegionBases;
};

//////////////////////////////////////////////////////////////////////////
class ArRegionContainer {

public :
    ArRegionContainer() = default;
    ~ArRegionContainer() = default;

    void InitRegionSystem(uint32 width, uint32 height);
    bool IsValidRegion(uint32 rx, uint32 ry, uint32 layer);
    uint32 IsVisibleRegion(uint32 rx, uint32 ry, uint32 _rx, uint32 _ry);
    ArRegion *GetRegion(WorldObject &pObject);
    ArRegion *GetRegion(uint32 rx, uint32 ry, uint32 layer);
    void DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, std::function<void (ArRegion*)> fn);
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