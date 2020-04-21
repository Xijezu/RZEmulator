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
#include "ItemFields.h"
#include "TS_SC_SKILL.h"

enum SKILL_TYPE : int16_t
{
    ST_INVALID = 0,
    ST_VALID = 1,
    ST_SYSTEM = 2,
};

enum SKILL_EFFECT_TARGET_LIMIT : int
{
    SKILL_EFFECT_TARGET_LIMIT_ANY_BODY = 0,
    SKILL_EFFECT_TARGET_LIMIT_NOT_ENEMY = 1,
    SKILL_EFFECT_TARGET_LIMIT_ONLY_ALLY = 2,
    SKILL_EFFECT_TARGET_LIMIT_ONLY_ENEMY = 3,
};

enum REGION_TYPE
{
    REGION_TYPE_DIRECTION = 0,
    REGION_TYPE_ARC_CIRCLE = 1,
    REGION_TYPE_CROSS = 2,
};

enum DISTRIBUTION_TYPE
{
    DISTRIBUTION_TYPE_NO_LIMIT = 0,
    DISTRIBUTION_TYPE_DISTRIBUTE = 1,
    DISTRIBUTION_TYPE_RANDOM = 2,
    DISTRIBUTION_TYPE_SEQUENTIAL_TARGET = 3,
    DISTRIBUTION_TYPE_SEQUENTIAL_CASTER = 4,
};

struct SkillResult
{
    uint8_t type;
    uint32_t hTarget;

    TS_SC_SKILL__HIT_DAMAGE hitDamage;
    TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK hitDamageWithKnockBack;
    TS_SC_SKILL__HIT_RESULT hitResult;
    TS_SC_SKILL__HIT_ADD_STAT hitAddStat;
    TS_SC_SKILL__HIT_ADDHPMPSP hitAddHPMPSP;
    TS_SC_SKILL__HIT_REBIRTH hitRebirth;
    TS_SC_SKILL__HIT_RUSH hitRush;
};

enum TARGET_TYPE : int
{
    TARGET_MISC = 0,
    TARGET_TARGET = 1,
    TARGET_REGION_WITH_TARGET = 2,
    TARGET_REGION_WITHOUT_TARGET = 3,
    TARGET_REGION = 4,
    TARGET_TARGET_EXCEPT_CASTER = 6,
    TARGET_PARTY = 21,
    TARGET_GUILD = 22,
    TARGET_ATTACKTEAM = 23,
    TARGET_SUMMON = 31,
    TARGET_PARTY_SUMMON = 32,
    TARGET_REGION_NEAR_MAIN_SUMMON_WITHOUT_TARGET = 44,
    TARGET_SELF_WITH_SUMMON = 45,
    TARGET_PARTY_WITH_SUMMON = 51,
    TARGET_MASTER = 101,
    TARGET_SELF_WITH_MASTER = 102,
    TARGET_CREATURE_TYPE_NONE = 201,
    TARGET_CREATURE_TYPE_FIRE = 202,
    TARGET_CREATURE_TYPE_WATER = 203,
    TARGET_CREATURE_TYPE_WIND = 204,
    TARGET_CREATURE_TYPE_EARTH = 205,
    TARGET_CREATURE_TYPE_LIGHT = 206,
    TARGET_CREATURE_TYPE_DARK = 207,
};

enum SKILL_UID : uint32_t
{
    SKILL_INCREASE_ENERGY = 1082,
    SKILL_DUAL_SWORD_EXPERT = 1181,
    SKILL_ARMOR_MASTERY = 1201,
    SKILL_LIFE_OF_DARKNESS = 1033,

    SKILL_CREATURE_CONTROL = 1801,
    SKILL_CREATURE_MASTERY = 1811,
    SKILL_TECHNICAL_CREATURE_CONTROL = 1881,
    SKILL_ADV_WEAPON_EXPERT = 1022,
    SKILL_GAIA_ARMOR_MASTERY = 1202,

