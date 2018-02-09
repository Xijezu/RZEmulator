//
// Created by xijezu on 01.12.17.
//

#ifndef PROJECT_ITEM_H
#define PROJECT_ITEM_H

#include "Common.h"
#include "ItemFields.h"
#include "Dynamic/UnorderedMap.h"
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
        uint64       UID{0};                                    // 0x8
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

        void SetCount(int64 count);
        bool IsWearable();
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


#endif //PROJECT_ITEM_H
