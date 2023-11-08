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

#include "RegionContainer.h"

#include "World.h"

#define REGION_BLOCK_COUNT 100

void RegionContainer::InitRegion(float_t map_width, float_t map_height)
{
    m_MapWidth = map_width;
    m_MapHeight = map_height;
    m_nRegionWidth = (uint32_t)((map_width / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) + 1.0f);
    m_nRegionHeight = (uint32_t)((map_height / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) + 1.0f);
    m_nRegionBlockWidth = (m_nRegionWidth / REGION_BLOCK_COUNT) + 1;
    m_nRegionBlockHeight = (m_nRegionHeight / REGION_BLOCK_COUNT) + 1;
    initRegion();
}

void RegionContainer::initRegion()
{
    NG_UNIQUE_GUARD writeGuard(i_lock);
    {
        uint32_t count = m_nRegionBlockHeight * m_nRegionBlockWidth;
        m_RegionBlock = std::vector<RegionBlock *>(count, nullptr);
    }
}

bool RegionContainer::IsValidRegion(uint32_t rx, uint32_t ry, uint8_t /* layer*/)
{
    return rx < m_nRegionWidth && ry < m_nRegionHeight;
}

Region *RegionContainer::GetRegion(WorldObject *pObject)
{
    return GetRegion(
        (uint32_t)(pObject->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint32_t)(pObject->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), pObject->GetLayer());
}

Region *RegionContainer::GetRegion(uint32_t rx, uint32_t ry, uint8_t layer)
{
    Region *result{nullptr};
    if (IsValidRegion(rx, ry, layer))
        result = getRegion(rx, ry, layer);
    return result;
}

void RegionContainer::DoEachVisibleRegion(uint32_t rx, uint32_t ry, uint8_t layer, RegionFunctor &fn)
{
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
    uint32_t left;

    left = rx - 3;
    if (rx < 3)
        left = 0;

    top = ry - 3;
    if (ry < 3)
        top = 0;

    right = rx + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom = ry + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;
    for (uint32_t x = left; x <= right; ++x) {
        for (uint32_t y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx, ry, x, y) != 0) {
                Region *region = getRegionPtr(x, y, layer);
                if (region != nullptr)
                    fn.Run(region);
            }
        }
    }
}

void RegionContainer::DoEachVisibleRegion(uint32_t rx1, uint32_t ry1, uint32_t rx2, uint32_t ry2, uint8_t layer, RegionFunctor &fn)
{
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
    uint32_t left;

    //             left = rx1;
    //             if(rx2 < rx1)
    //                 left = rx2;
    //
    //             top = ry1;
    //             if(ry2 < ry1)
    //                 top = ry2;
    //
    //             right = rx1;
    //             if(rx2 > rx1)
    //                 right = rx2;
    //             if(right >= this.m_nRegionWidth)
    //                 right = this.m_nRegionWidth-1;
    //
    //             bottom = ry1;
    //             if(ry2 > ry1)
    //                 bottom = ry2;
    //             if(bottom >= this.m_nRegionHeight)
    //                 bottom = this.m_nRegionHeight-1;
    //
    //
    //             right = rx2 - 3;
    //             left = rx1 - 3;

    left = rx2 - 3;
    if ((rx2 - 3) >= (rx1 - 3))
        left = rx1 - 3;
    if (left <= 0)
        left = 0;

    top = ry2 - 3;
    if ((ry2 - 3) >= (ry1 - 3))
        top = ry1 - 3;
    if (top <= 0)
        top = 0;

    right = rx2 + 3;
    if ((rx1 + 3) >= (rx2 + 3))
        right = rx1 + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom = ry2 + 3;
    if ((ry1 + 3) >= (ry2 + 3))
        bottom = ry1 + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;

    for (uint32_t x = left; x <= right; ++x) {
        for (uint32_t y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx1, ry1, x, y) != 0 || IsVisibleRegion(rx2, ry2, x, y) != 0) {
                Region *region = getRegionPtr(x, y, layer);
                if (region != nullptr)
                    fn.Run(region);
            }
        }
    }
}

