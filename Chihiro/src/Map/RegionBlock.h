#ifndef PROJECT_REGIONBLOCK_H
#define PROJECT_REGIONBLOCK_H

#include "Common.h"
#include "SharedMutex.h"
#include "Region.h"

struct RegionBase
{
    RegionBase()
    {
        m_Regions = std::vector<Region*>(100 * 100, nullptr);
    }

    ~RegionBase()
    {
        for(auto &x : m_Regions)
        {
            delete x;
            x = nullptr;
        }
        m_Regions.clear();
    }

    std::vector<Region*> m_Regions;
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
            for(auto &x : m_RegionBases)
            {
                delete x;
                x = nullptr;
            }
        }

        Region *getRegionPtr(uint rx, uint ry, uint8 layer)
        {
            Region* res{nullptr};
            {
                MX_UNIQUE_GUARD writeGuard(i_lock);
                RegionBase* rb = m_RegionBases[layer];
                if(rb != nullptr)
                    res = rb->m_Regions[ry + (100 * rx)];
            }
            return res;
        }

        Region *getRegion(uint rx, uint ry, uint8 layer)
        {
            Region *res{nullptr};
            {
                MX_UNIQUE_GUARD writeGuard(i_lock);
                RegionBase *rb = m_RegionBases[layer];
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
        MX_SHARED_MUTEX           i_lock;
        std::vector<RegionBase *> m_RegionBases;
};
#endif // PROJECT_REGIONBLOCK_H
