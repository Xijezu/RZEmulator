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

#include "ItemCollector.h"
#include "Item.h"
#include "World.h"

ItemCollector::~ItemCollector()
{
    NG_UNIQUE_GUARD writeGuard(i_lock);
    for(auto& item : m_vItemList)
    {
        Item::PendFreeItem(item.second);
    }
}

void ItemCollector::RegisterItem(Item *pItem)
{
    if(pItem == nullptr)
        return;
    NG_UNIQUE_GUARD writeGuard(i_lock);
    if(m_vItemList.count(pItem->GetHandle()) != 0)
        return;
    m_vItemList[pItem->GetHandle()] = pItem;
}

bool ItemCollector::UnregisterItem(Item *pItem)
{
    if(pItem == nullptr)
        return false;
    NG_UNIQUE_GUARD writeGuard(i_lock);
    if(m_vItemList.count(pItem->GetHandle()) == 0)
        return false;

    m_vItemList.erase(pItem->GetHandle());
    return true;
}

void ItemCollector::Update()
{
    NG_UNIQUE_GUARD writeGuard(i_lock);
    uint ct = sWorld.GetArTime();
    for (ItemMap::iterator itr = m_vItemList.begin(), next; itr != m_vItemList.end(); itr = next)
    {
        next = itr;
        ++next;

        if(itr->second->m_nDropTime + sWorld.getIntConfig(CONFIG_ITEM_HOLD_TIME) <= ct)
        {
            if(itr->second->IsInWorld())
                sWorld.RemoveObjectFromWorld(itr->second);
            m_vItemList.erase(itr);
            Item::PendFreeItem(itr->second);
        }
    }
}
