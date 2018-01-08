/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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
    uint ct = sWorld->GetArTime();
    uint try_cnt = ct;
    int nCountToDelete = 0;
    Monster* monster{nullptr};
    if(info.count < m_nMaxRespawnNum) {
        if(lastDeadTime != 0 && lastDeadTime + info.interval > ct)
            return;
        int respawn_count = info.inc;
        if(info.inc >= m_nMaxRespawnNum - info.count)
            respawn_count = m_nMaxRespawnNum - info.count;

        bool bFirstRegen = false;
        if(lastDeadTime == 0) {
            lastDeadTime = ct;
            respawn_count = m_nMaxRespawnNum;
            bFirstRegen = true;
        }
        if(respawn_count > 0) {
            try_cnt = 0;
            nCountToDelete = 0;
            while(true) {
                int x = irand((int)info.left, (int)info.right);
                int y = irand((int)info.top, (int)info.bottom);
                if(bFirstRegen) {
                    if(nCountToDelete == 0)
                    {
                        ++try_cnt;
                        if(try_cnt > 0x1F4)
                            break;
                    }
                }
                monster = nullptr;
                /// TODO: IsBlocked
                if(true) {
                    monster = sObjectMgr->RespawnMonster(x, y, info.layer, info.monster_id, info.is_wandering, info.way_point_id, this, true);
                }
                if(monster != nullptr) {
                    if(info.dungeon_id != 0) {
                        //monster.m_nDungeonId = info.dungeon_id;
                    }
                     m_vRespawnedMonster.emplace_back(monster->GetHandle());
                    info.count++;
                    nCountToDelete++;
                    if(nCountToDelete >= respawn_count)
                        break;
                }
                break;
            }
            return;
        }
        if(info.count != m_nMaxRespawnNum)
            return;
    }
    if(info.dungeon_id != 0 || info.way_point_id != 0)
        return;
    if(m_nMaxRespawnNum == info.prespawn_count)
        return;
    if(m_nMaxRespawnNum < info.prespawn_count) {
        m_nMaxRespawnNum = info.prespawn_count;
        sWorld->m_vRespawnList.emplace_back(this);
        return;
    }
    if(lastDeadTime == 0 || info.interval + lastDeadTime + 18000 > ct)
        return;

    nCountToDelete = (int)(m_nMaxRespawnNum - info.prespawn_count);
    if(m_nMaxRespawnNum == info.prespawn_count) {
        return;
    }
    while(true) {
        uint h = 0;
        if(!m_vRespawnedMonster.empty())
            h = m_vRespawnedMonster.back();

        //auto mob = dynamic_cast<Monster*>(sMemoryPool->getPtrFromId(h));
        auto mob = sMemoryPool->GetObjectInWorld<Monster>(h);
        if(mob == nullptr || mob->GetSubType() != ST_Mob)
            mob = nullptr;
        if(mob == nullptr || mob->m_pDeleteHandler != this) {
            m_nMaxRespawnNum--;
            info.count--;
            nCountToDelete--;
            m_vRespawnedMonster.erase(std::remove(m_vRespawnedMonster.begin(), m_vRespawnedMonster.end(), h), m_vRespawnedMonster.end());
        } else {
            m_nMaxRespawnNum--;
            info.count--;
            nCountToDelete--;
            m_vRespawnedMonster.erase(std::remove(m_vRespawnedMonster.begin(), m_vRespawnedMonster.end(), h), m_vRespawnedMonster.end());

            mob->m_pDeleteHandler = nullptr;
        }
        return;
    }
}

void RespawnObject::onMonsterDelete(Monster *mob)
{
    if(mob == nullptr)
        return;
    bool bNeedToRespawn{false};

    if(std::find(m_vRespawnedMonster.begin(), m_vRespawnedMonster.end(), mob->GetHandle()) != m_vRespawnedMonster.end()) {
        m_vRespawnedMonster.erase(std::remove(m_vRespawnedMonster.begin(), m_vRespawnedMonster.end(), mob->GetHandle()), m_vRespawnedMonster.end());
        //m_ndelete mob;

        lastDeadTime = sWorld->GetArTime();
        bNeedToRespawn = info.count-- <= m_nMaxRespawnNum;

        if(m_nMaxRespawnNum < info.max_num)
            m_nMaxRespawnNum++;
        if(bNeedToRespawn)
            sWorld->m_vRespawnList.emplace_back(this);
    }
}
