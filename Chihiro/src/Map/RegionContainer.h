#ifndef PROJECT_ARREGIONCONTAINER_H
#define PROJECT_ARREGIONCONTAINER_H

#include "Common.h"
#include "SharedMutex.h"
#include "RegionBlock.h"

class RegionContainer
{
    public:
        ~RegionContainer();
        void InitRegion(float map_width, float map_height);
        bool IsValidRegion(uint rx, uint ry, uint8 layer);
        Region* GetRegion(WorldObject* pObject);
        Region* GetRegion(uint rx, uint ry, uint8 layer);
        void DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, RegionFunctor& fn);
        void DoEachVisibleRegion(uint rx1, uint ry1, uint rx2, uint ry2, uint8_t layer, RegionFunctor& fn);
        void DoEachNewRegion(uint rx, uint ry, uint prx, uint pry, uint8_t layer, RegionFunctor& fn);
        uint IsVisibleRegion(uint rx, uint ry, uint _rx, uint _ry);
        uint IsVisibleRegion(WorldObject* obj1, WorldObject* obj2);
    private:
        void initRegion();
        void deinitRegion();
        RegionBlock* getRegionBlockPtr(uint rcx, uint rcy);
        RegionBlock* getRegionBlock(uint rcx, uint rcy);
        Region* getRegionPtr(uint rx, uint ry, uint8 layer);
        Region* getRegion(uint rx, uint ry, uint8 layer);

        float m_MapWidth;
        float m_MapHeight;
        uint m_nRegionWidth;
        uint m_nRegionHeight;
        uint m_nRegionBlockWidth;
        uint m_nRegionBlockHeight;
        std::vector<RegionBlock*> m_RegionBlock;
        MX_SHARED_MUTEX i_lock;
};
#define sRegion ACE_Singleton<RegionContainer, ACE_Thread_Mutex>::instance()
#endif // PROJECT_ARREGIONCONTAINER_H
