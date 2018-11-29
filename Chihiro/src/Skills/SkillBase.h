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
#include "TS_SC_SKILL.h"

enum SKILL_TYPE : int16_t
{
    ST_INVALID = 0,
    ST_VALID   = 1,
    ST_SYSTEM  = 2,
};

enum SKILL_EFFECT_TARGET_LIMIT : int
{
    SKILL_EFFECT_TARGET_LIMIT_ANY_BODY   = 0,
    SKILL_EFFECT_TARGET_LIMIT_NOT_ENEMY  = 1,
    SKILL_EFFECT_TARGET_LIMIT_ONLY_ALLY  = 2,
    SKILL_EFFECT_TARGET_LIMIT_ONLY_ENEMY = 3,
};

enum REGION_TYPE
{
    REGION_TYPE_DIRECTION  = 0,
    REGION_TYPE_ARC_CIRCLE = 1,
    REGION_TYPE_CROSS      = 2,
};

enum DISTRIBUTION_TYPE
{
    DISTRIBUTION_TYPE_NO_LIMIT          = 0,
    DISTRIBUTION_TYPE_DISTRIBUTE        = 1,
    DISTRIBUTION_TYPE_RANDOM            = 2,
    DISTRIBUTION_TYPE_SEQUENTIAL_TARGET = 3,
    DISTRIBUTION_TYPE_SEQUENTIAL_CASTER = 4,
};

struct SkillResult
{
    uint8 type;
    uint  hTarget;

    TS_SC_SKILL__HIT_DAMAGE                hitDamage;
    TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK hitDamageWithKnockBack;
    TS_SC_SKILL__HIT_RESULT                hitResult;
    TS_SC_SKILL__HIT_ADD_STAT              hitAddStat;
    TS_SC_SKILL__HIT_ADDHPMPSP             hitAddHPMPSP;
    TS_SC_SKILL__HIT_REBIRTH               hitRebirth;
    TS_SC_SKILL__HIT_RUSH                  hitRush;
};

enum TARGET_TYPE : int
{
    TARGET_MISC                                   = 0,
    TARGET_TARGET                                 = 1,
    TARGET_REGION_WITH_TARGET                     = 2,
    TARGET_REGION_WITHOUT_TARGET                  = 3,
    TARGET_REGION                                 = 4,
    TARGET_TARGET_EXCEPT_CASTER                   = 6,
    TARGET_PARTY                                  = 21,
    TARGET_GUILD                                  = 22,
    TARGET_ATTACKTEAM                             = 23,
    TARGET_SUMMON                                 = 31,
    TARGET_PARTY_SUMMON                           = 32,
    TARGET_REGION_NEAR_MAIN_SUMMON_WITHOUT_TARGET = 44,
    TARGET_SELF_WITH_SUMMON                       = 45,
    TARGET_PARTY_WITH_SUMMON                      = 51,
    TARGET_MASTER                                 = 101,
    TARGET_SELF_WITH_MASTER                       = 102,
    TARGET_CREATURE_TYPE_NONE                     = 201,
    TARGET_CREATURE_TYPE_FIRE                     = 202,
    TARGET_CREATURE_TYPE_WATER                    = 203,
    TARGET_CREATURE_TYPE_WIND                     = 204,
    TARGET_CREATURE_TYPE_EARTH                    = 205,
    TARGET_CREATURE_TYPE_LIGHT                    = 206,
    TARGET_CREATURE_TYPE_DARK                     = 207,
};

