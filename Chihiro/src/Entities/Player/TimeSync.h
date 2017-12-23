#ifndef PROJECT_TIMESYNCHER_H
#define PROJECT_TIMESYNCHER_H

#include "Common.h"

class TimeSynch{
public:
    TimeSynch(int L, int DC, int pMAX) : m_L(L), m_DC(DC), m_MAX(pMAX) { };
    ~TimeSynch() = default;

    void onEcho(uint t)
    {
        if(m_vT.size() == m_MAX) {
            //                 m_vT.
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     &thisa,
//                     *(v2 + 1),
//                     v2);
//                 v3 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&thisa);
//                 LODWORD(v3->end.x) = T;
        } else {
            m_vT.emplace_back(t);
        }
    }

    uint GetInterval()
    {
        uint tc = 0;
        int CDC = 0;
        uint size = (uint)m_vT.size();

        for(auto& i : m_vT)
        {
            if (i < m_L || CDC >= m_DC)
            {
                ++size;
                tc += i;
            }
            CDC++;
        }
        return tc / size >> 1;
    }

    uint GetTestCount()
    {
        return 0;
    }

    std::vector<uint> m_vT{};
private:
    int m_L{};
    int m_DC{};
    int m_MAX{};
};

#endif // PROJECT_TIMESYNCHER_H
