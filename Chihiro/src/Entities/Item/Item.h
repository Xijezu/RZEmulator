#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "ItemFields.h"
#include "DatabaseTemplates.h"
#include "Object.h"

class XPacket;
class Unit;
class Summon;

class ItemInstance
{
    public:
        void Copy(const ItemInstance &pFrom);

        uint         OwnerHandle{0};                            // 0x0
        uint         OwnSummonHandle{0};                        // 0x4
        int64        UID{0};                                    // 0x8
        int          Code{0};                                    // 0x10
        int          nIdx{0};                                    // 0x14
        int          nLevel{0};                                  // 0x18
        int          nEnhance{0};                                // 0x1C
        int          nEndurance{0};
        int          nCurrentEndurance{0};
        int          nOwnerUID{0};                               // 0x20
        int          nOwnSummonUID{0};                           // 0x24
        int          nAuctionID{0};                              // 0x28
        int          nItemKeepingID{0};                          // 0x2C
        int64        nCount{0};                                 // 0x30
        int64        tExpire{0};                                // 0x40
        //Elemental::Type eElementalEffectType;         // 0x48
        int          Flag{0};                                   // 0x60
        GenerateCode GenerateInfo = BY_UNKNOWN;      // 0x64
        ItemWearType nWearInfo{WEAR_CANTWEAR};             // 0x68
        int          Socket[4]{0};
};

class Item : public WorldObject
{
    public:
        static void EnterPacket(XPacket &pEnterPct, Item *pItem);
        static Item *AllocItem(uint64 uid, int code, int64 cnt, GenerateCode info, int level, int enhance,
                int flag, int socket_0, int socket_1, int socket_2, int socket_3, int remain_time);
        static void PendFreeItem(Item *pItem);

        Item();
        // Deleting the copy & assignment operators
        // Better safe than sorry
        Item(const Item &) = delete;
        Item &operator=(const Item &) = delete;

        void SetCount(int64 count);
		bool IsDropable();
        bool IsWearable();

        Summon *GetSummon() const { return m_pSummon; }
        bool IsTradable();
        bool IsExpireItem() const;
        bool IsJoinable() const;
        bool IsQuestItem() const;
        float GetWeight() const;
        void CopyFrom(const Item *pFrom);

        bool IsInInventory() const;
        bool IsInStorage() const;

        void DBUpdate();
        void DBInsert();
        void SetCurrentEndurance(int n);
        int GetMaxEndurance() const;
        void SetBindTarget(Unit *pUnit);

        ItemWearType GetWearType();
        int GetLevelLimit();
        int GetItemRank() const;
        bool IsBow();
        bool IsCrossBow();
        bool IsCashItem();

        bool IsItem() const override { return true; }

        void SetOwnerInfo(uint, int, int);
        void SetPickupOrder(const ItemPickupOrder &order);

// private:
        ItemInstance    m_Instance{ };
        ItemTemplate    *m_pItemBase{ };
        Summon          *m_pSummon{nullptr};
        uint            m_nHandle{0};
        int             m_nAccountID{0};
        int             m_nItemID{0};
        uint            m_hBindedTarget{0};
        uint            m_unInventoryIndex{0};
        uint            m_nDropTime{0};
        bool            m_bIsEventDrop{0};
        bool            m_bIsVirtualItem{0};
        bool            m_bIsNeedUpdateToDB{false};
        ItemPickupOrder m_pPickupOrder{ };
};