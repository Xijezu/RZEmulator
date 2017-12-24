//
// Created by xijezu on 09.12.17.
//

#include "MemPool.h"
#include "World.h"

Item *MemoryPoolMgr::AllocItem()
{
    auto *p = new Item{ };
    p->m_nHandle = m_nItemTop++;
    m_hsItem.insert(std::pair<uint, Item *>(p->m_nHandle, p));
    return p;
}

void MemoryPoolMgr::AllocMiscHandle(WorldObject &obj)
{
    obj.SetUInt32Value(UNIT_FIELD_HANDLE, m_nMiscTop++);
    m_nItemTop++;
    //m_hsMisc.emplace(obj.GetHandle(), obj);
    m_hsMisc.insert(std::make_pair<uint, WorldObject *>(obj.GetHandle(), &obj));
}

Player *MemoryPoolMgr::AllocPlayer()
{
    auto player = new Player{m_nPlayerTop++};
    //m_hsPlayer.insert(std::make_pair<uint32, Player*>(player->GetHandle(), player));
    m_hsPlayer.emplace(player->GetHandle(), player);
    return player;
}

Summon *MemoryPoolMgr::AllocSummon(uint pCode)
{
    auto summon = new Summon(0, pCode);
    summon->SetInt32Value(UNIT_FIELD_HANDLE, m_nSummonTop++);
    m_hsSummon.emplace(summon->GetHandle(), summon);
    return summon;
}

Item *MemoryPoolMgr::FindItem(uint32_t handle)
{
    if (m_hsItem.count(handle) == 1)
        return m_hsItem[handle];
    return nullptr;
}

WorldObject* MemoryPoolMgr::getPtrFromId(uint32_t handle)
{
    WorldObject* result = nullptr;

    uint idbase = handle & 0xE0000000;
    if (idbase != 0)
    {
        switch (idbase)
        {
            case 0x20000000:
                result = getMiscPtrFromId(handle);
                break;
            /*case 0x40000000:
                result = (GameObject)getMonsterPtrFromId(uid);
                break;*/
            case 0x80000000:
                result = getPlayerPtrFromId(handle);
                break;
            /*case 0xC0000000:
                result = (GameObject)getSummonPtrFromId(uid);
                break;
            case 0xE0000000:
                result = (GameObject)getPetPtrFromId(uid);
                break;*/
            default:
                result = nullptr;
                break;
        }
    }
    else
    {
        //result = (GameObject) getItemPtrFromId(uid);
    }
    return result;
}

WorldObject *MemoryPoolMgr::getMiscPtrFromId(uint32_t handle)
{
    if (m_hsMisc.count(handle) == 1)
        return m_hsMisc[handle];
    return nullptr;
}

void MemoryPoolMgr::AllocItemHandle(Item *item)
{
    item->m_nHandle = m_nItemTop++;
    item->m_Instance.UID = sWorld->getItemIndex();
    m_nMiscTop++;
    m_hsItem.insert(std::make_pair<uint, Item *>((uint)(item->m_nHandle), (Item*)item));
}

WorldObject *MemoryPoolMgr::getPlayerPtrFromId(uint32_t handle)
{
    if(m_hsPlayer.count(handle) == 1)
        return m_hsPlayer[handle];
    return nullptr;
}

Item *MemoryPoolMgr::getItemPtrFromId(uint32_t handle)
{
    if(m_hsItem.count(handle) == 1)
        return m_hsItem[handle];
    return nullptr;
}
