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

#include "Inventory.h"
#include "Item.h"

Inventory::Inventory() : m_pEventReceiver(nullptr), m_fWeight(0),
                         m_fWeightModifier(0), m_vList(),
                         m_vExpireItemList(), m_nIndex(0)
{

}

Item *Inventory::Push(Item *item, int64 cnt, bool bSkipUpdateItemToDB)
{
    if(item->IsJoinable())
    {
        auto ji = FindByCode(item->m_Instance.Code);
        if(ji != nullptr)
        {
            m_fWeight += item->m_pItemBase->weight * (float)cnt;
            if(ji->m_Instance.nWearInfo != WEAR_NONE)
                m_fWeightModifier -= item->m_pItemBase->weight * (float)cnt;
            int64 new_cnt = cnt + ji->m_Instance.nCount;
            setCount(ji, new_cnt, bSkipUpdateItemToDB);
            return ji;
        }
    }

    m_fWeight += item->GetWeight();
    if(item->m_Instance.nWearInfo != WEAR_NONE)
    {
        m_fWeightModifier -= item->GetWeight();
    }
    push(item, bSkipUpdateItemToDB);
    if(item->m_Instance.nIdx > m_nIndex)
        m_nIndex = item->m_Instance.nIdx;

    return item;
}

Item *Inventory::Pop(Item *pItem, int64 cnt, bool bSkipUpdateItemToDB)
{
    int64 new_cnt;

    if (!check(pItem))
        return nullptr;

    if(pItem->m_Instance.nCount == cnt)
    {
        m_fWeight -= pItem->m_pItemBase->weight * (float)cnt;
        if(pItem->m_Instance.nWearInfo != WEAR_NONE)
            m_fWeightModifier += pItem->m_pItemBase->weight * (float)cnt;
        pop(pItem, bSkipUpdateItemToDB);
        return pItem;
    }
    else if(pItem->m_Instance.nCount > cnt)
    {
        new_cnt = pItem->m_Instance.nCount - cnt;
        auto pNewItem = Item::AllocItem(0, pItem->m_Instance.Code,new_cnt, BY_DIVIDE, -1, -1, -1 -1 ,0 ,0 ,0 ,0 ,0);
        pNewItem->CopyFrom(pItem);
        pNewItem->SetCount(cnt);
        m_fWeight -= pItem->m_pItemBase->weight * (float)cnt;
        if(pItem->m_Instance.nWearInfo != WEAR_NONE)
            m_fWeightModifier += pItem->m_pItemBase->weight * (float)cnt;
        new_cnt = pItem->m_Instance.nCount - cnt;
        setCount(pItem, new_cnt, bSkipUpdateItemToDB);
        return pNewItem;
    }
    return nullptr;
}

bool Inventory::Erase(Item *pItem, int64 count, bool bSkipUpdateItemToDB)
{
    if(!check(pItem))
        return false;

    if(pItem->m_Instance.nCount <= count)
    {
        m_fWeight -= pItem->GetWeight();
        if(pItem->m_Instance.nWearInfo != WEAR_NONE)
            m_fWeightModifier += pItem->GetWeight();
        pop(pItem, bSkipUpdateItemToDB);
        Item::PendFreeItem(pItem);
        return true;
    }

    m_fWeight -= pItem->m_pItemBase->weight * (float)count;
    if(pItem->m_Instance.nWearInfo != WEAR_NONE)
        m_fWeightModifier += pItem->m_pItemBase->weight * (float)count;
    int64 nc = pItem->m_Instance.nCount - count;
    setCount(pItem, nc, bSkipUpdateItemToDB);
    return true;
}

Item *Inventory::Find(int code, uint flag, bool bFlag)
{
    for (auto &i : m_vList)
    {
        bool isFlagged = (flag & i->m_Instance.Flag) != 0;
        if (i->m_Instance.Code == code)
        {
            if (bFlag == isFlagged)
                return i;
        }
    }
    return nullptr;
}

Item *Inventory::FindByCode(int code)
{
    for(auto& i : m_vList)
    {
        if(i->m_Instance.Code == code)
            return i;
    }
    return nullptr;
}

Item *Inventory::FindBySID(int64 uid)
{
    for(auto& i : m_vList)
    {
        if(i->m_Instance.UID == uid)
            return i;
    }
    return nullptr;
}

Item *Inventory::FindByHandle(uint handle)
{
    for(auto& i : m_vList)
    {
        if(i->GetHandle() == handle)
            return i;
    }
    return nullptr;
}

void Inventory::setCount(Item *item, int64 newCnt, bool bSkipUpdateItemToDB)
{
    item->SetCount(newCnt);
    if(m_pEventReceiver != nullptr)
        m_pEventReceiver->onChangeCount(this, item, bSkipUpdateItemToDB);
}

void Inventory::push(Item *item, bool bSkipUpdateItemToDB)
{
    m_vList.emplace_back(item);
    if(item->IsExpireItem())
        m_vExpireItemList.emplace_back(item);
    item->m_unInventoryIndex = (uint)m_vList.size() - 1;
    if(m_pEventReceiver != nullptr)
        m_pEventReceiver->onAdd(this, item, bSkipUpdateItemToDB);
}

void Inventory::pop(Item *pItem, bool bSkipUpdateItemToDB)
{
    if(m_pEventReceiver != nullptr)
        m_pEventReceiver->onRemove(this, pItem, bSkipUpdateItemToDB);

    Item* last = m_vList.back();
    auto idx = (int)last->m_unInventoryIndex;
    if(last->GetHandle() != pItem->GetHandle())
    {
        last->m_unInventoryIndex = pItem->m_unInventoryIndex;
        m_vList[(int)last->m_unInventoryIndex] = last;
    }
    m_vList.erase(m_vList.begin() + idx);
    auto pos = std::find(m_vExpireItemList.begin(), m_vExpireItemList.end(), pItem);
    if(pos != m_vExpireItemList.end())
        m_vExpireItemList.erase(pos);
}

bool Inventory::check(Item *pItem)
{
    if(pItem->m_unInventoryIndex < m_vList.size())
        return m_vList[(int)pItem->m_unInventoryIndex]->GetHandle() == pItem->GetHandle();
    return false;
}
