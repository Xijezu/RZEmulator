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
#include "SharedMutex.h"
#include "RegionBlock.h"

#define NG_REGION_FUNCTOR(fn) [&fn](RegionType& pRegionType) { (fn).Run(pRegionType); }

enum class RegionVisitor : uint8_t
{
        ClientVisitor  = 0x01,
        MovableVisitor = 0x02,
        StaticVisitor  = 0x04
};

class RegionContainer
{
    public:
        static RegionContainer &Instance()
        {
            static RegionContainer instance;
            return instance;
        }
        ~RegionContainer();
        // Deleting the copy & assignment operators
        // Better safe than sorry
        RegionContainer(const RegionContainer &) = delete;
        RegionContainer &operator=(const RegionContainer &) = delete;

        void InitRegion(float map_width, float map_height);
        bool IsValidRegion(uint rx, uint ry, uint8 layer);
        Region *GetRegion(WorldObject *pObject);
        Region *GetRegion(uint rx, uint ry, uint8 layer);
        /* Deprecated*/
        void DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, RegionFunctor &fn);
        void DoEachVisibleRegion(uint rx1, uint ry1, uint rx2, uint ry2, uint8_t layer, RegionFunctor &fn);
        void DoEachNewRegion(uint rx, uint ry, uint prx, uint pry, uint8_t layer, RegionFunctor &fn);

        template<typename Visitor>
        void DoEachVisibleRegion(uint rx, uint ry, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
        {
            uint left = rx - 3;
            if (rx < 3)
                left = 0;

            uint top = ry - 3;
            if (ry < 3)
                top = 0;

            uint right = rx + 3;
            if (right >= m_nRegionWidth)
                right = m_nRegionWidth - 1;

            uint bottom = ry + 3;
            if (bottom >= m_nRegionHeight)
                bottom = m_nRegionHeight - 1;
            for (uint x = left; x <= right; ++x)
            {
                for (uint y = top; y < bottom; ++y)
                {
                    if (IsVisibleRegion(rx, ry, x, y) != 0)
                    {
                        Region *region = getRegionPtr(x, y, layer);
                        if (region != nullptr)
                        {
                            if ((nBitset & (uint8_t)RegionVisitor::ClientVisitor) != 0)
                                region->DoEachClient2(visitor);
                            if ((nBitset & (uint8_t)RegionVisitor::MovableVisitor) != 0)
                                region->DoEachMovableObject2(visitor);
                            if ((nBitset & (uint8_t)RegionVisitor::StaticVisitor) != 0)
                                region->DoEachStaticObject2(visitor);
                        }
                    }
                }
            }
        }

        template<typename Visitor>
        void DoEachVisibleRegion(uint rx1, uint ry1, uint rx2, uint ry2, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
        {
            uint left = rx2 - 3;
            if ((rx2 - 3) >= (rx1 - 3))
                left = rx1 - 3;
            if (left <= 0)
                left = 0;

            uint top = ry2 - 3;
            if ((ry2 - 3) >= (ry1 - 3))
                top = ry1 - 3;
            if (top <= 0)
                top = 0;

            uint right = rx2 + 3;
            if ((rx1 + 3) >= (rx2 + 3))
                right = rx1 + 3;
            if (right >= m_nRegionWidth)
                right = m_nRegionWidth - 1;

            uint bottom = ry2 + 3;
            if ((ry1 + 3) >= (ry2 + 3))
                bottom = ry1 + 3;
            if (bottom >= m_nRegionHeight)
                bottom = m_nRegionHeight - 1;

            for (uint x = left; x <= right; ++x)
            {
                for (uint y = top; y < bottom; ++y)
                {
                    if (IsVisibleRegion(rx1, ry1, x, y) != 0 || IsVisibleRegion(rx2, ry2, x, y) != 0)
                    {
                        Region *region = getRegionPtr(x, y, layer);
                        if (region != nullptr)
                        {
                            if ((nBitset & (uint8_t)RegionVisitor::ClientVisitor) != 0)
                                region->DoEachClient2(visitor);
                            if ((nBitset & (uint8_t)RegionVisitor::MovableVisitor) != 0)
                                region->DoEachMovableObject2(visitor);
                            if ((nBitset & (uint8_t)RegionVisitor::StaticVisitor) != 0)
                                region->DoEachStaticObject2(visitor);
                        }
                    }
                }
            }
        }

        template<typename Visitor>
        void DoEachNewRegion(uint rx, uint ry, uint prx, uint pry, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
        {
            uint left = rx - 3;
            if (rx < 3)
                left = 0;

            uint top = ry - 3;
            if (ry < 3)
                top = 0;

            uint right = rx + 3;
            if (right >= m_nRegionWidth)
                right = m_nRegionWidth - 1;

            uint bottom = ry + 3;
            if (bottom >= m_nRegionHeight)
                bottom = m_nRegionHeight - 1;

            for (uint x = left; x <= right; ++x)
            {
                for (uint y = top; y < bottom; ++y)
                {
                    if (IsVisibleRegion(rx, ry, x, y) != 0)
                    {
                        if (IsVisibleRegion(prx, pry, x, y) == 0)
                        {
                            Region *region = getRegionPtr(x, y, layer);
                            if (region != nullptr)
                            {
                                if ((nBitset & (uint8_t)RegionVisitor::ClientVisitor) != 0)
                                    region->DoEachClient2(visitor);
                                if ((nBitset & (uint8_t)RegionVisitor::MovableVisitor) != 0)
                                    region->DoEachMovableObject2(visitor);
                                if ((nBitset & (uint8_t)RegionVisitor::StaticVisitor) != 0)
                                    region->DoEachStaticObject2(visitor);
                            }
                        }
                    }
                }
            }
        }

        uint IsVisibleRegion(uint rx, uint ry, uint _rx, uint _ry);
        uint IsVisibleRegion(WorldObject *obj1, WorldObject *obj2);
    private:
        void initRegion();
        void deinitRegion();
        RegionBlock *getRegionBlockPtr(uint rcx, uint rcy);
        RegionBlock *getRegionBlock(uint rcx, uint rcy);
        Region *getRegionPtr(uint rx, uint ry, uint8 layer);
        Region *getRegion(uint rx, uint ry, uint8 layer);

        float                      m_MapWidth;
        float                      m_MapHeight;
        uint                       m_nRegionWidth;
        uint                       m_nRegionHeight;
        uint                       m_nRegionBlockWidth;
        uint                       m_nRegionBlockHeight;
        std::vector<RegionBlock *> m_RegionBlock;
        NG_SHARED_MUTEX i_lock;
    protected:
        RegionContainer() = default;
};
#define sRegion RegionContainer::Instance()