    SKILL_RAISE_EXP = 11002,
    SKILL_RAISE_JP = 11003,

    SKILL_GAIA_FORCE_SAVING = 2631,

    SKILL_ITEM_RESURRECTION_SCROLL = 6001,
    SKILL_ITEM_REGENERATION_SCROLL = 6002,
    SKILL_ITEM_HEALING_SCROLL = 6003,
    SKILL_ITEM_MANA_RECOVERY_SCROLL = 6004,
    SKILL_ITEM_ANTIDOTE_SCROLL = 6005,
    SKILL_ITEM_RECHARGE_SCROLL = 6006,
    SKILL_TOWN_PORTAL = 6007,
    SKILL_RETURN = 3802,
    SKILL_FORCE_CHIP = 6008,
    SKILL_SOUL_CHIP = 6009,
    SKILL_LUNA_CHIP = 6010,

    SKILL_ITEM_PERFECT_CREATURE_RESURRECTION_SCROLL = 6013,
    SKILL_ITEM_PIECE_OF_STRENGTH = 6014,
    SKILL_ITEM_PIECE_OF_VITALITY = 6015,
    SKILL_ITEM_PIECE_OF_DEXTERITY = 6016,
    SKILL_ITEM_PIECE_OF_AGILITY = 6017,
    SKILL_ITEM_PIECE_OF_INTELLIGENCE = 6018,
    SKILL_ITEM_PIECE_OF_MENTALITY = 6019,

    SKILL_RETURN_FEATHER = 6020,
    SKILL_RETURN_BACK_FEATHER = 6021,

    SKILL_FIRE_BOMB_PHYSICAL = 64809,
    SKILL_FIRE_BOMB_MAGICAL = 64810,

    SKILL_ITEM_REGENERATION_SCROLL_LV1 = 6022,
    SKILL_ITEM_REGENERATION_SCROLL_LV2 = 6023,
    SKILL_ITEM_REGENERATION_SCROLL_LV3 = 6024,
    SKILL_ITEM_REGENERATION_SCROLL_LV4 = 6025,
    SKILL_ITEM_REGENERATION_SCROLL_LV5 = 6026,
    SKILL_ITEM_REGENERATION_SCROLL_LV6 = 6027,
    SKILL_ITEM_REGENERATION_SCROLL_LV7 = 6028,
    SKILL_ITEM_REGENERATION_SCROLL_LV8 = 6029,
    SKILL_ITEM_HEALING_SCROLL_LV1 = 6030,
    SKILL_ITEM_HEALING_SCROLL_LV2 = 6031,
    SKILL_ITEM_HEALING_SCROLL_LV3 = 6032,
    SKILL_ITEM_HEALING_SCROLL_LV4 = 6033,
    SKILL_ITEM_HEALING_SCROLL_LV5 = 6034,
    SKILL_ITEM_HEALING_SCROLL_LV6 = 6035,
    SKILL_ITEM_HEALING_SCROLL_LV7 = 6036,
    SKILL_ITEM_HEALING_SCROLL_LV8 = 6037,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV1 = 6038,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV2 = 6039,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV3 = 6040,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV4 = 6041,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV5 = 6042,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV6 = 6043,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV7 = 6044,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV8 = 6045,
    SKILL_CALL_BLACK_PINE_TEA = 6046,

    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV1 = 6047,
    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV2 = 6048,
    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV3 = 6049,

    SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH = 6061,
    SKILL_ITEM_ALTERED_PIECE_OF_VITALITY = 6062,
    SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY = 6063,
    SKILL_ITEM_ALTERED_PIECE_OF_AGILITY = 6064,
    SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE = 6065,
    SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY = 6066,
    SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH_QUEST = 6067,
    SKILL_ITEM_ALTERED_PIECE_OF_VITALITY_QUEST = 6068,
    SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY_QUEST = 6069,
    SKILL_ITEM_ALTERED_PIECE_OF_AGILITY_QUEST = 6070,
    SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE_QUEST = 6071,
    SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY_QUEST = 6072,

