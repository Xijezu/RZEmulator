//
// Created by xijezu on 01.12.17.
//

#include "Item/Item.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "XPacket.h"
#include "DatabaseEnv.h"
#include "Messages.h"
#include "GameRule.h"

Item *Item::AllocItem(uint64 uid, int code, uint64 cnt, GenerateCode info, int level, int enhance,
                      int flag, int socket_0, int socket_1, int socket_2, int socket_3, int remain_time)
{
    Item *item = sMemoryPool->AllocItem();
    if(item == nullptr)
        return nullptr;

    item->m_pItemBase = sObjectMgr->GetItemBase((uint)code);

    item->m_Instance.UID          = uid;
    item->m_Instance.Code         = code;
    item->m_Instance.nCount       = cnt;

    // Workaround for gold :^)
    if(item->m_pItemBase != nullptr) {

        if (level == -1)
            item->m_Instance.nLevel = item->m_pItemBase->level;
        else
            item->m_Instance.nLevel = level;
        if (item->m_Instance.nLevel <= 0)
            item->m_Instance.nLevel = 1;

        item->m_Instance.GenerateInfo = info;
        item->m_Instance.Flag         = flag;

        if (enhance == -1)
            item->m_Instance.nEnhance = item->m_pItemBase->enhance;
        else
            item->m_Instance.nEnhance = enhance;

        if (item->m_pItemBase->group == 10 && item->m_Instance.nEnhance == 0)
            item->m_Instance.nEnhance = 1;

        if (flag == -1)
            item->m_Instance.Flag = item->m_pItemBase->status_flag;
        else
            item->m_Instance.Flag = flag;
    }

    item->m_Instance.OwnerHandle  = 0;
    item->m_Instance.nOwnerUID    = 0;
    item->m_Instance.Socket[0] = socket_0;
    item->m_Instance.Socket[1] = socket_1;
    item->m_Instance.Socket[2] = socket_2;
    item->m_Instance.Socket[3] = socket_3;
    item->m_Instance.tExpire = remain_time;

    return item;
}

ItemWearType Item::GetWearType()
{
    return (ItemWearType) m_pItemBase->wear_type;
}

bool Item::IsWearable()
{
    if (GetWearType() == ItemWearType::WearCantWear)
        return false;
    else
        return (~((m_Instance.Flag) >> 3) & 1) != 0;
}

void Item::DBUpdate()
{
    /*if (!m_bIsNeedUpdateToDB)
        return;*/

    uint8_t i = 0;
    //PrepareStatement(CHARACTER_UPD_ITEM, "UPDATE `Item` SET owner_id = ?, account_id = ?,
    // summon_id = ?, auction_id = ?, keeping_id = ?, idx = ?, cnt = ?, level = ?, enhance = ?,
    // flag = ?, wear_info = ?, socket_0 = ?, socket_1 = ?, socket_2 = ?, socket_3 = ?, remain_time = ?, update_time = ? WHERE sid = ?", CONNECTION_ASYNC);
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_ITEM);

    stmt->setInt32(i++, m_Instance.nOwnerUID);
    stmt->setInt32(i++, m_nAccountID);
    stmt->setInt32(i++, m_Instance.nOwnSummonUID);
    stmt->setInt32(i++, m_Instance.nAuctionID);
    stmt->setInt32(i++, m_Instance.nItemKeepingID);
    stmt->setInt32(i++, m_unInventoryIndex);
    stmt->setInt64(i++, m_Instance.nCount);
    stmt->setInt32(i++, m_Instance.nLevel);
    stmt->setInt32(i++, m_Instance.nEnhance);
    stmt->setInt32(i++, m_Instance.Flag);
    stmt->setInt32(i++, m_Instance.nWearInfo);
    stmt->setInt32(i++, m_Instance.Socket[0]);
    stmt->setInt32(i++, m_Instance.Socket[1]);
    stmt->setInt32(i++, m_Instance.Socket[2]);
    stmt->setInt32(i++, m_Instance.Socket[3]);
    stmt->setInt32(i++, m_Instance.tExpire);
    stmt->setInt32(i++, m_Instance.UID);

    CharacterDatabase.Execute(stmt);

    m_bIsNeedUpdateToDB = false;
}

