#pragma once
#include "Common.h"

constexpr int64_t MAX_GOLD_FOR_INVENTORY = 100000000000;
constexpr int64_t MAX_GOLD_FOR_STORAGE = 100000000000;
constexpr int32_t MAX_OPTION_NUMBER = 4;
constexpr int32_t MAX_ITEM_WEAR = 24;
constexpr int32_t MAX_COOLTIME_GROUP = 40;
constexpr int32_t MAX_SOCKET_NUMBER = 4;
constexpr int32_t MAX_ITEM_NAME_LENGTH = 32;
constexpr int32_t MAX_SPARE_ITEM_WEAR{28};

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

struct ItemPickupOrder {
    uint32_t hPlayer[3];
    int32_t nPartyID[3];
};


enum class ITEM_EFFECT_PASSIVE {
    NONE = 0,

    ATTACK_POint32_t = 11,
    MAGIC_POint32_t = 12,
    ACCURACY = 13,
    ATTACK_SPEED = 14,
    DEFENCE = 15,
    MAGIC_DEFENCE = 16,
    AVOID = 17,
    MOVE_SPEED = 18,
    BLOCK_CHANGE = 19,
    CARRY_WEIGHT = 20,
    BLOCK_DEFENCE = 21,
    CASTING_SPEED = 22,
    MAGIC_ACCURACY = 23,
    MAGIC_AVOID = 24,
    COOLTIME_SPEED = 25,
    BELT_SLOT = 26,
    MAX_CHAOS = 27,
    MAX_HP = 30,
    MAX_MP = 31,
    BOW_INTERVAL = 34,
    MP_REGEN_POint32_t = 33,
    TAMED_ITEM = 95,
    INC_PARAMETER_A = 96,
    INC_PARAMETER_B = 97,
    AMP_PARAMETER_A = 98,
    AMP_PARAMETER_B = 99,
};

enum ItemCode : int32_t {
    CHALK_OF_RESTORATION = 800000,
    UNIT_CARD = 800001,
    FEATHER_OF_RETURN = 910006,
    FEATHER_OF_REINSTATEMENT = 910009,
    E_REPAIR_POWDER = 950021,
    FEATHER_OF_RETURN_EVENT = 2902104
};

enum {
    IEP_NONE = 0,
    IEP_ATTACK_POINT = 11,
    IEP_MAGIC_POINT = 12,
    IEP_ACCURACY = 13,
    IEP_ATTACK_SPEED = 14,
    IEP_DEFENCE = 15,
    IEP_MAGIC_DEFENCE = 16,
    IEP_AVOID = 17,
    IEP_MOVE_SPEED = 18,
    IEP_BLOCK_CHANCE = 19,
    IEP_CARRY_WEIGHT = 20,
    IEP_BLOCK_DEFENCE = 21,
    IEP_CASTING_SPEED = 22,
    IEP_MAGIC_ACCURACY = 23,
    IEP_MAGIC_AVOID = 24,
    IEP_COOLTIME_SPEED = 25,
    IEP_BELT_SLOT = 26,
    IEP_MAX_CHAOS = 27,
    IEP_MAX_HP = 30,
    IEP_MAX_MP = 31,
    IEP_BOW_INTERVAL = 34,
    IEP_MP_REGEN_POINT = 33,
    IEP_TAMED_ITEM = 95,
    IEP_INC_PARAMETER_A = 96,
    IEP_INC_PARAMETER_B = 97,
    IEP_AMP_PARAMETER_A = 98,
    IEP_AMP_PARAMETER_B = 99,
};

enum class ITEM_EFFECT_INSTANT {
    NONE = 0,