    SKILL_COLLECTING = 6901,
    SKILL_MINING = 6902,
    SKILL_OPERATING = 6903,
    SKILL_ACTIVATING = 6904,
    SKILL_OPERATE_DEVICE = 6905,
    SKILL_OPERATE_DUNGEON_CORE = 6906,
    SKILL_OPERATE_EXPLORATION = 6909,

    SKILL_TREE_OF_HEALING_TYPE_A = 64806,
    SKILL_TREE_OF_HEALING_TYPE_B = 64807,

    SKILL_SHOVELING = 6907,
    SKILL_PET_TAMING = 6908,

    SKILL_CREATURE_TAMING = 4003,

    SKILL_THROW_SMALL_SNOWBALL = 10009,
    SKILL_THROW_BIG_SNOWBALL = 10010,
    SKILL_THROW_MARBLE = 10014,

    SKILL_CREATURE_RIDING = 11001,

    SKILL_UNIT_EXPERT_LV2 = 12001,
    SKILL_UNIT_EXPERT_LV3 = 12002,
    SKILL_UNIT_EXPERT_LV4 = 12003,
    SKILL_UNIT_EXPERT_LV5 = 12004,
    SKILL_UNIT_EXPERT_LV6 = 12005,

    SKILL_AMORY_UNIT = 14001,

    SKILL_TWIN_BLADE_EXPERT = 61010,
    SKILL_TWIN_AXE_EXPERT = 61015,

    SKILL_NEW_YEAR_CHAMPAGNE = 64808,

    SKILL_NAMUIR_LEAF_POISON = 64813,
    SKILL_NAMUIR_RIND_BLEEDING = 64814,

    SKILL_TREE_OF_HEALING_ON_DEATHMATCH = 64815,
    SKILL_RANKED_DEATHMATCH_ENTER = 64816,
    SKILL_FREED_DEATHMATCH_ENTER = 64817,
    SKILL_WARP_TO_HUNTAHOLIC_LOBBY = 64818,
    SKILL_INSTANCE_GAME_EXIT = 64827,
};

enum SKILL_EFFECT_TYPE : int
{
    EF_MISC = 0,

    EF_RESPAWN_MONSTER_NEAR = 2,

    EF_PARAMETER_INC = 3,
    EF_PARAMETER_AMP = 4,

    EF_RESPAWN_MONSTER_RANDOMLY = 5,
    EF_RESPAWN_MONSTER_WITH_DIFF_CODE = 6,

    EF_PHYSICAL_SINGLE_DAMAGE_T1 = 101,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T1 = 102,
    EF_PHYSICAL_SINGLE_DAMAGE_T2 = 103,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T2 = 104,
    EF_PHYSICAL_DIRECTIONAL_DAMAGE = 105,
    EF_PHYSICAL_SINGLE_DAMAGE_T3 = 106,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T3 = 107,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK_OLD = 108,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_OLD = 111,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD = 112,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE_OLD = 113,
    EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD = 117,
    EF_PHYSICAL_ABSORB_DAMAGE = 121,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_OLD = 122,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY_OLD = 125,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD = 131,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_OLD = 132,
    EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK = 151,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD = 152,

    EF_MAGIC_SINGLE_DAMAGE_T1_OLD = 201,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_OLD = 202,
    EF_MAGIC_SINGLE_DAMAGE_T2_OLD = 203,
    EF_MAGIC_MULTIPLE_DAMAGE_T2_OLD = 204,
    EF_MAGIC_MULTIPLE_DAMAGE_T3_OLD = 205,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD = 206,
    EF_MAGIC_SINGLE_REGION_DAMAGE_OLD = 211,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_OLD = 212,
    EF_MAGIC_SPECIAL_REGION_DAMAGE_OLD = 213,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_T2_OLD = 214,
    EF_MAGIC_ABSORB_DAMAGE_OLD = 221,

