/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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

#include "Item.h"

#include "DatabaseEnv.h"
#include "GameRule.h"
#include "MemPool.h"
#include "Messages.h"
#include "ObjectMgr.h"

Item *Item::AllocItem(uint64_t nUID, int32_t nCode, int64_t nCount, GenerateCode eGenerateCode, int32_t nLevel, int32_t nEnhance, int32_t nFlag, int32_t nSocket0, int32_t nSocket1, int32_t nSocket2,
    int32_t nSocket3, int32_t nRemainingTime)
{
    Item *pItem = sMemoryPool.AllocItem();
    if (pItem == nullptr)
        return nullptr;

    pItem->m_pItemBase = sObjectMgr.GetItemBase((uint32_t)nCode);

    pItem->GetItemInstance().SetUID(nUID);
    pItem->GetItemInstance().SetCode(nCode);
    pItem->GetItemInstance().SetCount(nCount);

    // Workaround for Gold (handled as item, too)
    // Don't set that stuff if it's gold
    if (pItem->GetItemTemplate() != nullptr) {
        pItem->GetItemInstance().SetLevel(nLevel == -1 ? pItem->GetItemTemplate()->level : nLevel);
        if (pItem->GetItemInstance().GetLevel() <= 0)
            pItem->GetItemInstance().SetLevel(1);

        pItem->GetItemInstance().SetEnhance(nEnhance == -1 ? pItem->GetItemTemplate()->enhance : nEnhance);
        if (pItem->GetItemGroup() == ItemGroup::GROUP_SKILLCARD && pItem->GetItemInstance().GetEnhance() == 0)
            pItem->GetItemInstance().SetEnhance(1);

        pItem->GetItemInstance().SetFlag(nFlag == -1 ? pItem->GetItemTemplate()->status_flag : nFlag);
        pItem->GetItemInstance().SetGenerateInfo(eGenerateCode);
        pItem->GetItemInstance().SetCurrentEndurance(pItem->GetItemBase()->endurance);
    }

    pItem->GetItemInstance().SetOwnerHandle(0);
    pItem->GetItemInstance().SetOwnerUID(0);
    pItem->GetItemInstance().SetSocket({nSocket0, nSocket1, nSocket2, nSocket3});
    pItem->GetItemInstance().SetExpire(nRemainingTime);

    return pItem;
}

ItemWearType Item::GetWearType()
{
    if (GetItemTemplate() == nullptr)
        return ItemWearType::WEAR_NONE;
    return GetItemTemplate()->eWearType;
}

bool Item::IsWearable()
{
    if (GetWearType() == ItemWearType::WEAR_CANTWEAR)
        return false;

    // If Flag has the bit "FLAG FAILED" set, you can't wear it
    // @todo: Create a HasFlag function
    return (~((GetItemInstance().GetFlag()) >> FlagBits::ITEM_FLAG_FAILED) & 1) != 0;
}

void Item::DBUpdate()
{
    /*if (!m_bIsNeedUpdateToDB)
        return;*/

    uint8_t i = 0;
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_ITEM);

    stmt->setInt32(i++, GetItemInstance().GetOwnerUID());
    stmt->setInt32(i++, m_nAccountID);
    stmt->setInt32(i++, GetItemInstance().GetOwnSummonUID());
    stmt->setInt32(i++, GetItemInstance().GetAuctionID());
    stmt->setInt32(i++, GetItemInstance().GetItemKeepingID());
    stmt->setInt32(i++, m_unInventoryIndex);
    stmt->setInt64(i++, GetItemInstance().GetCount());
    stmt->setInt32(i++, GetItemInstance().GetLevel());
    stmt->setInt32(i++, GetItemInstance().GetEnhance());
    stmt->setInt32(i++, GetItemInstance().GetFlag());
    stmt->setInt32(i++, static_cast<int32_t>(GetItemInstance().GetItemWearType()));
    stmt->setInt32(i++, GetItemInstance().GetSocketIndex(0));
    stmt->setInt32(i++, GetItemInstance().GetSocketIndex(1));
    stmt->setInt32(i++, GetItemInstance().GetSocketIndex(2));
    stmt->setInt32(i++, GetItemInstance().GetSocketIndex(3));
    stmt->setInt32(i++, GetItemInstance().GetExpire());
    stmt->setInt32(i, GetItemInstance().GetUID());

    CharacterDatabase.Execute(stmt);

    m_bIsNeedUpdateToDB = false;
}

void Item::SetOwnerInfo(uint32_t hHandle, int32_t nUID, int32_t account_id)
{
    GetItemInstance().SetOwnerHandle(hHandle);
    GetItemInstance().SetOwnerUID(nUID);
    m_nAccountID = account_id;
}

