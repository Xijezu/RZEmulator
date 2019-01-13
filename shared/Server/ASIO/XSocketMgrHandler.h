#pragma once
/*
 * Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "Common.h"

template <typename T>
class XSocketMgr;

template <typename T>
class XSocketMgrHandler
{
  public:
    static XSocketMgrHandler &Instance()
    {
        static XSocketMgrHandler instance;
        return instance;
    }

    uint32_t Insert(XSocketMgr<T> *pMgr)
    {
        sm_SocketMgrIdx++;
        sm_vSocketMgr[sm_SocketMgrIdx] = pMgr;
        return sm_SocketMgrIdx;
    }

    void Remove(uint32_t nIdx)
    {
        sm_vSocketMgr.erase(nIdx);
    }

    XSocketMgr<T> *GetManager(uint32_t nIdx)
    {
        if (sm_vSocketMgr.count(nIdx) == 0)
            return nullptr;
        return sm_vSocketMgr[nIdx];
    }

  private:
    std::atomic_uint32_t sm_SocketMgrIdx{};
    std::unordered_map<uint32_t, XSocketMgr<T> *> sm_vSocketMgr{};
};