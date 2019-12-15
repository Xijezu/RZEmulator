#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "ItemFields.h"
#include "Object.h"

class XPacket;
class Unit;
class Summon;

class ItemInstance
{
public:
  void Copy(const ItemInstance &pFrom);

  uint32_t OwnerHandle{0};     // 0x0
  uint32_t OwnSummonHandle{0}; // 0x4
  int64_t UID{0};              // 0x8
  int32_t Code{0};             // 0x10
  int32_t nIdx{0};             // 0x14
  int32_t nLevel{0};           // 0x18
  int32_t nEnhance{0};         // 0x1C
  int32_t nEndurance{0};
  int32_t nCurrentEndurance{0};
  int32_t nOwnerUID{0};      // 0x20
  int32_t nOwnSummonUID{0};  // 0x24
  int32_t nAuctionID{0};     // 0x28
  int32_t nItemKeepingID{0}; // 0x2C
  int64_t nCount{0};         // 0x30
  int64_t tExpire{0};        // 0x40
  //Elemental::Type eElementalEffectType;         // 0x48
  int32_t Flag{0};                        // 0x60
  GenerateCode GenerateInfo = BY_UNKNOWN; // 0x64
  ItemWearType nWearInfo{WEAR_CANTWEAR};  // 0x68
  int32_t Socket[4]{0};
};

class Item : public WorldObject
{
public:
  enum _TARGET_TYPE
  {
    TARGET_TYPE_PLAYER = 0,
    TARGET_TYPE_SUMMON = 1,
    TARGET_TYPE_MONSTER = 2,
    TARGET_TYPE_NPC = 3,
    TARGET_TYPE_UNKNOWN = 4
  };

  static void EnterPacket(XPacket &pEnterPct, Item *pItem);
  static Item *AllocItem(uint64_t uid, int32_t code, int64_t cnt, GenerateCode info = BY_BASIC, int32_t level = -1, int32_t enhance = -1,
                         int32_t flag = -1, int32_t socket_0 = 0, int32_t socket_1 = 0, int32_t socket_2 = 0, int32_t socket_3 = 0, int32_t remain_time = 0);
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

  inline int32_t GetItemCode() const { return m_Instance.Code; }
  inline int64_t GetItemUID() const { return m_Instance.UID; }
  inline int32_t GetOwnerUID() const { return m_Instance.nOwnerUID; }
  inline ItemWearType GetWearInfo() const { return m_Instance.nWearInfo; }
  inline ItemTemplate *GetItemBase() const { return m_pItemBase; }
  inline int32_t GetAccountID() const { return m_nAccountID; }
  inline int32_t GetCurrentEndurance() const { return m_Instance.nCurrentEndurance; }
  inline uint32_t GetBindedCreatureHandle() const { return m_hBindedTarget; }
  inline int32_t GetItemEnhance() const { return m_Instance.nEnhance; }
  inline int32_t GetItemGroup() const { return m_pItemBase != nullptr ? m_pItemBase->group : 0; }
  inline int32_t GetItemType() const { return m_pItemBase != nullptr ? m_pItemBase->type : 0; }
  int32_t GetSummonSID() const { return static_cast<int32_t>(m_Instance.Socket[0]); }
  ItemClass GetItemClass() const { return ((GetItemBase() != nullptr) ? static_cast<ItemClass>(GetItemBase()->iclass) : ItemClass::CLASS_ETC); }
  inline uint32_t GetOwnerHandle() const { return m_Instance.OwnerHandle; }
  inline int64_t GetCount() const { return m_Instance.nCount; }
  inline int32_t GetIdx() const { return m_Instance.nIdx; }
  inline uint32_t GetStorageIndex() const { return m_unInventoryIndex; }

  void SetStorageIndex(uint32_t idx);
  void SetSummonSID(int32_t idx);

  inline void SetWearInfo(ItemWearType wear_info)
  {
    m_Instance.nWearInfo = wear_info;
    m_bIsNeedUpdateToDB = true;
  }

  inline void SetOwnSummonInfo(uint32_t handle, int32_t UID)
  {
    m_Instance.OwnSummonHandle = handle;
    m_Instance.nOwnSummonUID = UID;
  }
  inline int32_t GetOwnSummonUID() const { return m_Instance.nOwnSummonUID; }
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

  inline bool IsTwoHandItem() const { return GetItemBase()->wear_type == ItemWearType::WEAR_TWOHAND; }

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

  // private:
  ItemInstance m_Instance{};
  ItemTemplate *m_pItemBase{};
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
};