void Item::DBInsert()
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_ITEM);
    uint8_t idx = 0;

    stmt->setUInt64(idx++, GetItemInstance().GetUID()); // owner_id
    stmt->setInt32(idx++, GetItemInstance().GetOwnerUID()); // owner_id
    stmt->setInt32(idx++, GetAccountID()); // account_id
    stmt->setInt32(idx++, GetItemInstance().GetOwnSummonUID()); // summon_id
    stmt->setInt32(idx++, GetItemInstance().GetAuctionID()); // auction_id
    stmt->setInt32(idx++, GetItemInstance().GetItemKeepingID()); // keepind_id
    stmt->setInt32(idx++, GetItemInstance().GetIndex()); // Idx
    stmt->setInt32(idx++, GetItemInstance().GetCode()); // code
    stmt->setInt64(idx++, GetItemInstance().GetCount()); // Count
    stmt->setInt32(idx++, GetItemInstance().GetLevel()); // level
    stmt->setInt32(idx++, GetItemInstance().GetEnhance()); // enhance
    stmt->setInt32(idx++, GetItemInstance().GetEndurance()); // Endurance
    stmt->setInt32(idx++, GetItemInstance().GetFlag()); // Flag
    stmt->setInt32(idx++, static_cast<int32_t>(GetItemInstance().GetGenerateCode())); // GCode
    stmt->setInt32(idx++, static_cast<int32_t>(GetItemInstance().GetItemWearType())); // Wear Info
    stmt->setInt32(idx++, GetItemInstance().GetSocketIndex(0)); // Socket_0
    stmt->setInt32(idx++, GetItemInstance().GetSocketIndex(1)); // Socket_1
    stmt->setInt32(idx++, GetItemInstance().GetSocketIndex(2)); // Socket_2
    stmt->setInt32(idx++, GetItemInstance().GetSocketIndex(3)); // Socket_3
    stmt->setInt32(idx++, 0); // Socket_4
    stmt->setInt32(idx++, 0); // Socket_5
    stmt->setInt32(idx++, GetItemInstance().GetExpire()); // remain time
    stmt->setInt32(idx++, 0); // elemental effect type
    stmt->setInt32(idx++, 0); // elemental effect expire_time
    stmt->setInt32(idx++, 0); // elemental effect attack point
    stmt->setInt32(idx, 0); // elemental effect magic point

    CharacterDatabase.Execute(stmt);

    m_bIsNeedUpdateToDB = false;
}

void Item::EnterPacket(TS_SC_ENTER &pEnterPct, Item *pItem)
{
    TS_SC_ENTER__ITEM_INFO itemInfo{};
    itemInfo.code = pItem->GetItemInstance().GetCode();
    itemInfo.count = pItem->GetItemInstance().GetCount();
    itemInfo.pick_up_order.drop_time = pItem->m_nDropTime;
    for(auto i = 0; i < 3; i++) {
        itemInfo.pick_up_order.hPlayer[i] = pItem->m_pPickupOrder.hPlayer[i];
        itemInfo.pick_up_order.nPartyID[i] = pItem->m_pPickupOrder.nPartyID[i];
    }
    pEnterPct.itemInfo = itemInfo;
}

void Item::SetPickupOrder(const ItemPickupOrder &order)
{
    for (int32_t i = 0; i < 3; i++) {
        m_pPickupOrder.hPlayer[i] = order.hPlayer[i];
        m_pPickupOrder.nPartyID[i] = order.nPartyID[i];
    }
}

void Item::PendFreeItem(Item *pItem)
{
    // sMemoryPool.RemoveObject(pItem, true);
    pItem->DeleteThis();
}

int32_t Item::GetLevelLimit()
{
    return GameRule::GetItemLevelLimit(GetItemTemplate()->rank);
}

bool Item::IsBow()
{
    return GetItemTemplate()->eClass == ItemClass::CLASS_HEAVY_BOW || GetItemTemplate()->eClass == ItemClass::CLASS_LIGHT_BOW;
}

bool Item::IsCrossBow()
{
    return GetItemTemplate()->eClass == ItemClass::CLASS_CROSSBOW;
}

bool Item::IsCashItem()
{
    return (GetItemTemplate()->flaglist[FLAG_CASHITEM] == 1);
}

int32_t Item::GetItemRank() const
{
    return GetItemTemplate()->rank;
}

