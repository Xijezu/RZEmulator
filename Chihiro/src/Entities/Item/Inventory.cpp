/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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

Item *Inventory::Push(Item *pItem, int64_t cnt, bool bSkipUpdateItemToDB)
{
    Item *pExistItem{nullptr};
    if (pItem->IsJoinable())
        pExistItem = FindByCode(pItem->GetItemCode());

    if (!pItem->IsJoinable() || pExistItem == nullptr)
    {
        m_fWeight += pItem->GetWeight();
        if (pItem->GetWearInfo() != ItemWearType::WEAR_NONE)
            m_fWeightModifier -= pItem->GetWeight();

        push(pItem, bSkipUpdateItemToDB);

        if (pItem->GetIdx() > m_nIndex)
            m_nIndex = pItem->GetIdx();

        return pItem;
    }

    m_fWeight += pExistItem->GetWeight();
    if (pExistItem->GetWearInfo() != ItemWearType::WEAR_NONE)
        m_fWeightModifier -= pExistItem->GetWeight();

    setCount(pExistItem, pExistItem->GetCount() + cnt, bSkipUpdateItemToDB);
    return pExistItem;
}

Item *Inventory::Pop(Item *pItem, int64_t cnt, bool bSkipUpdateItemToDB)
{
    check(pItem);

    if (pItem->GetCount() > cnt)
    {
        auto pNewItem = Item::AllocItem(0, pItem->GetItemCode(), 1, GenerateCode::BY_DIVIDE);

        pNewItem->CopyFrom(pItem);
        pNewItem->SetCount(cnt);

        m_fWeight -= pItem->GetWeight();
        if (pItem->GetWearInfo() != ItemWearType::WEAR_NONE)
            m_fWeightModifier += pItem->GetWeight();

        setCount(pItem, pItem->GetCount() - cnt, bSkipUpdateItemToDB);
        return pNewItem;
    }

    if (pItem->GetCount() == cnt)
    {
        m_fWeight -= pItem->GetWeight();
        if (pItem->GetWearInfo() != ItemWearType::WEAR_NONE)
            m_fWeightModifier += pItem->GetWeight();

        pop(pItem, bSkipUpdateItemToDB);
        return pItem;
    }

    return nullptr;
}

bool Inventory::Erase(Item *pItem, int64_t count, bool bSkipUpdateItemToDB)
{
    if (!check(pItem))
        return false;

    if (pItem->GetCount() > count)
    {
        m_fWeight -= pItem->GetWeight();
        if (pItem->GetWearInfo() != ItemWearType::WEAR_NONE)
            m_fWeightModifier += pItem->GetWeight();
        setCount(pItem, pItem->GetCount() - count, bSkipUpdateItemToDB);
        return true;
    }

    m_fWeight -= pItem->GetWeight();
    if (pItem->GetWearInfo() != ItemWearType::WEAR_NONE)
        m_fWeightModifier += pItem->GetWeight();
    pop(pItem, bSkipUpdateItemToDB);
    pItem->DeleteThis();
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
    for (auto &i : m_vList)
    {
        if (i->m_Instance.Code == code)
            return i;
    }
    return nullptr;
}

Item *Inventory::FindBySID(int64_t uid)
{
    for (auto &i : m_vList)
    {
        if (i->m_Instance.UID == uid)
            return i;
    }
    return nullptr;
}

Item *Inventory::FindByHandle(uint handle)
{
    for (auto &i : m_vList)
    {
        if (i->GetHandle() == handle)
            return i;
    }
    return nullptr;
}

void Inventory::setCount(Item *pItem, int64_t newCnt, bool bSkipUpdateItemToDB)
{
    pItem->SetCount(newCnt);
    if (m_pEventReceiver != nullptr)
        m_pEventReceiver->onChangeCount(this, pItem, bSkipUpdateItemToDB);
}

void Inventory::push(Item *pItem, bool bSkipUpdateItemToDB)
{
    m_vList.emplace_back(pItem);

    if (pItem->IsExpireItem())
        m_vExpireItemList.emplace_back(pItem);

    pItem->SetStorageIndex(static_cast<uint32_t>(m_vList.size() - 1));
    if (m_pEventReceiver != nullptr)
        m_pEventReceiver->onAdd(this, pItem, bSkipUpdateItemToDB);
}

void Inventory::pop(Item *pItem, bool bSkipUpdateItemToDB)
{
    if (m_pEventReceiver != nullptr)
        m_pEventReceiver->onRemove(this, pItem, bSkipUpdateItemToDB);

    if (m_vList.back() != pItem)
    {
        m_vList.back()->SetStorageIndex(pItem->GetStorageIndex());
        m_vList[pItem->GetStorageIndex()] = m_vList.back();
    }
    m_vList.pop_back();

    auto pos = std::find(m_vExpireItemList.begin(), m_vExpireItemList.end(), pItem);
    if (pos != m_vExpireItemList.end())
        m_vExpireItemList.erase(pos);
}

bool Inventory::check(Item *pItem) const
{
    ASSERT(pItem->GetStorageIndex() < m_vList.size(), "Inventory::check: GetStorageIndex invalid!!!");
    ASSERT(m_vList[pItem->GetStorageIndex()] == pItem, "Inventory::check: Position in list invalid!!!");
    return true;
}