enum SKILL_UID : uint
{
    SKILL_INCREASE_ENERGY                           = 0x43A,
    SKILL_DUAL_SWORD_EXPERT                         = 0x49D,
    SKILL_ARMOR_MASTERY                             = 0x4B1,
    SKILL_LIFE_OF_DARKNESS                          = 0x409,
    SKILL_CREATURE_CONTROL                          = 0x709,
    SKILL_CREATURE_MASTERY                          = 0x713,
    SKILL_TECHNICAL_CREATURE_CONTROL                = 0x759,
    SKILL_ADV_WEAPON_EXPERT                         = 0x3FE,
    SKILL_GAIA_ARMOR_MASTERY                        = 0x4B2,
    SKILL_RAISE_EXP                                 = 0x2AFA,
    SKILL_RAISE_JP                                  = 0x2AFB,
    SKILL_GAIA_FORCE_SAVING                         = 0xA47,
    SKILL_ITEM_RESURRECTION_SCROLL                  = 0x1771,
    SKILL_ITEM_REGENERATION_SCROLL                  = 0x1772,
    SKILL_ITEM_HEALING_SCROLL                       = 0x1773,
    SKILL_ITEM_MANA_RECOVERY_SCROLL                 = 0x1774,
    SKILL_ITEM_ANTIDOTE_SCROLL                      = 0x1775,
    SKILL_ITEM_RECHARGE_SCROLL                      = 0x1776,
    SKILL_TOWN_PORTAL                               = 0x1777,
    SKILL_RETURN                                    = 0xEDA,
    SKILL_FORCE_CHIP                                = 0x1778,
    SKILL_SOUL_CHIP                                 = 0x1779,
    SKILL_LUNA_CHIP                                 = 0x177A,
    SKILL_ITEM_PERFECT_CREATURE_RESURRECTION_SCROLL = 0x177D,
    SKILL_ITEM_PIECE_OF_STRENGTH                    = 0x177E,
    SKILL_ITEM_PIECE_OF_VITALITY                    = 0x177F,
    SKILL_ITEM_PIECE_OF_DEXTERITY                   = 0x1780,
    SKILL_ITEM_PIECE_OF_AGILITY                     = 0x1781,
    SKILL_ITEM_PIECE_OF_INTELLIGENCE                = 0x1782,
    SKILL_ITEM_PIECE_OF_MENTALITY                   = 0x1783,
    SKILL_RETURN_FEATHER                            = 0x1784,
    SKILL_RETURN_BACK_FEATHER                       = 0x1785,
    SKILL_FIRE_BOMB_PHYSICAL                        = 0xFFFFFD29,
    SKILL_FIRE_BOMB_MAGICAL                         = 0xFFFFFD2A,
    SKILL_ITEM_REGENERATION_SCROLL_LV1              = 0x1786,
    SKILL_ITEM_REGENERATION_SCROLL_LV2              = 0x1787,
    SKILL_ITEM_REGENERATION_SCROLL_LV3              = 0x1788,
    SKILL_ITEM_REGENERATION_SCROLL_LV4              = 0x1789,
    SKILL_ITEM_REGENERATION_SCROLL_LV5              = 0x178A,
    SKILL_ITEM_REGENERATION_SCROLL_LV6              = 0x178B,
    SKILL_ITEM_REGENERATION_SCROLL_LV7              = 0x178C,
    SKILL_ITEM_REGENERATION_SCROLL_LV8              = 0x178D,
    SKILL_ITEM_HEALING_SCROLL_LV1                   = 0x178E,
    SKILL_ITEM_HEALING_SCROLL_LV2                   = 0x178F,
    SKILL_ITEM_HEALING_SCROLL_LV3                   = 0x1790,
    SKILL_ITEM_HEALING_SCROLL_LV4                   = 0x1791,
    SKILL_ITEM_HEALING_SCROLL_LV5                   = 0x1792,
    SKILL_ITEM_HEALING_SCROLL_LV6                   = 0x1793,
    SKILL_ITEM_HEALING_SCROLL_LV7                   = 0x1794,
    SKILL_ITEM_HEALING_SCROLL_LV8                   = 0x1795,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV1             = 0x1796,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV2             = 0x1797,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV3             = 0x1798,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV4             = 0x1799,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV5             = 0x179A,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV6             = 0x179B,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV7             = 0x179C,
    SKILL_ITEM_MANA_RECOVERY_SCROLL_LV8             = 0x179D,
    SKILL_CALL_BLACK_PINE_TEA                       = 0x179E,
    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV1           = 0x179F,
    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV2           = 0x17A0,
    SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV3           = 0x17A1,
    SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH            = 0x17AD,
    SKILL_ITEM_ALTERED_PIECE_OF_VITALITY            = 0x17AE,
    SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY           = 0x17AF,
    SKILL_ITEM_ALTERED_PIECE_OF_AGILITY             = 0x17B0,
    SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE        = 0x17B1,
    SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY           = 0x17B2,
    SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH_QUEST      = 0x17B3,
    SKILL_ITEM_ALTERED_PIECE_OF_VITALITY_QUEST      = 0x17B4,
    SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY_QUEST     = 0x17B5,
    SKILL_ITEM_ALTERED_PIECE_OF_AGILITY_QUEST       = 0x17B6,
    SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE_QUEST  = 0x17B7,
    SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY_QUEST     = 0x17B8,
    SKILL_ITEM_ALTERED_MAX_LIFE_PIECE               = 0x17BF,
    SKILL_ITEM_ALTERED_MAX_MANA_PIECE               = 0x17C0,
    SKILL_ITEM_ALTERED_AGILENESS_PIECE              = 0x17C1,
    SKILL_ITEM_ALTERED_GUARDIAN_PIECE               = 0x17C2,
    SKILL_ITEM_ALTERED_QUICKNESS_PIECE              = 0x17C3,
    SKILL_ITEM_ALTERED_IMPREGNABLENESS_PIECE        = 0x17C4,
    SKILL_ITEM_ALTERED_CAREFULNESS_PIECE            = 0x17C5,
    SKILL_ITEM_ALTERED_FATALNESS_PIECE              = 0x17C6,
    SKILL_ITEM_ALTERED_MANA_PIECE                   = 0x17C7,
    SKILL_ITEM_ALTERED_SHARPNESS_PIECE              = 0x17C8,
    SKILL_ITEM_ALTERED_SPELL_PIECE                  = 0x17C9,
    SKILL_ITEM_ALTERED_BRILLIANCE_PIECE             = 0x17CA,
    SKILL_ITEM_ALTERED_INSIGHT_PIECE                = 0x17CB,
    SKILL_COLLECTING                                = 0x1AF5,
    SKILL_MINING                                    = 0x1AF6,
    SKILL_OPERATING                                 = 0x1AF7,
    SKILL_ACTIVATING                                = 0x1AF8,
    SKILL_OPERATE_DEVICE                            = 0x1AF9,
    SKILL_OPERATE_DUNGEON_CORE                      = 0x1AFA,
    SKILL_OPERATE_EXPLORATION                       = 0x1AFD,
    SKILL_TREE_OF_HEALING_TYPE_A                    = 0xFFFFFD26,
    SKILL_TREE_OF_HEALING_TYPE_B                    = 0xFFFFFD27,
    SKILL_SHOVELING                                 = 0x1AFB,
    SKILL_PET_TAMING                                = 0x1AFC,
    SKILL_CREATURE_TAMING                           = 0xFA3,
    SKILL_THROW_SMALL_SNOWBALL                      = 0x2719,
    SKILL_THROW_BIG_SNOWBALL                        = 0x271A,
    SKILL_THROW_MARBLE                              = 0x271E,
    SKILL_THROW_RED_TOMATO                          = 0x271F,
    SKILL_THROW_GREEN_TOMATO                        = 0x2720,
    SKILL_CREATURE_RIDING                           = 0x2AF9,
    SKILL_UNIT_EXPERT_LV2                           = 0x2EE1,
    SKILL_UNIT_EXPERT_LV3                           = 0x2EE2,
    SKILL_UNIT_EXPERT_LV4                           = 0x2EE3,
    SKILL_UNIT_EXPERT_LV5                           = 0x2EE4,
    SKILL_UNIT_EXPERT_LV6                           = 0x2EE5,
    SKILL_AMORY_UNIT                                = 0x36B1,
    SKILL_RULER_OF_TIME                             = 0x7A5C,
    SKILL_FRIENDSHIP_OF_CROWN                       = 0xFFFFB7B3,
    SKILL_GRACE                                     = 0xFFFFC4E1,
    SKILL_TWIN_BLADE_EXPERT                         = 0xFFFFEE52,
    SKILL_TWIN_AXE_EXPERT                           = 0xFFFFEE57,
    SKILL_NEW_YEAR_CHAMPAGNE                        = 0xFFFFFD28,
    SKILL_NAMUIR_LEAF_POISON                        = 0xFFFFFD2D,
    SKILL_NAMUIR_RIND_BLEEDING                      = 0xFFFFFD2E,
    SKILL_TREE_OF_HEALING_ON_DEATHMATCH             = 0xFFFFFD2F,
    SKILL_RANKED_DEATHMATCH_ENTER                   = 0xFFFFFD30,
    SKILL_FREED_DEATHMATCH_ENTER                    = 0xFFFFFD31,
    SKILL_WARP_TO_HUNTAHOLIC_LOBBY                  = 0xFFFFFD32,
    SKILL_INSTANCE_GAME_EXIT                        = 0xFFFFFD3B,
    SKILL_ITEM_RESPAWN_SCROLL_RANDOMLY1             = 0xFFFFFE24,
    SKILL_ITEM_RESPAWN_SCROLL_WITH_DIFF_CODE1       = 0xFFFFFE25,
    SKILL_ITEM_RESPAWN_SCROLL_WITH_DIFF_CODE2       = 0xFFFFFE26,
    SKILL_PROP_RESPAWN1                             = 0xFFFFFE27,
    SKILL_ITEM_REGENERATION_SCROLL_OF_VITALITY      = 0xFFFFFE4F,
    SKILL_ITEM_MANA_SCROLL_OF_VITALITY              = 0xFFFFFE50,
    SKILL_ITEM_HEALING_SCROLL_OF_VITALITY           = 0xFFFFFE51,
    SKILL_ITEM_SPELL_BREAKER_SCROLL                 = 0x259B,
    SKILL_ITEM_STONECURSE_SCROLL                    = 0x259C,
    SKILL_ITEM_REGION_STONECURSE_SCROLL             = 0x259D
};

