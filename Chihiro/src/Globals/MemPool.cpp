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

#include "MemPool.h"
#include "World.h"
#include "ObjectMgr.h"
#include "FieldPropManager.h"
#include "ItemCollector.h"

template class HashMapHolder<Player>;
template class HashMapHolder<WorldObject>;
template class HashMapHolder<Summon>;
template class HashMapHolder<Monster>;
template class HashMapHolder<Item>;

Item *MemoryPoolMgr::AllocItem()
{
    auto *p = new Item{ };
    p->m_nHandle = m_nItemTop++;
    p->SetUInt32Value(UNIT_FIELD_HANDLE, p->m_nHandle);
    //m_hsItem.insert(std::pair<uint, Item *>(p->m_nHandle, p));
    AddObject(p);
    return p;
}

void MemoryPoolMgr::AllocMiscHandle(WorldObject *obj)
{
    obj->SetUInt32Value(UNIT_FIELD_HANDLE, m_nMiscTop++);
    m_nItemTop++;
    AddObject(obj);
}

Player *MemoryPoolMgr::AllocPlayer()
{
    auto player = new Player{m_nPlayerTop++};
    AddObject(player);
    return player;
}

Summon *MemoryPoolMgr::AllocSummon(uint pCode)
{
    auto summon = new Summon{0, pCode};
    summon->SetInt32Value(UNIT_FIELD_HANDLE, m_nSummonTop++);
    AddObject(summon);
    return summon;
}

void MemoryPoolMgr::AllocItemHandle(Item *item)
{
    item->m_nHandle = m_nItemTop++;
    item->SetUInt32Value(UNIT_FIELD_HANDLE, item->m_nHandle);
    if (item->m_Instance.UID == 0)
    {
        item->m_Instance.UID = sWorld->GetItemIndex();
    }
    m_nMiscTop++;
    //m_hsItem.insert(std::make_pair<uint, Item *>((uint)(item->m_nHandle), (Item*)item));
    //HashMapHolder<Item>::Insert(item);
    AddObject(item);
}

Summon *MemoryPoolMgr::AllocNewSummon(Player *pPlayer, Item *pItem)
{
    if (pPlayer == nullptr || pItem == nullptr || pItem->m_pItemBase == nullptr)
        return nullptr;
    Summon *s = Summon::AllocSummon(pPlayer, (uint)pItem->m_pItemBase->summon_id);
    s->SetUInt32Value(UNIT_FIELD_UID, (uint)sWorld->GetSummonIndex());
    s->SetLevel(1);
    s->m_pItem = pItem;
    s->CalculateStat();
    s->SetJP(10);
    s->SetName(sObjectMgr->GetSummonName());

    s->SetFullHealth();
    s->SetMana(s->GetMaxMana());

    pItem->m_Instance.Socket[0] = s->GetUInt32Value(UNIT_FIELD_UID);
    pItem->m_bIsNeedUpdateToDB = true;
    pItem->m_pSummon           = s;
    return s;
}

Monster *MemoryPoolMgr::AllocMonster(uint idx)
{
    auto mb = sObjectMgr->GetMonsterInfo(idx);
    if (mb == nullptr)
        return nullptr;
    auto p = new Monster{m_nMonsterTop, mb};
    p->SetInt32Value(UNIT_FIELD_HANDLE, m_nMonsterTop);
    m_nMonsterTop++;
    //m_hsMonster.insert(std::make_pair<uint,Monster*>(p->GetHandle(), (Monster*)p));
    //HashMapHolder<Monster>::Insert(p);
    AddObject(p);
    return p;
}

void MemoryPoolMgr::Destroy()
{
    _unload<Player>();
    _unload<WorldObject>();
    _unload<Item>();
    _unload<Monster>();
    _unload<Summon>();
}

void MemoryPoolMgr::Update(uint diff)
{
    ///- Add new update objects
    WorldObject *sess = nullptr;
    while (addUpdateQueue.next(sess))
        i_objectsToUpdate[sess->GetHandle()] = sess;

    for (UpdateMap::iterator itr = i_objectsToUpdate.begin(), next; itr != i_objectsToUpdate.end(); itr = next)
    {
        next = itr;
        ++next;

        itr->second->Update(0);
        if (itr->second->IsDeleteRequested())
        {
            AddToDeleteList(itr->second);
            i_objectsToUpdate.erase(itr->second->GetHandle());
            continue;
        }
    }
    // First deleting all things in the remove list
    while (!i_objectsToRemove.empty())
    {
        std::set<WorldObject *>::iterator itr  = i_objectsToRemove.begin();
        WorldObject                       *obj = *itr;

        if (obj->IsInWorld())
            sWorld->RemoveObjectFromWorld(obj);

        switch (obj->GetSubType())
        {
            case ST_Player:
                RemoveObject((Player *)obj);
                break;
            case ST_Mob:
                RemoveObject((Monster *)obj);
                break;
            case ST_Summon:
                RemoveObject((Summon *)obj);
                break;
            case ST_Object: // In this case item
                RemoveObject((Item *)obj);
                break;
            default:
                RemoveObject(obj);
                break;
        }
        i_objectsToRemove.erase(itr);
        delete obj;
        //*&obj = nullptr;
    }
    sFieldPropManager->Update(diff);
    sItemCollector->Update();
}

Item *MemoryPoolMgr::AllocGold(int64 gold, GenerateCode gcode)
{
    return Item::AllocItem(0, 0, gold, gcode, -1, -1, -1, 0, 0, 0, 0, 0);
}

template<class T>
void MemoryPoolMgr::_unload()
{
    NG_UNIQUE_GUARD uniqueGuard(*HashMapHolder<T>::GetLock());
    auto            container = HashMapHolder<T>::GetContainer();
    for (auto &obj : container)
    {
        container.erase(obj.second->GetHandle());
        delete obj.second;
    }
    HashMapHolder<T>::GetContainer().clear();
}

void MemoryPoolMgr::AddToDeleteList(WorldObject *obj)
{
    i_objectsToRemove.insert(obj);
}

const HashMapHolder<Player>::MapType &MemoryPoolMgr::GetPlayers()
{
    return HashMapHolder<Player>::GetContainer();
}
