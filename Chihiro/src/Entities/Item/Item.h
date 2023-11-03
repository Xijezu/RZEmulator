#pragma once
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

#include "Common.h"
#include "DatabaseTemplates.h"
#include "ItemInstance.h"
#include "Object.h"

class Unit;
class Summon;
struct TS_SC_ENTER;

class Item : public WorldObject {
public:
    enum _TARGET_TYPE { TARGET_TYPE_PLAYER = 0, TARGET_TYPE_SUMMON = 1, TARGET_TYPE_MONSTER = 2, TARGET_TYPE_NPC = 3, TARGET_TYPE_UNKNOWN = 4 };

    static void EnterPacket(TS_SC_ENTER &pEnterPct, Item *pItem);
    static Item *AllocItem(uint64_t uid, int32_t code, int64_t cnt, GenerateCode info = GenerateCode::BY_BASIC, int32_t level = -1, int32_t enhance = -1, int32_t flag = -1, int32_t socket_0 = 0,
        int32_t socket_1 = 0, int32_t socket_2 = 0, int32_t socket_3 = 0, int32_t remain_time = 0);
    static void PendFreeItem(Item *pItem);

    Item();
    // Deleting the copy & assignment operators
    // Better safe than sorry
    Item(const Item &) = delete;
    Item &operator=(const Item &) = delete;

    void SetCount(int64_t count);
    void SetIdx(int32_t idx);
    bool IsDropable();
    bool IsWearable();

    inline int32_t GetItemCode() const { return GetItemInstance().GetCode(); }
    inline int64_t GetItemUID() const { return GetItemInstance().GetUID(); }
    inline int32_t GetOwnerUID() const { return GetItemInstance().GetOwnerUID(); }
    inline ItemWearType GetWearInfo() const { return GetItemInstance().GetItemWearType(); }
    inline int32_t GetCurrentEndurance() const { return GetItemInstance().GetCurrentEndurance(); }
    inline int32_t GetItemEnhance() const { return GetItemInstance().GetEnhance(); }
    inline uint32_t GetSummonSID() const { return static_cast<uint32_t>(GetItemInstance().GetSocketIndex(0)); }
    inline uint32_t GetOwnerHandle() const { return GetItemInstance().GetOwnerHandle(); }
    inline int64_t GetCount() const { return GetItemInstance().GetCount(); }
    inline int32_t GetIdx() const { return GetItemInstance().GetIndex(); }
    inline int32_t GetOwnSummonUID() const { return GetItemInstance().GetOwnSummonUID(); }

    inline int32_t GetAccountID() const { return m_nAccountID; }
    inline uint32_t GetBindedCreatureHandle() const { return m_hBindedTarget; }
    inline ItemGroup GetItemGroup() const { return GetItemTemplate() != nullptr ? GetItemTemplate()->eGroup : ItemGroup::GROUP_ETC; }
    inline ItemType GetItemType() const { return GetItemTemplate() != nullptr ? GetItemTemplate()->eType : ItemType::TYPE_ETC; }

    ItemClass GetItemClass() const { return ((GetItemBase() != nullptr) ? GetItemBase()->eClass : ItemClass::CLASS_ETC); }

    inline uint32_t GetStorageIndex() const { return m_unInventoryIndex; }
    inline std::shared_ptr<ItemTemplate> GetItemBase() const { return m_pItemBase; }

    void SetStorageIndex(uint32_t idx);
    void SetSummonSID(int32_t idx);

    inline void SetWearInfo(ItemWearType wear_info)
    {
        GetItemInstance().SetWearInfo(wear_info);
        m_bIsNeedUpdateToDB = true;
    }

    inline void SetOwnSummonInfo(uint32_t handle, int32_t UID)
    {
        GetItemInstance().SetOwnSummonUID(handle);
        GetItemInstance().SetOwnSummonUID(UID);
    }
    Summon *GetSummon() const { return m_pSummon; }
    void SetBindedCreatureHandle(uint32_t target) { m_hBindedTarget = target; }

    bool IsTradable();
    bool IsExpireItem() const;
    bool IsJoinable() const;
    bool IsQuestItem() const;
    float GetWeight() const;
    void CopyFrom(const Item *pFrom);

    inline bool IsBullet() const { return GetItemGroup() == ItemGroup::GROUP_BULLET; }
    inline bool IsBelt() const { return GetItemGroup() == ItemGroup::GROUP_BELT; }
    inline bool IsWeapon() const { return GetItemGroup() == ItemGroup::GROUP_WEAPON; }
    inline bool IsAccessory() const { return GetItemGroup() == ItemGroup::GROUP_ACCESSORY; }
    inline bool IsItemCard() const { return GetItemGroup() == ItemGroup::GROUP_ITEMCARD; }
    inline bool IsSummonCard() const { return GetItemGroup() == ItemGroup::GROUP_SUMMONCARD; }
    inline bool IsSkillCard() const { return GetItemGroup() == ItemGroup::GROUP_SKILLCARD; }
    inline bool IsSpellCard() const { return GetItemGroup() == ItemGroup::GROUP_SPELLCARD; }
    inline bool IsStrikeCube() const { return GetItemGroup() == ItemGroup::GROUP_STRIKE_CUBE; }
    inline bool IsDefenceCube() const { return GetItemGroup() == ItemGroup::GROUP_DEFENCE_CUBE; }
    inline bool IsSkillCube() const { return GetItemGroup() == ItemGroup::GROUP_SKILL_CUBE; }
    inline bool IsArtifact() const { return GetItemGroup() == ItemGroup::GROUP_ARTIFACT; }
    inline bool IsCharm() const { return GetItemType() == ItemType::TYPE_CHARM; }

    inline bool IsTwoHandItem() const { return GetItemBase()->eWearType == ItemWearType::WEAR_TWOHAND; }

    bool IsInInventory() const;
    bool IsInStorage() const;

    void DBUpdate();
    void DBInsert();
    void SetCurrentEndurance(int32_t n);
    int32_t GetMaxEndurance() const;
    void SetBindTarget(Unit *pUnit);

    ItemWearType GetWearType();
    int32_t GetLevelLimit();
    int32_t GetItemRank() const;
    bool IsBow();
    bool IsCrossBow();
    bool IsCashItem();

    bool IsItem() const override { return true; }

    void SetOwnerInfo(uint32_t, int32_t, int32_t);
    void SetPickupOrder(const ItemPickupOrder &order);

    inline ItemInstance &GetItemInstance() { return m_Instance; }
    inline ItemInstance const &GetItemInstance() const { return m_Instance; }

    inline ItemTemplate *GetItemTemplate() const { return m_pItemBase.get(); }

    Summon *m_pSummon{nullptr};
    uint32_t m_nHandle{0};
    int32_t m_nAccountID{0};
    int32_t m_nItemID{0};
    uint32_t m_hBindedTarget{0};
    uint32_t m_unInventoryIndex{0};
    uint32_t m_nDropTime{0};
    bool m_bIsEventDrop{0};
    bool m_bIsVirtualItem{0};
    bool m_bIsNeedUpdateToDB{false};
    ItemPickupOrder m_pPickupOrder{};

private:
    ItemInstance m_Instance{};
    std::shared_ptr<ItemTemplate> m_pItemBase{};
};