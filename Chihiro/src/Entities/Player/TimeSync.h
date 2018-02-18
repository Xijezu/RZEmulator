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

#ifndef NGEMITY_TIMESYNCHER_H
#define NGEMITY_TIMESYNCHER_H

#include "Common.h"

class TimeSynch
{
    public:
        TimeSynch(int L, int DC, int pMAX) : m_L(L), m_DC(DC), m_MAX(pMAX) {};
        ~TimeSynch() = default;

        void onEcho(uint t)
        {
            if (m_vT.size() == m_MAX)
            {
                //                 m_vT.
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     &thisa,
//                     *(v2 + 1),
//                     v2);
//                 v3 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&thisa);
//                 LODWORD(v3->end.x) = T;
            }
            else
            {
                m_vT.emplace_back(t);
            }
        }

        uint GetInterval()
        {
            uint tc   = 0;
            int  CDC  = 0;
            auto size = (uint)m_vT.size();

            for (auto &i : m_vT)
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

        std::vector<uint> m_vT{ };
    private:
        int m_L{ };
        int m_DC{ };
        int m_MAX{ };
};

#endif // NGEMITY_TIMESYNCHER_H
