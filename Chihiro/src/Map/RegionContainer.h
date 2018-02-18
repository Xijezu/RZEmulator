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

#ifndef NGEMITY_REGIONCONTAINER_H
#define NGEMITY_REGIONCONTAINER_H

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
        NG_SHARED_MUTEX i_lock;
};
#define sRegion ACE_Singleton<RegionContainer, ACE_Thread_Mutex>::instance()
#endif // NGEMITY_REGIONCONTAINER_H