    EF_MAGIC_SINGLE_DAMAGE = 231,
    EF_MAGIC_MULTIPLE_DAMAGE = 232,
    EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP = 233,
    EF_MAGIC_SINGLE_DAMAGE_OR_DEATH = 234,
    EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP = 235,
    EF_MAGIC_SINGLE_PERCENT_DAMAGE = 236,
    EF_MAGIC_SINGLE_PERCENT_MANABURN = 237,
    EF_MAGIC_SINGLE_PERCENT_OF_MAX_MP_MANABURN = 238,
    EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE = 239,
    EF_MAGIC_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE = 240,
    EF_MAGIC_MULTIPLE_DAMAGE_AT_ONCE = 241,

    EF_MAGIC_SINGLE_REGION_DAMAGE = 261,
    EF_MAGIC_SPECIAL_REGION_DAMAGE = 262,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE = 263,
    EF_MAGIC_REGION_PERCENT_DAMAGE = 264,
    EF_MAGIC_SINGLE_REGION_DAMAGE_USING_CORPSE = 265,

    EF_ADD_HP_MP_BY_ABSORB_HP_MP = 266,

    EF_MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD = 267,

    EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE = 268,

    EF_MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE = 269,

    EF_AREA_EFFECT_MAGIC_DAMAGE = 271,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL = 272,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2 = 273,

    EF_ADD_STATE = 301,
    EF_ADD_REGION_STATE = 302,

    EF_CASTING_CANCEL_WITH_ADD_STATE = 304,

    EF_ADD_STATE_BY_SELF_COST = 305,
    EF_ADD_REGION_STATE_BY_SELF_COST = 306,
    EF_ADD_STATE_BY_TARGET_TYPE = 307,

    EF_ADD_STATES_WITH_EACH_DIFF_LV = 308,
    EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION = 309,
    EF_ADD_STATE_STEP_BY_STEP = 310,
    EF_ADD_STATE_TO_CASTER_AND_TARGET = 311,
    EF_ADD_RANDOM_STATE = 312,
    EF_ADD_RANDOM_REGION_STATE = 313,
    EF_ADD_STATE_BY_USING_ITEM = 314,

    EF_AREA_EFFECT_MAGIC_DAMAGE_OLD = 352,
    EF_AREA_EFFECT_HEAL = 353,

    EF_TRAP_PHYSICAL_DAMAGE = 381,
    EF_TRAP_MAGICAL_DAMAGE = 382,
    EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE = 383,
    EF_TRAP_MULTIPLE_MAGICAL_DAMAGE = 384,

    EF_REMOVE_BAD_STATE = 401,
    EF_REMOVE_GOOD_STATE = 402,
    EF_ADD_HP = 501,
    EF_ADD_MP = 502,
    EF_RESURRECTION = 504,
    EF_ADD_HP_MP = 505,
    EF_ADD_HP_MP_BY_SUMMON_DAMAGE = 506,
    EF_ADD_HP_MP_BY_SUMMON_DEAD = 507,
    EF_ADD_REGION_HP_MP = 508,
    EF_ADD_HP_BY_ITEM = 509,
    EF_ADD_MP_BY_ITEM = 510,
    EF_CORPSE_ABSORB = 511,
    EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP = 512,
    EF_ADD_HP_MP_WITH_LIMIT_PERCENT = 513,

    EF_ADD_REGION_HP = 521,
    EF_ADD_REGION_MP = 522,

    EF_SUMMON = 601,
    EF_UNSUMMON = 602,
    EF_UNSUMMON_AND_ADD_STATE = 605,

    EF_TOGGLE_AURA = 701,
    EF_TOGGLE_DIFFERENTIAL_AURA = 702,

    EF_TAUNT = 900,
    EF_REGION_TAUNT = 901,
    EF_REMOVE_HATE = 902,
    EF_REGION_REMOVE_HATE = 903,
    EF_REGION_REMOVE_HATE_OF_TARGET = 904,

