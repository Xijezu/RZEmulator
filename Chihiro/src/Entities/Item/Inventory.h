#ifndef PROJECT_INVENTORY_H
#define PROJECT_INVENTORY_H

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

        InventoryEventReceiver* m_pEventReceiver;
        float m_fWeight;
        float m_fWeightModifier;
        std::vector<Item*> m_vList;
        std::vector<Item*> m_vExpireItemList;
        int m_nIndex;
};

#endif // PROJECT_INVENTORY_H
