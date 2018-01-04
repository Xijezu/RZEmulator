//
// Created by xijezu on 09.12.17.
//

#include "MemPool.h"
#include "World.h"
#include "ObjectMgr.h"

Item *MemoryPoolMgr::AllocItem()
{
    auto *p = new Item{ };
    p->m_nHandle = m_nItemTop++;
    p->SetUInt32Value(UNIT_FIELD_HANDLE, p->m_nHandle);
    m_hsItem.insert(std::pair<uint, Item *>(p->m_nHandle, p));
    return p;
}

void MemoryPoolMgr::AllocMiscHandle(WorldObject* obj)
{
    obj->SetUInt32Value(UNIT_FIELD_HANDLE, m_nMiscTop++);
    m_nItemTop++;
    //m_hsMisc.emplace(obj.GetHandle(), obj);
    m_hsMisc.insert(std::make_pair<uint, WorldObject *>(obj->GetHandle(), (WorldObject*)obj));
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
            case 0x40000000:
                result = getMonsterPtrFromId(handle);
                break;
            case 0x80000000:
                result = getPlayerPtrFromId(handle);
                break;
            case 0xC0000000:
                result = getSummonPtrFromId(handle);
                break;
            /*case 0xE0000000:
                result = (GameObject)getPetPtrFromId(uid);
                break;*/
            default:
                result = nullptr;
                break;
        }
    }
    else
    {
        result = getItemPtrFromId(handle);
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
    item->SetUInt32Value(UNIT_FIELD_HANDLE, item->m_nHandle);
    if(item->m_Instance.UID == 0) {
        item->m_Instance.UID = sWorld->GetItemIndex();
    }
    m_nMiscTop++;
    m_hsItem.insert(std::make_pair<uint, Item *>((uint)(item->m_nHandle), (Item*)item));
}

WorldObject *MemoryPoolMgr::getPlayerPtrFromId(uint32_t handle)
{
    if(m_hsPlayer.count(handle) == 1)
        return m_hsPlayer[handle];
    return nullptr;
}

WorldObject *MemoryPoolMgr::getItemPtrFromId(uint32_t handle)
{
    if(m_hsItem.count(handle) == 1)
        return m_hsItem[handle];
    return nullptr;
}

Summon *MemoryPoolMgr::AllocNewSummon(Player *pPlayer, Item *pItem)
{
    if(pPlayer == nullptr || pItem == nullptr || pItem->m_pItemBase == nullptr)
        return nullptr;
    std::string szName = "HurrDurr!"s;
    Summon* s = Summon::AllocSummon(pPlayer, (uint)pItem->m_pItemBase->summon_id);
    s->SetUInt32Value(UNIT_FIELD_UID, sWorld->GetSummonIndex());
    s->SetLevel(1);
    s->m_pItem = pItem;
    s->CalculateStat();
    s->SetJP(10);
    s->SetName(szName);

    s->SetFullHealth();
    s->SetMana(s->GetMaxMana());

    pItem->m_Instance.Socket[0] = s->GetUInt32Value(UNIT_FIELD_UID);
    pItem->m_bIsNeedUpdateToDB = true;
    pItem->m_pSummon = s;
    return s;
}

Monster *MemoryPoolMgr::AllocMonster(uint idx)
{
    auto mb = sObjectMgr->GetMonsterInfo(idx);
    if(mb == nullptr)
        return nullptr;
    auto p = new Monster{m_nMonsterTop, mb};
    p->SetInt32Value(UNIT_FIELD_HANDLE, m_nMonsterTop);
    m_nMonsterTop++;
    m_hsMonster.insert(std::make_pair<uint,Monster*>(p->GetHandle(), (Monster*)p));
    return p;
}

WorldObject *MemoryPoolMgr::getSummonPtrFromId(uint32_t handle)
{
    if(m_hsSummon.count(handle) == 1)
        return m_hsSummon[handle];
    return nullptr;
}

WorldObject *MemoryPoolMgr::getMonsterPtrFromId(uint32_t handle)
{
    if(m_hsMonster.count(handle) == 1)
        return m_hsMonster[handle];
    return nullptr;
}

void MemoryPoolMgr::Destroy()
{
    for(auto& obj : m_hsMisc) {
        delete obj.second;
    }
    m_hsMisc.clear();

    for(auto& obj : m_hsPlayer) {
        delete obj.second;
    }
    m_hsPlayer.clear();

    for(auto& obj : m_hsItem) {
        delete obj.second;
    }
    m_hsItem.clear();

    for(auto& obj : m_hsSummon) {
        delete obj.second;
    }
    m_hsSummon.clear();

    for(auto& obj : m_hsMonster) {
        delete obj.second;
    }
    m_hsMonster.clear();
}

void MemoryPoolMgr::Update(uint diff)
{
    for(auto& player : m_hsPlayer) {
        if(player.second == nullptr) {
            m_hsPlayer.erase(player.first);
            continue;
        }
        player.second->Update(diff);
    }

    for(auto& mob : m_hsMonster) {
        if(mob.second == nullptr) {
            m_hsMonster.erase(mob.first);
            continue;
        }
        mob.second->Update(diff);
    }

    for(auto& obj : m_hsMisc) {
        if(obj.second == nullptr) {
            m_hsMisc.erase(obj.first);
            continue;
        }
        obj.second->Update(diff);
    }
}

void MemoryPoolMgr::DeletePlayer(uint handle, bool bNeedToDelete)
{
    if(m_hsPlayer.count(handle) == 1) {
        if(bNeedToDelete) {
            delete m_hsPlayer[handle];
            m_hsPlayer[handle] = nullptr;
        }
        m_hsPlayer.erase(handle);
    }
}

void MemoryPoolMgr::DeleteMonster(uint handle, bool bNeedToDelete)
{
    if(m_hsMonster.count(handle) == 1) {
        if(bNeedToDelete) {
            delete m_hsMonster[handle];
            m_hsMonster[handle] = nullptr;
        }
        m_hsMonster.erase(handle);
    }
}

void MemoryPoolMgr::DeleteItem(uint handle, bool bNeedToDelete)
{
    if(m_hsItem.count(handle) == 1) {
        if(bNeedToDelete) {
            delete m_hsItem[handle];
            m_hsItem[handle] = nullptr;
        }
        m_hsItem.erase(handle);
    }
}

void MemoryPoolMgr::DeleteSummon(uint handle, bool bNeedToDelete)
{
    if(m_hsSummon.count(handle) == 1) {
        if(bNeedToDelete) {
            delete m_hsSummon[handle];
            m_hsSummon[handle] = nullptr;
        }
        m_hsSummon.erase(handle);
    }
}

void MemoryPoolMgr::DeleteNPC(uint handle, bool bNeedToDelete)
{
    if(m_hsMisc.count(handle) == 1) {
        if(bNeedToDelete) {
            delete m_hsMisc[handle];
            m_hsMisc[handle] = nullptr;
        }
        m_hsMisc.erase(handle);
    }
}

Item *MemoryPoolMgr::AllocGold(uint64 gold, GenerateCode gcode)
{
    return Item::AllocItem(0, 0, gold, gcode, 0, 0, 0, 0, 0, 0, 0, 0);
}
