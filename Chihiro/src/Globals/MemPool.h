#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "Common.h"
#include <Item/Item.h>
#include <Player/Player.h>
#include "Summon/Summon.h"
#include "Dynamic/UnorderedMap.h"
#include "Monster.h"
#include "SharedMutex.h"
#include "HashMapHolder.h"

typedef UNORDERED_MAP<uint32, WorldObject*> UpdateMap;

class MemoryPoolMgr {
private:
    /*MemoryPoolMgr() = default;
    ~MemoryPoolMgr() = default;
    MemoryPoolMgr(const MemoryPoolMgr&);
    MemoryPoolMgr& operator=(const MemoryPoolMgr&);*/
public:
    static MemoryPoolMgr* instance()
    {
        static auto *instance = new MemoryPoolMgr{};
        return instance;
    }

    template<class T> T* GetObjectInWorld(uint handle)
    {
        uint idbase = handle & 0xE0000000;
        switch (idbase) {
            case 0x00000000:
                return dynamic_cast<T*>(HashMapHolder<Item>::Find(handle));
            case 0x20000000:
                return dynamic_cast<T*>(HashMapHolder<WorldObject>::Find(handle));
            case 0x40000000:
                return dynamic_cast<T*>(HashMapHolder<Monster>::Find(handle));
            case 0x80000000:
                return dynamic_cast<T*>(HashMapHolder<Player>::Find(handle));
            case 0xC0000000:
                return dynamic_cast<T*>(HashMapHolder<Summon>::Find(handle));
            default:
                return nullptr;
        }
        ACE_NOTREACHED(return nullptr);
    }

    template<class T> void RemoveObject(T* object, bool bNeedToDelete=true)
    {
        HashMapHolder<T>::Remove(object);
    }
    template<class T> void AddObject(T* object)
    {
        HashMapHolder<T>::Insert(object);
        if(object->GetSubType() != ST_Object)
            addUpdateQueue.add(object);
    }


    Item *AllocItem();
    Item *AllocGold(uint64 gold, GenerateCode gcode);
    void AllocMiscHandle(WorldObject* obj);
    void AllocItemHandle(Item *item);
    Player *AllocPlayer();
    Summon *AllocSummon(uint);
    Monster* AllocMonster(uint idx);
    Summon *AllocNewSummon(Player*,Item*);

    void Destroy();
    void Update(uint diff);
    // when using this, you must use the HashMapHolders lock!
    const HashMapHolder<Player>::MapType &GetPlayers();
private:
    template<class T> void _unload();
    void AddToDeleteList(WorldObject* obj);
    std::set<WorldObject*> i_objectsToRemove{};
    ACE_Based::LockedQueue<WorldObject*, ACE_Thread_Mutex> addUpdateQueue{};

    UpdateMap i_objectsToUpdate{};

    uint32_t m_nMiscTop{0x20000001};
    uint32_t m_nMonsterTop{0x40000001};
    uint32_t m_nPlayerTop{0x80000001};
    uint32_t m_nSummonTop{0xC0000001};
#if EPIC >= 5
    uint32_t m_nPetTop{0xE0000001};
#endif
    uint32_t m_nItemTop{0x00000001};
};

#define sMemoryPool ACE_Singleton<MemoryPoolMgr, ACE_Thread_Mutex>::instance()
#endif //MEMORYPOOL_H