void RegionContainer::DoEachNewRegion(uint32_t rx, uint32_t ry, uint32_t prx, uint32_t pry, uint8_t layer, RegionFunctor &fn)
{
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
    uint32_t left;

    left = rx - 3;
    if (rx < 3)
        left = 0;

    top = ry - 3;
    if (ry < 3)
        top = 0;

    right = rx + 3;
    if (right >= m_nRegionWidth)
        right = m_nRegionWidth - 1;

    bottom = ry + 3;
    if (bottom >= m_nRegionHeight)
        bottom = m_nRegionHeight - 1;

    for (uint32_t x = left; x <= right; ++x) {
        for (uint32_t y = top; y < bottom; ++y) {
            if (IsVisibleRegion(rx, ry, x, y) != 0) {
                if (IsVisibleRegion(prx, pry, x, y) == 0) {
                    Region *region = getRegionPtr(x, y, layer);
                    if (region != nullptr)
                        fn.Run(region);
                }
            }
        }
    }
}

uint32_t RegionContainer::IsVisibleRegion(uint32_t rx, uint32_t ry, uint32_t _rx, uint32_t _ry)
{
    int32_t nx = VISIBLE_REGION_RANGE + _rx - rx;
    int32_t ny = VISIBLE_REGION_RANGE + _ry - ry;

    if (nx < 0 || nx >= VISIBLE_REGION_BOX_WIDTH)
        return false;
    if (ny < 0 || ny >= VISIBLE_REGION_BOX_WIDTH)
        return false;

    return s_Matrix[nx][ny];
}

uint32_t RegionContainer::IsVisibleRegion(WorldObject *obj1, WorldObject *obj2)
{
    if (obj1 == nullptr || obj2 == nullptr)
        return 0;

    return IsVisibleRegion((uint32_t)(obj1->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint32_t)(obj1->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
        (uint32_t)(obj2->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint32_t)(obj2->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)));
};

RegionBlock *RegionContainer::getRegionBlockPtr(uint32_t rcx, uint32_t rcy)
{
    RegionBlock *res{nullptr};
    {
        NG_SHARED_GUARD readGuard(i_lock);
        res = m_RegionBlock[rcx + rcy * m_nRegionBlockWidth];
    }
    return res;
}

RegionBlock *RegionContainer::getRegionBlock(uint32_t rcx, uint32_t rcy)
{
    RegionBlock *res{nullptr};
    {
        NG_UNIQUE_GUARD writeGuard(i_lock);
        res = m_RegionBlock[rcx + rcy * m_nRegionBlockWidth];
        if (res == nullptr) {
            res = new RegionBlock{};
            m_RegionBlock[rcx + rcy * m_nRegionBlockWidth] = res;
        }
    }
    return res;
}

Region *RegionContainer::getRegionPtr(uint32_t rx, uint32_t ry, uint8_t layer)
{
    RegionBlock *b = getRegionBlockPtr(rx / REGION_BLOCK_COUNT, ry / REGION_BLOCK_COUNT);
    if (b != nullptr)
        return b->getRegion(rx % REGION_BLOCK_COUNT, ry % REGION_BLOCK_COUNT, layer);
    return nullptr;
}

Region *RegionContainer::getRegion(uint32_t rx, uint32_t ry, uint8_t layer)
{
    RegionBlock *b = getRegionBlock(rx / REGION_BLOCK_COUNT, ry / REGION_BLOCK_COUNT);
    if (b != nullptr)
        return b->getRegion(rx % REGION_BLOCK_COUNT, ry % REGION_BLOCK_COUNT, layer);
    return nullptr;
}

RegionContainer::~RegionContainer()
{
    deinitRegion();
}

void RegionContainer::deinitRegion()
{
    NG_UNIQUE_GUARD writeLock(i_lock);
    for (auto &x : m_RegionBlock) {
        delete x;
        x = nullptr;
    }
}
