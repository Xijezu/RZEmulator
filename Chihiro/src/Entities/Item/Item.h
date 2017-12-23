//
// Created by xijezu on 01.12.17.
//

#ifndef PROJECT_ITEM_H
#define PROJECT_ITEM_H

#include "Common.h"
#include "ItemFields.h"
#include "Dynamic/UnorderedMap.h"
#include "DatabaseTemplates.h"

class Summon;

class ItemInstance {
public:
    uint         OwnerHandle{0};                            // 0x0
    uint         OwnSummonHandle{0};                        // 0x4
    long         UID{0};                                    // 0x8
    int          Code{0};                                    // 0x10
    int          nIdx{0};                                    // 0x14
    int          nLevel{0};                                  // 0x18
    int          nEnhance{0};                                // 0x1C
    int          nEndurance{0};
    int          nOwnerUID{0};                               // 0x20
    int          nOwnSummonUID{0};                           // 0x24
    int          nAuctionID{0};                              // 0x28
    int          nItemKeepingID{0};                          // 0x2C
    long         nCount{0};                                 // 0x30
    long         tExpire{0};                                // 0x40
    //Elemental::Type eElementalEffectType;         // 0x48
    int          Flag{0};                                   // 0x60
    GenerateCode GenerateInfo = GenerateCode::ByUnknown;      // 0x64
    ItemWearType nWearInfo{WearCantWear};             // 0x68
    int          Socket[4]{0};
};

class Item {
public:
    static const int MAX_COOLTIME_GROUP   = 40;
    static const int MAX_OPTION_NUMBER    = 4;
    static const int MAX_SOCKET_NUMBER    = 4;
    static const int MAX_ITEM_NAME_LENGTH = 32;
    static const int MAX_ITEM_WEAR        = 24;

    static Item *AllocItem(long uid, int code, long cnt, GenerateCode info, int level, int enhance,
            int flag, int socket_0, int socket_1, int socket_2, int socket_3, int remain_time);

    bool IsWearable();
    void DBUpdate();
    void DBInsert();

    ItemWearType GetWearType();

    void SetOwnerInfo(uint, int, int);

// private:
    ItemInstance m_Instance{ };
    ItemTemplate m_pItemBase{ };
    Summon       *m_pSummon{nullptr};
    uint32_t     m_nHandle{0};
    int          m_nAccountID{0};
    int          m_nItemID{0};
    uint32_t     m_unInventoryIndex{0};
    bool         m_bIsEventDrop{0};
    bool         m_bIsVirtualItem{0};
    bool         m_bIsNeedUpdateToDB{false};
};


#endif //PROJECT_ITEM_H