enum SKILL_EFFECT_TYPE : int
{
    EF_MISC = 0,

    EF_RESPAWN_MONSTER_NEAR = 2,

    EF_PHYSICAL_SINGLE_DAMAGE_T1                             = 101,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T1                           = 102,
    EF_PHYSICAL_SINGLE_DAMAGE_T2                             = 103,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T2                           = 104,
    EF_PHYSICAL_DIRECTIONAL_DAMAGE                           = 105,
    EF_PHYSICAL_SINGLE_DAMAGE_T3                             = 106,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T3                           = 107,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK_OLD            = 108,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_OLD                     = 111,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD                   = 112,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE_OLD             = 113,
    EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD                    = 117,
    EF_PHYSICAL_ABSORB_DAMAGE                                = 121,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_OLD           = 122,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY_OLD                 = 125,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD                  = 131,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_OLD           = 132,
    EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK = 151,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD             = 152,

    EF_MAGIC_SINGLE_DAMAGE_T1_OLD                  = 201,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_OLD                = 202,
    EF_MAGIC_SINGLE_DAMAGE_T2_OLD                  = 203,
    EF_MAGIC_MULTIPLE_DAMAGE_T2_OLD                = 204,
    EF_MAGIC_MULTIPLE_DAMAGE_T3_OLD                = 205,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD = 206,
    EF_MAGIC_SINGLE_REGION_DAMAGE_OLD              = 211,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_OLD            = 212,
    EF_MAGIC_SPECIAL_REGION_DAMAGE_OLD             = 213,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_T2_OLD         = 214,
    EF_MAGIC_ABSORB_DAMAGE_OLD                     = 221,