    EF_CORPSE_EXPLOSION = 1001,

    EF_CREATE_ITEM = 9001,
    EF_ACTIVATE_FIELD_PROP = 9501,
    EF_REGION_HEAL_BY_FIELD_PROP = 9502,
    EF_AREA_EFFECT_HEAL_BY_FIELD_PROP = 9503,

    EF_WEAPON_MASTERY = 10001,
    EF_BATTLE_PARAMTER_INCREASE = 10002,
    EF_BLOCK_INCREASE = 10003,
    EF_ATTACK_RANGE_INCREASE = 10004,
    EF_RESISTANCE_INCREASE = 10005,
    EF_MAGIC_REGISTANCE_INCREASE = 10006,
    EF_SPECIALIZE_ARMOR = 10007,
    EF_INCREASE_BASE_ATTRIBUTE = 10008,
    EF_INCREASE_EXTENSION_ATTRIBUTE = 10009,

    EF_SPECIALIZE_ARMOR_AMP = 10010,
    EF_AMPLIFY_BASE_ATTRIBUTE = 10011,

    EF_MAGIC_TRAINING = 10012,
    EF_HUNTING_TRAINING = 10013,
    EF_BOW_TRAINING = 10014,
    EF_INCREASE_STAT = 10015,
    EF_AMPLIFY_STAT = 10016,

    EF_INCREASE_HP_MP = 10021,
    EF_AMPLIFY_HP_MP = 10022,
    EF_HEALING_AMPLIFY = 10023,
    EF_HEALING_AMPLIFY_BY_ITEM = 10024,
    EF_HEALING_AMPLIFY_BY_REST = 10025,
    EF_HATE_AMPLIFY = 10026,

    EF_INCREASE_SUMMON_HP_MP_SP = 10031,
    EF_AMPLIFY_SUMMON_HP_MP_SP = 10032,
    EF_BELT_ON_PARAMETER_INC = 10035,
    EF_BELT_ON_ATTRIBUTE_INC = 10036,
    EF_BELT_ON_ATTRIBUTE_EX_INC = 10037,
    EF_BELT_ON_ATTRIBUTE_EX2_INC = 10038,

    EF_UNIT_EXPERT = 10041,

    EF_BELT_ON_PARAMETER_AMP = 10042,
    EF_BELT_ON_ATTRIBUTE_AMP = 10043,
    EF_BELT_ON_ATTRIBUTE_EX_AMP = 10044,
    EF_BELT_ON_ATTRIBUTE_EX2_AMP = 10045,

    EF_SUMMON_ITEM_EXPERT = 10046,

    EF_ADD_STATE_ON_ATTACK = 10048,
    EF_ADD_STATE_BY_SELF_ON_ATTACK = 10049,
    EF_ADD_STATE_ON_BEING_ATTACKED = 10050,
    EF_ADD_STATE_BY_SELF_ON_BEING_ATTACKED = 10051,
    EF_ADD_STATE_BY_SELF_ON_KILL = 10052,
    EF_ADD_STATE_ON_CRITICAL_ATTACK = 10053,
    EF_ADD_STATE_BY_SELF_ON_CRITICAL_ATTACK = 10054,
    EF_ADD_STATE_ON_BEING_CRITICAL_ATTACKED = 10055,
    EF_ADD_STATE_BY_SELF_ON_BEING_CRITICAL_ATTACKED = 10056,
    EF_ADD_STATE_ON_AVOID = 10057,
    EF_ADD_STATE_BY_SELF_ON_AVOID = 10058,
    EF_ADD_STATE_ON_BLOCK = 10059,
    EF_ADD_STATE_BY_SELF_ON_BLOCK = 10060,
    EF_ADD_STATE_ON_PERFECT_BLOCK = 10061,
    EF_ADD_STATE_BY_SELF_ON_PERFECT_BLOCK = 10062,
    EF_ADD_ENERGY_ON_ATTACK = 32262,
    EF_ADD_ENERGY_ON_BEING_ATTACKED = 32263,
    EF_INC_SKILL_COOL_TIME_ON_ATTACK = 10063,
    EF_INC_SKILL_COOL_TIME_ON_BEING_ATTACKED = 10064,
    EF_INC_SKILL_COOL_TIME_ON_KILL = 10065,
    EF_INC_SKILL_COOL_TIME_ON_CRITICAL_ATTACK = 10066,
    EF_INC_SKILL_COOL_TIME_ON_BEING_CRITICAL_ATTACKED = 10067,
    EF_INC_SKILL_COOL_TIME_ON_AVOID = 10068,
    EF_INC_SKILL_COOL_TIME_ON_BLOCK = 10069,
    EF_INC_SKILL_COOL_TIME_ON_PERFECT_BLOCK = 10070,
    EF_INC_SKILL_COOL_TIME_ON_SKILL_OF_ID = 32281,