void Item::SetCurrentEndurance(int32_t nCurrentEndurance)
{
    int32_t nMaxEndurance = GetMaxEndurance();
    GetItemInstance().SetCurrentEndurance(nCurrentEndurance);

    if (nCurrentEndurance > nMaxEndurance)
        GetItemInstance().SetCurrentEndurance(nMaxEndurance);
    if (nCurrentEndurance < 0)
        GetItemInstance().SetCurrentEndurance(0);

    m_bIsNeedUpdateToDB = true;
}

int32_t Item::GetMaxEndurance() const
{
    int32_t result = 0;

    if (GetItemTemplate() == nullptr)
        return 0;

    if (GetItemTemplate()->socket != 0) {
        if (GetItemTemplate()->socket <= 0)
            return GetItemTemplate()->endurance;

        int32_t total_endurance = 0;
        for (int32_t i = 0; i < GetItemTemplate()->socket; ++i) {
            if (GetItemInstance().GetSocketIndex(i) != 0) {
                total_endurance += sObjectMgr.GetItemBase(GetItemInstance().GetSocketIndex(i))->endurance;
            }
        }
        if (total_endurance != 0)
            result = total_endurance;
        else
            result = GetItemTemplate()->endurance;
    }
    else {
        result = GetItemTemplate()->endurance;
    }
    return result;
}

bool Item::IsQuestItem() const
{
    if (GetItemTemplate() == nullptr)
        return false;
    return GetItemTemplate()->flaglist[FLAG_QUEST] != 0;
}

bool Item::IsJoinable() const
{
    if (GetItemTemplate() == nullptr)
        return false;
    return GetItemTemplate()->flaglist[FLAG_DUPLICATE] != 0;
}

float Item::GetWeight() const
{
    return GetItemTemplate()->weight * (float)GetItemInstance().GetCount();
}

void Item::SetCount(int64_t count)
{
    GetItemInstance().SetCount(count);
}

void Item::SetIdx(int32_t idx)
{
    GetItemInstance().SetIdx(idx);
}

void Item::SetSummonSID(int32_t sid)
{
    GetItemInstance().SetOwnSummonUID(sid);
}

void Item::SetStorageIndex(uint32_t idx)
{
    m_unInventoryIndex = idx;
}

bool Item::IsExpireItem() const
{
    return GetItemTemplate()->decrease_type == 1 || GetItemTemplate()->decrease_type == 2;
}

bool Item::IsInInventory() const
{
    return m_nAccountID == 0 && GetItemInstance().GetOwnerUID() != 0;
}

bool Item::IsInStorage() const
{
    return m_nAccountID != 0 && GetItemInstance().GetOwnerUID() == 0 && GetItemInstance().GetCode() != 0;
}

void Item::CopyFrom(const Item *pFrom)
{
    auto oldOwner = GetItemInstance().GetOwnerHandle();
    auto oldUID = GetItemInstance().GetUID();
    Relocate(pFrom->GetPositionX(), pFrom->GetPositionY(), pFrom->GetPositionZ(), pFrom->GetOrientation());
    SetLayer(pFrom->GetLayer());
    m_Instance.Copy(pFrom->m_Instance);
    GetItemInstance().SetUID(0);
    GetItemInstance().SetOwnerUID((int32_t)oldUID);
    GetItemInstance().SetOwnerHandle(oldOwner);
}

void Item::SetBindTarget(Unit *pUnit)
{
    if (pUnit != nullptr && pUnit->IsPlayer())
        GetItemInstance().SetSocketIndex(0, pUnit->GetUInt32Value(UNIT_FIELD_UID));
    else
        GetItemInstance().SetSocketIndex(0, 0);

    if (pUnit != nullptr && pUnit->IsSummon())
        GetItemInstance().SetSocketIndex(1, pUnit->GetUInt32Value(UNIT_FIELD_UID));
    else
        GetItemInstance().SetSocketIndex(1, 0);

    if (pUnit != nullptr)
        m_hBindedTarget = pUnit->GetHandle();
    else
        m_hBindedTarget = 0;

    m_bIsNeedUpdateToDB = true;
}

Item::Item()
    : WorldObject(true)
{
    _mainType = MT_StaticObject;
    _subType = ST_Object;
    _objType = OBJ_STATIC;

    _valuesCount = UNIT_FIELD_HANDLE + 1;
    _InitValues();
}

bool Item::IsTradable()
{
    if (GetItemTemplate() == nullptr)
        return false;

    return GetItemTemplate()->flaglist[FLAG_TRADE] == 0;
}

bool Item::IsDropable()
{
    if (GetItemTemplate() == nullptr)
        return false;

    return GetItemTemplate()->flaglist[FLAG_DROP] == 0;
}