    INC_HP = 1,
    INC_MP = 2,
    WARP = 3,
    RESURECTION = 4,
    SKILL = 5,
    ADD_STATE = 6,
    REMOVE_STATE = 7,
    TOGGLE_STATE = 8,
    ADD_IMMORAL_POint32_t = 41,
    SET_IMMORAL_POint32_t = 42,
    WARP_TO_SPECIAL_POSITION = 43,
    INC_STAMINA = 80,
    SUMMON_PET = 90,
    GENERATE_ITEM = 94,
    INC_HP_PERCENT = 101,
    INC_MP_PERCENT = 102,
    INC_GOLD = 103,
    INC_HUNTAHOLIC_POint32_t = 104,
    RECALL = 112,
    RESET_SKILL = 113,
    RESET_JOB = 118,
    RENAME_SUMMON = 115,
    RESET_SUMMON_SKILL = 116,
    WARP_TO_PLAYER = 117,
    RENAME_CHARACTER = 119,
    RENAME_PET = 120,
    ADD_CASH = 121,
    ADD_STATE_EX = 124,
};

enum ElementalType : int32_t { TYPE_NONE = 0, TYPE_FIRE = 1, TYPE_WATER = 2, TYPE_WIND = 3, TYPE_EARTH = 4, TYPE_LIGHT = 5, TYPE_DARK = 6, TYPE_COUNT = 7, TYPE_UNKN = 99 };

enum FlagBits : uint32_t {
    ITEM_FLAG_NORMAL = 0x00,
    ITEM_FLAG_CARD = 0x01,
    ITEM_FLAG_FULL = 0x02,
    ITEM_FLAG_INSERTED = 0x04,
    ITEM_FLAG_FAILED = 0x08,
    ITEM_FLAG_EVENT = 0x10,
    ITEM_FLAG_CONTAIN_PET = 0x20,
    ITEM_FLAG_TAMING = 0x20000000,
    ITEM_FLAG_NON_CHAOS_STONE = 0x40000000,
    ITEM_FLAG_SUMMON = 0x80000000,
};

/// \brief This is actually the idx for the ItemBase::flaglist
/// Not the retail bitset
enum ItemFlag : int32_t {
    FLAG_CASHITEM = 0,
    FLAG_WEAR = 1,
    FLAG_USE = 2,
    FLAG_TARGET_USE = 3,
    FLAG_DUPLICATE = 4,
    FLAG_DROP = 5,
    FLAG_TRADE = 6,
    FLAG_SELL = 7,
    FLAG_STORAGE = 8,
    FLAG_OVERWEIGHT = 9,
    FLAG_RIDING = 10,
    FLAG_MOVE = 11,
    FLAG_SIT = 12,
    FLAG_ENHANCE = 13,
    FLAG_QUEST = 14,
    FLAG_RAID = 15,
    FLAG_SECROUTE = 16,
    FLAG_EVENTMAP = 17,
    FLAG_HUNTAHOLIC = 18
};


enum GenerateCode : int32_t {
    BY_MONSTER = 0,
    BY_MARKET = 1,
    BY_QUEST = 2,
    BY_SCRIPT = 3,
    BY_MIX = 4,
    BY_GM = 5,
    BY_BASIC = 6,
    BY_TRADE = 7,
    BY_DIVIDE = 8,
    BY_ITEM = 10,
    BY_FIELD_PROP = 11,
    BY_AUCTION = 12,
    BY_SHOVELING = 13,
    BY_HUNTAHOLIC = 14,
    BY_DONATION_REWARD = 15,
    BY_UNKNOWN = 126
};