    EF_PHYSICAL_SINGLE_DAMAGE = 30001,
    EF_PHYSICAL_SINGLE_DAMAGE_ABSORB = 30002,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY = 30003,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH = 30004,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK = 30005,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK = 30006,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK = 30007,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF = 30008,

    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE = 30009,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK = 30010,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE = 30011,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE = 30012,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE = 30013,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE = 30014,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF = 30015,
    EF_PHYSICAL_MULTIPLE_DAMAGE = 30016,
    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK = 30017,
    EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE = 30018,
    EF_PHYSICAL_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE = 30019,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_ADDING_MAGICAL_DAMAGE = 30020,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL = 30030,

    EF_RESURRECTION_WITH_RECOVER = 30501,
    EF_REMOVE_STATE_GROUP = 30601,
    EF_LOTTO = 30701,

    EF_WEAPON_TRAINING = 31001,
    EF_AMPLIFY_BASE_ATTRIBUTE_OLD = 31002,
    EF_AMPLIFY_EXT_ATTRIBUTE = 31003,

    EF_AMPLIFY_EXP_FOR_SUMMON = 32001,

    EF_ENHANCE_SKILL = 32011,

    EF_MAGIC_SINGLE_DAMAGE_WITH_PHYSICAL_DAMAGE = 32021,

    EF_INC_DAMAGE_BY_TARGET_STATE = 32031,
    EF_AMP_DAMAGE_BY_TARGET_STATE = 32032,
    EF_TRANFER_HEALING = 32051,
    EF_PHYSICAL_CHAIN_DAMAGE = 32061,
    EF_MAGIC_CHAIN_DAMAGE = 32062,
    EF_CHAIN_HEAL = 32063,
    EF_PHYSICAL_SINGLE_DAMAGE_DEMINISHED_HP_MP = 32141,
    EF_MODIFY_SKILL_COST = 32171,
    EF_RESIST_HARMFUL_STATE = 32183,
    EF_INC_SKILL_COOL_TIME = 32191,
    EF_AMP_SKILL_COOL_TIME = 32192,
    EF_INC_DAMAGE_INC_CRIT_RATE_BY_TARGET_HP_RATIO = 32201,
    EF_AMP_DAMAGE_INC_CRIT_RATE_BY_TARGET_HP_RATIO = 32202,
    EF_ABSORB_DAMAGE = 32211,
    EF_STEAL_HP_MP = 32212,
    EF_PHYSICAL_SINGLE_DAMAGE_PROP_REMAIN_MP = 32251,
    EF_REPLENISH_ENERGY_HP_MP = 32261,
    EF_INCREASE_ENERGY_UNCONSUMPTION_RATE = 32264,
    EF_INC_PARAM_AMPLIFY_HEAL = 32271,
    EF_AMP_PARAM_AMPLIFY_HEAL = 32272,

    EF_INC_PARAM_BY_STATE = 32291,
    EF_AMP_PARAM_BY_STATE = 32292,

