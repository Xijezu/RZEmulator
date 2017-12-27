#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "Common.h"
#include <Item/Item.h>
#include <Player/Player.h>
#include "Summon/Summon.h"
#include "Dynamic/UnorderedMap.h"
#include "Monster.h"

class MemoryPoolMgr {
public:
    MemoryPoolMgr() = default;
    ~MemoryPoolMgr() = default;

    Item *AllocItem();
    void AllocMiscHandle(WorldObject &obj);
    void AllocItemHandle(Item *item);
    Player *AllocPlayer();
    Summon *AllocSummon(uint);
    Monster* AllocMonster(uint idx);
    Summon *AllocNewSummon(Player*,Item*);

    Item *FindItem(uint32_t handle);
    WorldObject* getPtrFromId(uint32_t handle);
    WorldObject* getMiscPtrFromId(uint32_t handle);
    WorldObject* getMonsterPtrFromId(uint32_t handle);
    WorldObject* getPlayerPtrFromId(uint32_t handle);
    Item* getItemPtrFromId(uint32_t handle);

private:
    uint32_t m_nMiscTop{0x20000001};
    uint32_t m_nMonsterTop{0x40000001};
    uint32_t m_nPlayerTop{0x80000001};
    uint32_t m_nSummonTop{0xC0000001};
    uint32_t m_nPetTop{0xE0000001};
    uint32_t m_nItemTop{0x00000001};

    std::map<uint, WorldObject *> m_hsMisc{ };
    std::map<uint, Player *>      m_hsPlayer{ };
    std::map<uint, Item *>        m_hsItem{ };
    std::map<uint, Summon *>      m_hsSummon{ };
    std::map<uint, Monster *>     m_hsMonster{ };

};

#define sMemoryPool ACE_Singleton<MemoryPoolMgr, ACE_Null_Mutex>::instance()
#endif //MEMORYPOOL_H