enum ItemWearType : int16_t {
    WEAR_CANTWEAR = -1,
    WEAR_NONE = -1,
    WEAR_WEAPON = 0,
    WEAR_SHIELD = 1,
    WEAR_ARMOR = 2,
    WEAR_HELM = 3,
    WEAR_GLOVE = 4,
    WEAR_BOOTS = 5,
    WEAR_BELT = 6,
    WEAR_MANTLE = 7,
    WEAR_ARMULET = 8,
    WEAR_RING = 9,
    WEAR_SECOND_RING = 10,
    WEAR_EAR = 11,
    WEAR_FACE = 12,
    WEAR_HAIR = 13,
    WEAR_DECO_WEAPON = 14,
    WEAR_DECO_SHIELD = 15,
    WEAR_DECO_ARMOR = 16,
    WEAR_DECO_HELM = 17,
    WEAR_DECO_GLOVE = 18,
    WEAR_DECO_BOOTS = 19,
    WEAR_DECO_MANTLE = 20,
    WEAR_DECO_SHOULDER = 21,
    WEAR_RIDE_ITEM = 22,
    WEAR_BAG_SLOT = 23,
    WEAR_TWOFINGER_RING = 94,
    WEAR_TWOHAND = 99,
    WEAR_SKILL = 100,
    WEAR_RIGHTHAND = 0,
    WEAR_LEFTHAND = 1,
    WEAR_BULLET = 1,
};

enum ItemClass : int32_t {
    CLASS_ETC = 0,
    CLASS_DOUBLE_AXE = 95,
    CLASS_DOUBLE_SWORD = 96,
    CLASS_DOUBLE_DAGGER = 98,
    CLASS_EVERY_WEAPON = 99,
    CLASS_ETCWEAPON = 100,
    CLASS_ONEHAND_SWORD = 101,
    CLASS_TWOHAND_SWORD = 102,
    CLASS_DAGGER = 103,
    CLASS_TWOHAND_SPEAR = 104,
    CLASS_TWOHAND_AXE = 105,
    CLASS_ONEHAND_MACE = 106,
    CLASS_TWOHAND_MACE = 107,
    CLASS_HEAVY_BOW = 108,
    CLASS_LIGHT_BOW = 109,
    CLASS_CROSSBOW = 110,
    CLASS_ONEHAND_STAFF = 111,
    CLASS_TWOHAND_STAFF = 112,
    CLASS_ONEHAND_AXE = 113,
    CLASS_ARMOR = 200,
    CLASS_FIGHTER_ARMOR = 201,
    CLASS_HUNTER_ARMOR = 202,
    CLASS_MAGICIAN_ARMOR = 203,
    CLASS_SUMMONER_ARMOR = 204,
    CLASS_SHIELD = 210,
    CLASS_HELM = 220,
    CLASS_BOOTS = 230,
    CLASS_GLOVE = 240,
    CLASS_BELT = 250,
    CLASS_MANTLE = 260,
    CLASS_ETC_ACCESSORY = 300,
    CLASS_RING = 301,
    CLASS_EARRING = 302,
    CLASS_ARMULET = 303,
    CLASS_EYEGLASS = 304,
    CLASS_MASK = 305,
    CLASS_CUBE = 306,
    CLASS_BOOST_CHIP = 400,
    CLASS_SOULSTONE = 401,
    CLASS_DECO_SHIELD = 601,
    CLASS_DECO_ARMOR = 602,
    CLASS_DECO_HELM = 603,
    CLASS_DECO_GLOVE = 604,
    CLASS_DECO_BOOTS = 605,
    CLASS_DECO_MALTLE = 606,
    CLASS_DECO_SHOULDER = 607,
    CLASS_DECO_HAIR = 608,
    CLASS_DECO_ONEHAND_SWORD = 609,
    CLASS_DECO_TWOHAND_SWORD = 610,
    CLASS_DECO_DAGGER = 611,
    CLASS_DECO_TWOHAND_SPEAR = 612,
    CLASS_DECO_TWOHAND_AXE = 613,
    CLASS_DECO_ONEHAND_MACE = 614,
    CLASS_DECO_TWOHAND_MACE = 615,
    CLASS_DECO_HEAVY_BOW = 616,
    CLASS_DECO_LIGHT_BOW = 617,
    CLASS_DECO_CROSSBOW = 618,
    CLASS_DECO_ONEHAND_STAFF = 619,
    CLASS_DECO_TWOHAND_STAFF = 620,
    CLASS_DECO_ONEHAND_AXE = 621,
};