    EF_MAGIC_SINGLE_DAMAGE                            = 231,
    EF_MAGIC_MULTIPLE_DAMAGE                          = 232,
    EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP           = 233,
    EF_MAGIC_SINGLE_DAMAGE_OR_DEATH                   = 234,
    EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP                 = 235,
    EF_MAGIC_SINGLE_PERCENT_DAMAGE                    = 236,
    EF_MAGIC_SINGLE_PERCENT_MANABURN                  = 237,
    EF_MAGIC_SINGLE_PERCENT_OF_MAX_MP_MANABURN        = 238,
    EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE           = 239,
    EF_MAGIC_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE = 240,
    EF_MAGIC_MULTIPLE_DAMAGE_AT_ONCE                  = 241,

    EF_MAGIC_SINGLE_REGION_DAMAGE              = 261,
    EF_MAGIC_SPECIAL_REGION_DAMAGE             = 262,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE            = 263,
    EF_MAGIC_REGION_PERCENT_DAMAGE             = 264,
    EF_MAGIC_SINGLE_REGION_DAMAGE_USING_CORPSE = 265,

    EF_ADD_HP_MP_BY_ABSORB_HP_MP = 266,

    EF_MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD = 267,

    EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE = 268,

    EF_MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE = 269,

    EF_AREA_EFFECT_MAGIC_DAMAGE             = 271,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL    = 272,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2 = 273,

