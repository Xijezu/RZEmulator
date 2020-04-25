#pragma once
/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include "RegionBlock.h"
#include "SharedMutex.h"

constexpr int32_t VISIBLE_REGION_RANGE = 3;
constexpr int VISIBLE_REGION_BOX_WIDTH = (VISIBLE_REGION_RANGE * 2 + 1);
constexpr int s_Matrix[VISIBLE_REGION_BOX_WIDTH][VISIBLE_REGION_BOX_WIDTH] =
    {
        {0, 0, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 0},
        {1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 0, 0},
};

#define NG_REGION_FUNCTOR(fn) [&fn](RegionType &pRegionType) { (fn).Run(pRegionType); }

enum class RegionVisitor : uint8_t
{
    ClientVisitor = 0x01,
    MovableVisitor = 0x02,
    StaticVisitor = 0x04
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
    bool IsValidRegion(uint32_t rx, uint32_t ry, uint8_t layer);
    Region *GetRegion(WorldObject *pObject);
    Region *GetRegion(uint32_t rx, uint32_t ry, uint8_t layer);
    /* Deprecated*/
    void DoEachVisibleRegion(uint32_t rx, uint32_t ry, uint8_t layer, RegionFunctor &fn);
    void DoEachVisibleRegion(uint32_t rx1, uint32_t ry1, uint32_t rx2, uint32_t ry2, uint8_t layer, RegionFunctor &fn);
    void DoEachNewRegion(uint32_t rx, uint32_t ry, uint32_t prx, uint32_t pry, uint8_t layer, RegionFunctor &fn);

    template <typename Visitor>
    void DoEachVisibleRegion(uint32_t rx, uint32_t ry, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
    {
        int32_t tx = rx + VISIBLE_REGION_RANGE;
        int32_t ty = ry + VISIBLE_REGION_RANGE;
        int32_t fx = std::max((int32_t)rx - VISIBLE_REGION_RANGE, 0);
        int32_t fy = std::max((int32_t)ry - VISIBLE_REGION_RANGE, 0);

        for (int32_t x = fx; x <= tx; x++)
        {
            for (int32_t y = fy; y <= ty; y++)
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

    template <typename Visitor>
    void DoEachVisibleRegion(uint32_t rx1, uint32_t ry1, uint32_t rx2, uint32_t ry2, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
    {
        int32_t left = std::max(0, std::min((int32_t)rx1 - VISIBLE_REGION_RANGE, (int32_t)rx2 - VISIBLE_REGION_RANGE));
        int32_t top = std::max(0, std::min((int32_t)ry1 - VISIBLE_REGION_RANGE, (int32_t)ry2 - VISIBLE_REGION_RANGE));

        int32_t right = std::min(std::max((int32_t)rx1 + VISIBLE_REGION_RANGE, (int32_t)rx2 + VISIBLE_REGION_RANGE), (int32_t)m_nRegionWidth - 1);
        int32_t bottom = std::min(std::max((int32_t)ry1 + VISIBLE_REGION_RANGE, (int32_t)ry2 + VISIBLE_REGION_RANGE), (int32_t)m_nRegionHeight - 1);

        for (int32_t y = top; y <= bottom; y++)
        {
            for (int32_t x = left; x <= right; x++)
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

    template <typename Visitor>
    void DoEachNewRegion(uint32_t rx, uint32_t ry, uint32_t prx, uint32_t pry, uint8_t layer, Visitor &&visitor, uint8_t nBitset)
    {
        int32_t left = std::max(0, std::min((int32_t)rx - VISIBLE_REGION_RANGE, (int32_t)prx - VISIBLE_REGION_RANGE));
        int32_t top = std::max(0, std::min((int32_t)ry - VISIBLE_REGION_RANGE, (int32_t)pry - VISIBLE_REGION_RANGE));

        int32_t right = std::min(std::max((int32_t)rx + VISIBLE_REGION_RANGE, (int32_t)prx + VISIBLE_REGION_RANGE), (int32_t)m_nRegionWidth - 1);
        int32_t bottom = std::min(std::max((int32_t)ry + VISIBLE_REGION_RANGE, (int32_t)pry + VISIBLE_REGION_RANGE), (int32_t)m_nRegionHeight - 1);

        for (int32_t y = top; y <= bottom; y++)
        {
            for (int32_t x = left; x <= right; x++)
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

    uint32_t IsVisibleRegion(uint32_t rx, uint32_t ry, uint32_t _rx, uint32_t _ry);
    uint32_t IsVisibleRegion(WorldObject *obj1, WorldObject *obj2);

private:
    void initRegion();
    void deinitRegion();
    RegionBlock *getRegionBlockPtr(uint32_t rcx, uint32_t rcy);
    RegionBlock *getRegionBlock(uint32_t rcx, uint32_t rcy);
    Region *getRegionPtr(uint32_t rx, uint32_t ry, uint8_t layer);
    Region *getRegion(uint32_t rx, uint32_t ry, uint8_t layer);

    float m_MapWidth;
    float m_MapHeight;
    uint32_t m_nRegionWidth;
    uint32_t m_nRegionHeight;
    uint32_t m_nRegionBlockWidth;
    uint32_t m_nRegionBlockHeight;
    std::vector<RegionBlock *> m_RegionBlock;
    NG_SHARED_MUTEX i_lock;

protected:
    RegionContainer() = default;
};
#define sRegion RegionContainer::Instance()