enum ItemType : int32_t {
    TYPE_ETC = 0,
    TYPE_ARMOR = 1,
    TYPE_CARD = 2,
    TYPE_SUPPLY = 3,
    TYPE_CUBE = 4,
    TYPE_CHARM = 5,
    TYPE_USE = 6,
    TYPE_SOULSTONE = 7,
    TYPE_USE_CARD = 8,
};

enum ItemGroup : int32_t {
    GROUP_ETC = 0,
    GROUP_WEAPON = 1,
    GROUP_ARMOR = 2,
    GROUP_SHIELD = 3,
    GROUP_HELM = 4,
    GROUP_GLOVE = 5,
    GROUP_BOOTS = 6,
    GROUP_BELT = 7,
    GROUP_MANTLE = 8,
    GROUP_ACCESSORY = 9,
    GROUP_SKILLCARD = 10,
    GROUP_ITEMCARD = 11,
    GROUP_SPELLCARD = 12,
    GROUP_SUMMONCARD = 13,
    GROUP_FACE = 15,
    GROUP_UNDERWEAR = 16,
    GROUP_BAG = 17,
    GROUP_PETCAGE = 18,
    GROUP_STRIKE_CUBE = 21,
    GROUP_DEFENCE_CUBE = 22,
    GROUP_SKILL_CUBE = 23,
    GROUP_CASH_ITEM = 80,
    GROUP_SOULSTONE = 93,
    GROUP_BULLET = 98,
    GROUP_USEABLE = 99,
    GROUP_DECO = 110,
    GROUP_RIDING = 120,
    GROUP_ARTIFACT = 130,
    GROUP_BOSSCARD = 140,
};

enum LIMIT_FLAG : int32_t {
    LIMIT_DEVA = (1 << 2),
    LIMIT_ASURA = (1 << 3),
    LIMIT_GAIA = (1 << 4),

    LIMIT_FIGHTER = (1 << 10),
    LIMIT_HUNTER = (1 << 11),
    LIMIT_MAGICIAN = (1 << 12),
    LIMIT_SUMMONER = (1 << 13),
};

struct ItemTemplate {
    int32_t nID;
    int32_t nNameID;
    ItemType eType;
    ItemGroup eGroup;
    ItemClass eClass;
    ItemWearType eWearType;
    int32_t set_id;
    int32_t set_part_flag;
    int32_t rank;
    int32_t level;
    int32_t enhance;
    int32_t socket;
    int32_t status_flag;
    int32_t limit_deva;
    int32_t limit_asura;
    int32_t limit_gaia;
    int32_t limit_fighter;
    int32_t limit_hunter;
    int32_t limit_magician;
    int32_t limit_summoner;
    int32_t nLimit;
    int32_t use_min_level;
    int32_t use_max_level;
    int32_t target_min_level;
    int32_t target_max_level;
    float range;
    float weight;
    int32_t price;
    int32_t endurance;
    int32_t material;
    int32_t summon_id;
    int8_t flaglist[19];
    int32_t available_period;
    int16_t decrease_type;
    float throw_range;
    int8_t distribute_type;
    int16_t base_type[4];
    float base_var[4][2];
    int16_t opt_type[4];
    float opt_var[4][2];
    int16_t enhance_id[2];
    float _enhance[2][4];
    int32_t skill_id;
    int32_t state_id;
    int32_t state_level;
    int32_t state_time;
    int32_t state_type;
    int32_t cool_time;
    int16_t cool_time_group;
    std::string script_text;

    void SetCombinedFlags()
    {
        if (limit_asura != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_ASURA);
        if (limit_gaia != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_GAIA);
        if (limit_deva != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_DEVA);

        if (limit_hunter != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_HUNTER);
        if (limit_fighter != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_FIGHTER);
        if (limit_magician != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_MAGICIAN);
        if (limit_summoner != 0)
            nLimit |= static_cast<int32_t>(LIMIT_FLAG::LIMIT_SUMMONER);
    }
};