    EF_ADD_STATE        = 301,
    EF_ADD_REGION_STATE = 302,

    EF_CASTING_CANCEL_WITH_ADD_STATE = 304,

    EF_ADD_STATE_BY_SELF_COST        = 305,
    EF_ADD_REGION_STATE_BY_SELF_COST = 306,
    EF_ADD_STATE_BY_TARGET_TYPE      = 307,

    EF_ADD_STATES_WITH_EACH_DIFF_LV          = 308,
    EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION = 309,

    EF_AREA_EFFECT_MAGIC_DAMAGE_OLD = 352,
    EF_AREA_EFFECT_HEAL             = 353,

    EF_TRAP_PHYSICAL_DAMAGE          = 381,
    EF_TRAP_MAGICAL_DAMAGE           = 382,
    EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE = 383,
    EF_TRAP_MULTIPLE_MAGICAL_DAMAGE  = 384,

    EF_REMOVE_BAD_STATE                = 401,
    EF_REMOVE_GOOD_STATE               = 402,
    EF_ADD_HP                          = 501,
    EF_ADD_MP                          = 502,
    EF_RESURRECTION                    = 504,
    EF_ADD_HP_MP                       = 505,
    EF_ADD_HP_MP_BY_SUMMON_DAMAGE      = 506,
    EF_ADD_HP_MP_BY_SUMMON_DEAD        = 507,
    EF_ADD_REGION_HP_MP                = 508,
    EF_ADD_HP_BY_ITEM                  = 509,
    EF_ADD_MP_BY_ITEM                  = 510,
    EF_CORPSE_ABSORB                   = 511,
    EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP = 512,
    EF_ADD_HP_MP_WITH_LIMIT_PERCENT    = 513,

    EF_ADD_REGION_HP = 521,
    EF_ADD_REGION_MP = 522,

    EF_SUMMON                 = 601,
    EF_UNSUMMON               = 602,
    EF_UNSUMMON_AND_ADD_STATE = 605,

    EF_TOGGLE_AURA              = 701,
    EF_TOGGLE_DIFFERENTIAL_AURA = 702,

    EF_TAUNT              = 900,
    EF_REGION_TAUNT       = 901,
    EF_REMOVE_HATE        = 902,
    EF_REGION_REMOVE_HATE = 903,

    EF_CORPSE_EXPLOSION = 1001,

    EF_CREATE_ITEM                    = 9001,
    EF_ACTIVATE_FIELD_PROP            = 9501,
    EF_REGION_HEAL_BY_FIELD_PROP      = 9502,
    EF_AREA_EFFECT_HEAL_BY_FIELD_PROP = 9503,

    EF_WEAPON_MASTERY               = 10001,
    EF_BATTLE_PARAMTER_INCREASE     = 10002,
    EF_BLOCK_INCREASE               = 10003,
    EF_ATTACK_RANGE_INCREASE        = 10004,
    EF_RESISTANCE_INCREASE          = 10005,
    EF_MAGIC_REGISTANCE_INCREASE    = 10006,
    EF_SPECIALIZE_ARMOR             = 10007,
    EF_INCREASE_BASE_ATTRIBUTE      = 10008,
    EF_INCREASE_EXTENSION_ATTRIBUTE = 10009,

    EF_SPECIALIZE_ARMOR_AMP   = 10010,
    EF_AMPLIFY_BASE_ATTRIBUTE = 10011,

    EF_MAGIC_TRAINING   = 10012,
    EF_HUNTING_TRAINING = 10013,
    EF_BOW_TRAINING     = 10014,
    EF_INCREASE_STAT    = 10015,
    EF_AMPLIFY_STAT     = 10016,

    EF_INCREASE_HP_MP          = 10021,
    EF_AMPLIFY_HP_MP           = 10022,
    EF_HEALING_AMPLIFY         = 10023,
    EF_HEALING_AMPLIFY_BY_ITEM = 10024,
    EF_HEALING_AMPLIFY_BY_REST = 10025,
    EF_HATE_AMPLIFY            = 10026,

