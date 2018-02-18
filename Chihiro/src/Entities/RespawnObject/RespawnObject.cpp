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

#include "RespawnObject.h"
#include "World.h"
#include "ObjectMgr.h"
#include "MemPool.h"

RespawnObject::RespawnObject(MonsterRespawnInfo rh) : info(RespawnInfo(rh))
{
    m_nMaxRespawnNum = info.prespawn_count;
    lastDeadTime = 0;
}

void RespawnObject::Update(uint diff)
{
    /// No need to update anything
    if(info.count >= m_nMaxRespawnNum)
        return;

    uint ct = sWorld->GetArTime();

    /// Only update based on spawn rates (each X seconds after dead)
    if(lastDeadTime != 0 && lastDeadTime + info.interval > ct)
        return;

    int respawn_count = m_nMaxRespawnNum - info.count;

    if(lastDeadTime == 0) {
        lastDeadTime = ct;
        respawn_count = m_nMaxRespawnNum;
    }

    /// Do we need a respawn?
    if(respawn_count > 0) {
        while (true) {
            /// Generate random respawn coordinates based on a rectangle
            int x = irand((int)info.left, (int)info.right);
            int y = irand((int)info.top, (int)info.bottom);

            /// Generate monster if not blocked
            Monster* monster{nullptr};
            if (!sObjectMgr->IsBlocked(x, y)) {
                monster = sObjectMgr->RespawnMonster(x, y, info.layer, info.monster_id, info.is_wandering, info.way_point_id, this, true);
            }
            /// Put it to the list when it's not blocked
            if (monster != nullptr) {
                if (info.dungeon_id != 0) {
                    //monster.m_nDungeonId = info.dungeon_id;
                }
                m_vRespawnedMonster.emplace_back(monster->GetHandle());
                info.count++;
            }
            /// And finally break if we reached our respawn count
            if (info.count >= respawn_count)
                break;
        }
    }
}

void RespawnObject::onMonsterDelete(Monster *mob)
{
    if(mob == nullptr)
        return;

    auto pos = std::find(m_vRespawnedMonster.begin(), m_vRespawnedMonster.end(), mob->GetHandle());
    if(pos != m_vRespawnedMonster.end()) {
        m_vRespawnedMonster.erase(pos);
        mob->m_pDeleteHandler = nullptr;
        lastDeadTime = sWorld->GetArTime();
        info.count--;
    }
}