    EF_INC_PARAM_BASED_PARAM = 32301,
    EF_INC_SUMMON_PARAM_BASED_PARAM = 32302,
    EF_INC_SUMMON_PARAM_BASED_SUMMON_PARAM = 32303,
    EF_INC_PARAM_BASED_SUMMON_PARAM = 32304,
};

enum SKILL_CATEGORY
{
    SC_PHYSICAL = 1,
    SC_MAGICAL = 2,
    SC_EVERY = 99,
};

struct SkillTreeBase
{
    int32_t job_id{};
    int32_t skill_id{};
    int32_t min_skill_lv{};
    int32_t max_skill_lv{};
    int32_t lv{};
    int32_t job_lv{};
    float jp_ratio{};
    int32_t need_skill_id[3]{};
    int32_t need_skill_lv[3]{};
};

struct SkillTreeGroup
{
    int32_t job_id{};
    int32_t skill_id{};
    std::vector<SkillTreeBase> skillTrees{};
};

struct SkillBase
{
    enum
    {
        USE_SELF = 0,
        USE_ALLY,
        USE_GUILD,
        USE_NEUTRAL,
        USE_PURPLE,
        USE_ENEMY,
    };

    int32_t GetID() const;
    int32_t GetNameID() const;
    bool IsValid() const;
    bool IsSystemSkill() const;

    bool IsPassive() const;
    bool IsPhysicalSkill() const;
    bool IsMagicalSkill() const;
    bool IsHarmful() const;
    bool IsNeedTarget() const;
    bool IsValidToCorpse() const;
    bool IsToggle() const;

    int32_t GetCriticalBonus(int32_t skill_lv) const;
    int32_t GetCastRange() const;
    int32_t GetValidRange() const;

    int32_t GetToggleGroup() const;
    int32_t GetSkillTargetType() const;
    int32_t GetSkillEffectType() const;
    int32_t GetElementalType() const;

    int32_t GetCostEXP(int32_t skill_lv, int32_t enhance) const;
    int32_t GetCostJP(int32_t skill_lv, int32_t enhance) const;
    int32_t GetProbabilityOnHit(int32_t slv) const;
    int32_t GetCostItemCode() const;
    int64_t GetCostItemCount(int32_t skill_lv) const;
    int32_t GetCostHP(int32_t skill_lv) const;
    float GetCostHPPercent(int32_t skill_lv) const;
    int32_t GetCostMP(int32_t skill_lv, int32_t enhance) const;
    float GetCostMPPercent(int32_t skill_lv) const;
    int32_t GetCostHavoc(int32_t skill_lv) const;
    int32_t GetCostEnergy(int32_t skill_lv) const;
    int32_t GetCostItem() const;
    int32_t GetNeedLevel() const;
    int32_t GetNeedHP() const;
    int32_t GetNeedMP() const;
    int32_t GetNeedHavoc() const;
    int32_t GetNeedHavocBurst() const;
    int32_t GetNeedStateId() const;
    uint8_t GetNeedStateLevel() const;
    bool NeedStateExhaust() const;

    bool IsUseableOnAvatar() const;
    bool IsUseableOnMonster() const;
    bool IsUseableOnSummon() const;

    int32_t GetHatePoint(int32_t lv, int32_t point, int32_t enhance) const;

    bool IsNeedShield() const;
    bool IsNeedWeapon() const;

    int32_t GetNeedJobPoint(int32_t skill_lv);
    bool IsUseableWeapon(ItemClass cl);
    int32_t GetStateSecond(int32_t skill_lv, int32_t enhance_lv);
    int32_t GetStateType() const;
    int32_t GetHitBonus(int32_t enhance, int32_t level_diff) const;
    int32_t GetStateId() const;
    int32_t GetStateLevel(int32_t skill_lv, int32_t enhance_lv);
    uint32_t GetCastDelay(int32_t skill_lv, int32_t enhance);
    uint32_t GetCommonDelay() const;
    uint32_t GetCoolTime(int32_t enhance) const;
    uint32_t GetFireRange() const;
    bool IsUsable(uint8_t nUseIndex) const;
    int32_t GetCostEnergy(uint8_t skill_lv) const;