    EF_INCREASE_SUMMON_HP_MP_SP      = 10031,
    EF_AMPLIFY_SUMMON_HP_MP_SP       = 10032,
    EF_CREATURE_ASSIGNMENT_INCREASE  = 10033,
    EF_CREATURE_ACQUIREMENT_INCREASE = 10034,
    EF_BELT_ON_PARAMETER_INC         = 10035,
    EF_BELT_ON_ATTRIBUTE_INC         = 10036,
    EF_BELT_ON_ATTRIBUTE_EX_INC      = 10037,
    EF_BELT_ON_ATTRIBUTE_HPMP_INC    = 10038,

    EF_UNIT_EXPERT = 10041,

    EF_BELT_ON_PARAMETER_AMP      = 10042,
    EF_BELT_ON_ATTRIBUTE_AMP      = 10043,
    EF_BELT_ON_ATTRIBUTE_EX_AMP   = 10044,
    EF_BELT_ON_ATTRIBUTE_HPMP_AMP = 10045,

    EF_SUMMON_ITEM_EXPERT = 10046,

    EF_PHYSICAL_SINGLE_DAMAGE                = 30001,
    EF_PHYSICAL_SINGLE_DAMAGE_ABSORB         = 30002,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY     = 30003,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH           = 30004,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK = 30005,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK      = 30006,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK      = 30007,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF = 30008,

    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE      = 30009,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK = 30010,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE                     = 30011,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE                   = 30012,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE             = 30013,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE           = 30014,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF      = 30015,
    EF_PHYSICAL_MULTIPLE_DAMAGE                          = 30016,
    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK       = 30017,
    EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE          = 30018,
    EF_PHYSICAL_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE = 30019,

    EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL = 30030,

    EF_RESURRECTION_WITH_RECOVER = 30501,
    EF_REMOVE_STATE_GROUP        = 30601,

    EF_WEAPON_TRAINING            = 31001,
    EF_AMPLIFY_BASE_ATTRIBUTE_OLD = 31002,
    EF_AMPLIFY_EXT_ATTRIBUTE      = 31003,

    EF_AMPLIFY_EXP_FOR_SUMMON = 32001,
};

struct SkillTreeBase
{
    int   job_id{ };
    int   skill_id{ };
    int   min_skill_lv{ };
    int   max_skill_lv{ };
    int   lv{ };
    int   job_lv{ };
    float jp_ratio{ };
    int   need_skill_id[3]{ };
    int   need_skill_lv[3]{ };
};

struct SkillTreeGroup
{
    int                        job_id{ };
    int                        skill_id{ };
    std::vector<SkillTreeBase> skillTrees{ };
};

struct SkillBase
{

    int GetID() const;
    int GetNameID() const;
    bool IsValid() const;
    bool IsSystemSkill() const;

    bool IsPassive() const;
    bool IsPhysicalSkill() const;
    bool IsMagicalSkill() const;
    bool IsHarmful() const;
    bool IsNeedTarget() const;
    bool IsValidToCorpse() const;
    bool IsToggle() const;

    int GetCriticalBonus(int skill_lv) const;
    int GetCastRange() const;
    int GetValidRange() const;

    int GetToggleGroup() const;
    int GetSkillTargetType() const;
    int GetSkillEffectType() const;
    int GetElementalType() const;

    int32_t GetCostEXP(int skill_lv, int enhance) const;
    int32_t GetCostJP(int skill_lv, int enhance) const;
    int GetProbabilityOnHit(int slv) const;
    int32_t GetCostItemCode() const;
    int64_t GetCostItemCount(int skill_lv) const;
    int32_t GetCostHP(int skill_lv) const;
    float GetCostHPPercent(int skill_lv) const;
    int32_t GetCostMP(int skill_lv, int enhance) const;
    float GetCostMPPercent(int skill_lv) const;
    int32_t GetCostHavoc(int skill_lv) const;
    int32_t GetCostEnergy(int skill_lv) const;
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

    int GetHatePoint(int lv, int point, int enhance) const;

    bool IsNeedShield() const;
    bool IsNeedWeapon() const;

