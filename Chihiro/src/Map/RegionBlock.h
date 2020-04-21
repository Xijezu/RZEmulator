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
#include "SharedMutex.h"
#include "Region.h"

struct RegionBase
{
    RegionBase()
    {
        m_Regions = std::vector<Region *>(100 * 100, nullptr);
    }

    ~RegionBase()
    {
        for (auto &x : m_Regions)
        {
            delete x;
            x = nullptr;
        }
        m_Regions.clear();
    }

    // Deleting the copy & assignment operators
    // Better safe than sorry
    RegionBase(const RegionBase &) = delete;
    RegionBase &operator=(const RegionBase &) = delete;

    std::vector<Region *> m_Regions;
};

class RegionBlock
{
    public:
        RegionBlock()
        {
            m_RegionBases = std::vector<RegionBase *>(256, nullptr);
        }

        ~RegionBlock()
        {
            for (auto &x : m_RegionBases)
            {
                delete x;
                x = nullptr;
            }
        }

        // Deleting the copy & assignment operators
        // Better safe than sorry
        RegionBlock(const RegionBlock &) = delete;
        RegionBlock &operator=(const RegionBlock &) = delete;

        Region *getRegionPtr(uint32_t rx, uint32_t ry, uint8_t layer)
        {
            Region *res{nullptr};
            {
                NG_UNIQUE_GUARD writeGuard(i_lock);
                RegionBase      *rb = m_RegionBases[layer];
                if (rb != nullptr)
                    res = rb->m_Regions[ry + (100 * rx)];
            }
            return res;
        }

        Region *getRegion(uint32_t rx, uint32_t ry, uint8_t layer)
        {
            Region *res{nullptr};
            {
                NG_UNIQUE_GUARD writeGuard(i_lock);
                RegionBase      *rb = m_RegionBases[layer];
                if (rb == nullptr)
                {
                    rb = new RegionBase{ };
                    m_RegionBases[layer] = rb;
                }
                res = rb->m_Regions[rx + (100 * ry)];
                if (res == nullptr)
                {
                    res = new Region{ };
                    res->x     = rx;
                    res->y     = ry;
                    res->layer = layer;
                    rb->m_Regions[rx + (100 * ry)] = res;
                }
            }
            return res;
        }

    private:
        NG_SHARED_MUTEX           i_lock;
        std::vector<RegionBase *> m_RegionBases;
};