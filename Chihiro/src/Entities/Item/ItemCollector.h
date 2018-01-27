#ifndef PROJECT_ITEMCOLLECTOR_H
#define PROJECT_ITEMCOLLECTOR_H

#include "Common.h"
#include "SharedMutex.h"

class Item;
class ItemCollector
{
    public:
        ItemCollector() = default;
        ~ItemCollector();
        void RegisterItem(Item *pItem);
        bool UnregisterItem(Item *pItem);
        void Update();

    private:
        typedef UNORDERED_MAP<uint, Item *> ItemMap;

        ItemMap m_vItemList;
        MX_SHARED_MUTEX i_lock;
};
#define sItemCollector ACE_Singleton<ItemCollector, ACE_Null_Mutex>::instance()
#endif // PROJECT_ITEMCOLLECTOR_H