    int GetNeedJobPoint(int skill_lv);
    bool IsUseableWeapon(ItemClass cl);
    int GetStateSecond(int skill_lv, int enhance_lv);
    int GetStateType() const;
    int GetHitBonus(int enhance, int level_diff) const;
    int GetStateId() const;
    int GetStateLevel(int skill_lv, int enhance_lv);
    uint GetCastDelay(int skill_lv, int enhance);
    uint32_t GetCommonDelay() const;
    uint GetCoolTime(int enhance) const;
    uint32_t GetFireRange() const;
    bool IsUsable(uint8 nUseIndex) const;
    int GetCostEnergy(uint8 skill_lv) const;

    int m_need_jp[50]{ };

    int     id{ };
    int     text_id{ };
    short   is_valid{ };
    uint8_t elemental{ };
    uint8_t is_passive{ };
    uint8_t is_physical_act{ };
    uint8_t is_harmful{ };
    uint8_t is_need_target{ };
    uint8_t is_corpse{ };
    uint8_t is_toggle{ };
    int     toggle_group{ };
    uint8_t casting_type{ };
    uint8_t casting_level{ };
    int     cast_range{ };
    int     valid_range{ };
    int     cost_hp{ };
    int     cost_hp_per_skl{ };
    int     cost_mp{ };
    int     cost_mp_per_skl{ };
    int     cost_mp_per_enhance{ };
    float   cost_hp_per{ };
    float   cost_hp_per_skl_per{ };
    float   cost_mp_per{ };
    float   cost_mp_per_skl_per{ };
    int     cost_havoc{ };
    int     cost_havoc_per_skl{ };
    float   cost_energy{ };
    float   cost_energy_per_skl{ };
    int     cost_exp{ };
    int     cost_exp_per_enhance{ };
    int     cost_jp{ };
    int     cost_jp_per_enhance{ };
    int     cost_item{ };
    int     cost_item_count{ };
    int     cost_item_count_per{ };
    int     need_level{ };
    int     need_hp{ };
    int     need_mp{ };
    int     need_havoc{ };
    int     need_havoc_burst{ };
    int     need_state_id{ };
    short   need_state_level{ };
    short   need_state_exhaust{ };
    uint8_t vf_one_hand_sword{ };
    uint8_t vf_two_hand_sword{ };
    uint8_t vf_double_sword{ };
    uint8_t vf_dagger{ };
    uint8_t vf_double_dagger{ };
    uint8_t vf_spear{ };
    uint8_t vf_axe{ };
    uint8_t vf_one_hand_axe{ };
    uint8_t vf_double_axe{ };
    uint8_t vf_one_hand_mace{ };
    uint8_t vf_two_hand_mace{ };
    uint8_t vf_lightbow{ };
    uint8_t vf_heavybow{ };
    uint8_t vf_crossbow{ };
    uint8_t vf_one_hand_staff{ };
    uint8_t vf_two_hand_staff{ };
    uint8_t vf_shield_only{ };
    uint8_t vf_is_not_need_weapon{ };
    float   delay_cast{ };
    float   delay_cast_per_skl{ };
    float   delay_cast_mode_per{ };
    float   delay_common{ };
    float   delay_cooltime{ };
    float   delay_cooltime_mode{ };
    int     cool_time_group_id{ };
    uint8_t uf_self{ };
    uint8_t uf_party{ };
    uint8_t uf_guild{ };
    uint8_t uf_neutral{ };
    uint8_t uf_purple{ };
    uint8_t uf_enemy{ };
    uint8_t tf_avatar{ };
    uint8_t tf_summon{ };
    uint8_t tf_monster{ };
    short   target{ };
    short   effect_type{ };
    int     state_id{ };
    int     state_level_base{ };
    float   state_level_per_skl{ };
    float   state_level_per_enhance{ };
    float   state_second{ };
    float   state_second_per_level{ };
    float   state_second_per_enhance{ };
    uint8_t state_type{ };
    int     probability_on_hit{ };
    int     probability_inc_by_slv{ };
    short   hit_bonus{ };
    short   hit_bonus_per_enhance{ };
    short   percentage{ };
    float   hate_mod{ };
    short   hate_basic{ };
    float   hate_per_skl{ };
    float   hate_per_enhance{ };
    int     critical_bonus{ };
    int     critical_bonus_per_skl{ };
    float   var[20]{ };
    short   is_projectile{ };
    float   projectile_speed{ };
    float   projectile_acceleration{ };
};