void Item::SetOwnerInfo(uint handle, int UID, int account_id)
{
    m_Instance.OwnerHandle = handle;
    m_Instance.nOwnerUID = UID;
    m_nAccountID = account_id;
}

void Item::DBInsert()
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_ITEM);
    uint8_t idx = 0;

    stmt->setUInt64(idx++, m_Instance.UID); // owner_id
    stmt->setInt32(idx++, m_Instance.nOwnerUID); // owner_id
    stmt->setInt32(idx++, 0); // account_id
    stmt->setInt32(idx++, m_Instance.nOwnSummonUID); // summon_id
    stmt->setInt32(idx++, m_Instance.nAuctionID); // auction_id
    stmt->setInt32(idx++, m_Instance.nItemKeepingID); // keepind_id
    stmt->setInt32(idx++, m_Instance.nIdx); // Idx
    stmt->setInt32(idx++, m_Instance.Code); // code
    stmt->setInt64(idx++, m_Instance.nCount); // Count
    stmt->setInt32(idx++, m_Instance.nLevel); // level
    stmt->setInt32(idx++, m_Instance.nEnhance); // enhance
    stmt->setInt32(idx++, m_Instance.nEndurance); // Endurance
    stmt->setInt32(idx++, m_Instance.Flag); // Flag
    stmt->setInt32(idx++, m_Instance.GenerateInfo); // GCode
    stmt->setInt32(idx++, m_Instance.nWearInfo); // Wear Info
    stmt->setInt32(idx++, m_Instance.Socket[0]); // Socket_0
    stmt->setInt32(idx++, m_Instance.Socket[1]); // Socket_1
    stmt->setInt32(idx++, m_Instance.Socket[2]); // Socket_2
    stmt->setInt32(idx++, m_Instance.Socket[3]); // Socket_3
    stmt->setInt32(idx++, 0); // Socket_4
    stmt->setInt32(idx++, 0); // Socket_5
    stmt->setInt32(idx++, m_Instance.tExpire); // remain time
    stmt->setInt32(idx++, 0); // elemental effect type
    stmt->setInt32(idx++, 0); // elemental effect expire_time
    stmt->setInt32(idx++, 0); // elemental effect attack point
    stmt->setInt32(idx++, 0); // elemental effect magic point

    CharacterDatabase.Execute(stmt);

    m_bIsNeedUpdateToDB = false;
}

void Item::EnterPacket(XPacket &pEnterPct, Item *pItem)
{
    Messages::GetEncodedInt(pEnterPct, pItem->m_Instance.Code);
    pEnterPct << (uint64)pItem->m_Instance.nCount;

    pEnterPct << (uint)pItem->m_nDropTime;
    for(int i = 0; i < 3; i++) {
        pEnterPct << (uint)pItem->m_pPickupOrder.hPlayer[i];
        pEnterPct << (uint)pItem->m_pPickupOrder.nPartyID[i];
    }
}

void Item::SetPickupOrder(const ItemPickupOrder &order)
{
    for(int i = 0; i < 3; i++) {
        m_pPickupOrder.hPlayer[i] = order.hPlayer[i];
        m_pPickupOrder.nPartyID[i] = order.nPartyID[i];
    }
}

void Item::PendFreeItem(Item *pItem)
{
    sMemoryPool->RemoveObject(pItem, true);
}

int Item::GetLevelLimit()
{
    return GameRule::GetItemLevelLimit(m_pItemBase->rank);
}

bool Item::IsBow()
{
    return m_pItemBase->iclass == ItemClass::ClassHeavyBow || m_pItemBase->iclass == ItemClass::ClassLightBow;
}

bool Item::IsCrossBow()
{
    return m_pItemBase->iclass == ItemClass::ClassCrossBow;
}

int Item::GetItemRank() const
{
    return m_pItemBase->rank;
}