    int32_t m_need_jp[50]{};

    int32_t id{};
    int32_t text_id{};
    short is_valid{};
    uint8_t elemental{};
    uint8_t is_passive{};
    uint8_t is_physical_act{};
    uint8_t is_harmful{};
    uint8_t is_need_target{};
    uint8_t is_corpse{};
    uint8_t is_toggle{};
    int32_t toggle_group{};
    uint8_t casting_type{};
    uint8_t casting_level{};
    int32_t cast_range{};
    int32_t valid_range{};
    int32_t cost_hp{};
    int32_t cost_hp_per_skl{};
    int32_t cost_mp{};
    int32_t cost_mp_per_skl{};
    int32_t cost_mp_per_enhance{};
    float cost_hp_per{};
    float cost_hp_per_skl_per{};
    float cost_mp_per{};
    float cost_mp_per_skl_per{};
    int32_t cost_havoc{};
    int32_t cost_havoc_per_skl{};
    float cost_energy{};
    float cost_energy_per_skl{};
    int32_t cost_exp{};
    int32_t cost_exp_per_enhance{};
    int32_t cost_jp{};
    int32_t cost_jp_per_enhance{};
    int32_t cost_item{};
    int32_t cost_item_count{};
    int32_t cost_item_count_per{};
    int32_t need_level{};
    int32_t need_hp{};
    int32_t need_mp{};
    int32_t need_havoc{};
    int32_t need_havoc_burst{};
    int32_t need_state_id{};
    short need_state_level{};
    short need_state_exhaust{};
    uint8_t vf_one_hand_sword{};
    uint8_t vf_two_hand_sword{};
    uint8_t vf_double_sword{};
    uint8_t vf_dagger{};
    uint8_t vf_double_dagger{};
    uint8_t vf_spear{};
    uint8_t vf_axe{};
    uint8_t vf_one_hand_axe{};
    uint8_t vf_double_axe{};
    uint8_t vf_one_hand_mace{};
    uint8_t vf_two_hand_mace{};
    uint8_t vf_lightbow{};
    uint8_t vf_heavybow{};
    uint8_t vf_crossbow{};
    uint8_t vf_one_hand_staff{};
    uint8_t vf_two_hand_staff{};
    uint8_t vf_shield_only{};
    uint8_t vf_is_not_need_weapon{};
    float delay_cast{};
    float delay_cast_per_skl{};
    float delay_cast_mode_per{};
    float delay_common{};
    float delay_cooltime{};
    float delay_cooltime_mode{};
    int32_t cool_time_group_id{};
    uint8_t uf_self{};
    uint8_t uf_party{};
    uint8_t uf_guild{};
    uint8_t uf_neutral{};
    uint8_t uf_purple{};
    uint8_t uf_enemy{};
    uint8_t tf_avatar{};
    uint8_t tf_summon{};
    uint8_t tf_monster{};
    short target{};
    short effect_type{};
    int32_t state_id{};
    int32_t state_level_base{};
    float state_level_per_skl{};
    float state_level_per_enhance{};
    float state_second{};
    float state_second_per_level{};
    float state_second_per_enhance{};
    uint8_t state_type{};
    int32_t probability_on_hit{};
    int32_t probability_inc_by_slv{};
    short hit_bonus{};
    short hit_bonus_per_enhance{};
    short percentage{};
    float hate_mod{};
    short hate_basic{};
    float hate_per_skl{};
    float hate_per_enhance{};
    int32_t critical_bonus{};
    int32_t critical_bonus_per_skl{};
    float var[20]{};
    short is_projectile{};
    float projectile_speed{};
    float projectile_acceleration{};
};