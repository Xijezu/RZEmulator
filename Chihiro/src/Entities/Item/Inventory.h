#pragma once
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

#include "Common.h"

class Inventory;
class Item;
struct InventoryEventReceiver
{
    virtual void onAdd(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) = 0;
    virtual void onRemove(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) = 0;
    virtual void onChangeCount(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) = 0;
};

class Inventory
{
    public:
        friend class Player;
        Inventory();
        ~Inventory() = default;

        Item* Push(Item* item, int64 cnt, bool bSkipUpdateItemToDB);
        Item* Pop(Item* pItem, int64 cnt, bool bSkipUpdateItemToDB);
        bool Erase(Item* pItem, int64 count, bool bSkipUpdateItemToDB);
        Item* Find(int code, uint flag, bool bFlag);
        Item* FindByCode(int code);
        Item* FindBySID(int64 uid);
        Item* FindByHandle(uint handle);
    private:
        void setCount(Item* item, int64 newCnt, bool bSkipUpdateItemToDB);
        void push(Item* item, bool bSkipUpdateItemToDB);
        void pop(Item* pItem, bool bSkipUpdateItemToDB);
        bool check(Item* pItem);

        InventoryEventReceiver * m_pEventReceiver;
        float              m_fWeight;
        float              m_fWeightModifier;
        std::vector<Item*> m_vList;
        std::vector<Item*> m_vExpireItemList;
        int                m_nIndex;
};