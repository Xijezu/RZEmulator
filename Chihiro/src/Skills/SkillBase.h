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

enum SKILL_RESULT_TYPE : int
{
    SRT_DAMAGE                 = 0,
    SRT_MAGIC_DAMAGE           = 1,
    SRT_DAMAGE_WITH_KNOCK_BACK = 2,
    SRT_RESULT                 = 10,
    SRT_ADD_HP                 = 20,
    SRT_ADD_MP                 = 21,
    SRT_ADD_HP_MP_SP           = 22,
    SRT_REBIRTH                = 23,
    SRT_RUSH                   = 30,
    SRT_NOT_USE                = 100,
};

struct SR_DamageType
{
    int    target_hp;
    uint8  damage_type;
    int    damage;
    int    flag;
    uint16 elemental_damage[7];
};

struct SR_DamageWithKnockBackType
{
    int    target_hp;
    uint8  damage_type;
    int    damage;
    int    flag;
    uint16 elemental_damage[7];
    float  x;
    float  y;
    uint8  speed;
    uint   knock_back_time;
};

struct SR_ResultType
{
    bool bResult;
    int  success_type;
};

struct SR_AddHPType
{
    int target_hp;
    int nIncHP;
};

struct SR_AddHPMPSPType
{
    int   target_hp;
    int   nIncHP;
    int   nIncMP;
    int   nIncSP;
    int16 target_mp;
};

struct SR_RushType
{
    bool  bResult;
    float x;
    float y;
    float face;
    uint8 speed;
};

struct SR_RebirthType
{
    int   target_hp;
    int   nIncHP;
    int   nIncMP;
    int   nRecoveryEXP;
    int16 target_mp;
};

struct SkillResult
{
    uint8 type;
    uint  hTarget;

    SR_DamageType              damage;
    SR_DamageWithKnockBackType damage_kb;
    SR_ResultType              result;
    SR_AddHPType               add_hp;
    SR_AddHPMPSPType           add_hp_mp_sp;
    SR_RushType                rush;
    SR_RebirthType             rebirth;
};

enum SKILL_STATUS : short
{
    ST_FIRE           = 0,
    ST_CASTING        = 1,
    ST_CASTING_UPDATE = 2,
    ST_CANCEL         = 3,
    ST_REGION_FIRE    = 4,
    ST_COMPLETE       = 5
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
    EF_MISC                                                  = 0x0,
    EF_RESPAWN_MONSTER_NEAR                                  = 0x2,
    EF_PARAMETER_INC                                         = 0x3,
    EF_PARAMETER_AMP                                         = 0x4,
    EF_RESPAWN_MONSTER_RANDOMLY                              = 0x5,
    EF_RESPAWN_MONSTER_WITH_DIFF_CODE                        = 0x6,
    EF_PHYSICAL_SINGLE_DAMAGE_T1                             = 0x65,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T1                           = 0x66,
    EF_PHYSICAL_SINGLE_DAMAGE_T2                             = 0x67,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T2                           = 0x68,
    EF_PHYSICAL_DIRECTIONAL_DAMAGE                           = 0x69,
    EF_PHYSICAL_SINGLE_DAMAGE_T3                             = 0x6A,
    EF_PHYSICAL_MULTIPLE_DAMAGE_T3                           = 0x6B,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK_OLD            = 0x6C,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_OLD                     = 0x6F,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD                   = 0x70,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE_OLD             = 0x71,
    EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD                    = 0x75,
    EF_PHYSICAL_ABSORB_DAMAGE                                = 0x79,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_OLD           = 0x7A,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY_OLD                 = 0x7D,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD                  = 0x83,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_OLD           = 0x84,
    EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK = 0x97,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD             = 0x98,
    EF_MAGIC_SINGLE_DAMAGE_T1_OLD                            = 0xC9,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_OLD                          = 0xCA,
    EF_MAGIC_SINGLE_DAMAGE_T2_OLD                            = 0xCB,
    EF_MAGIC_MULTIPLE_DAMAGE_T2_OLD                          = 0xCC,
    EF_MAGIC_MULTIPLE_DAMAGE_T3_OLD                          = 0xCD,
    EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD           = 0xCE,
    EF_MAGIC_SINGLE_REGION_DAMAGE_OLD                        = 0xD3,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_OLD                      = 0xD4,
    EF_MAGIC_SPECIAL_REGION_DAMAGE_OLD                       = 0xD5,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_T2_OLD                   = 0xD6,
    EF_MAGIC_ABSORB_DAMAGE_OLD                               = 0xDD,
    EF_MAGIC_SINGLE_DAMAGE                                   = 0xE7,
    EF_MAGIC_MULTIPLE_DAMAGE                                 = 0xE8,
    EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP                  = 0xE9,
    EF_MAGIC_SINGLE_DAMAGE_OR_DEATH                          = 0xEA,
    EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP                        = 0xEB,
    EF_MAGIC_SINGLE_PERCENT_DAMAGE                           = 0xEC,
    EF_MAGIC_SINGLE_PERCENT_MANABURN                         = 0xED,
    EF_MAGIC_SINGLE_PERCENT_OF_MAX_MP_MANABURN               = 0xEE,
    EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE                  = 0xEF,
    EF_MAGIC_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE        = 0xF0,
    EF_MAGIC_MULTIPLE_DAMAGE_AT_ONCE                         = 0xF1,
    EF_MAGIC_SINGLE_REGION_DAMAGE                            = 0x105,
    EF_MAGIC_SPECIAL_REGION_DAMAGE                           = 0x106,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE                          = 0x107,
    EF_MAGIC_REGION_PERCENT_DAMAGE                           = 0x108,
    EF_MAGIC_SINGLE_REGION_DAMAGE_USING_CORPSE               = 0x109,
    EF_ADD_HP_MP_BY_ABSORB_HP_MP                             = 0x10A,
    EF_MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD             = 0x10B,
    EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE           = 0x10C,
    EF_MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE                  = 0x10D,
    EF_AREA_EFFECT_MAGIC_DAMAGE                              = 0x10F,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL                     = 0x110,
    EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2                  = 0x111,
    EF_ADD_STATE                                             = 0x12D,
    EF_ADD_REGION_STATE                                      = 0x12E,
    EF_CASTING_CANCEL_WITH_ADD_STATE                         = 0x130,
    EF_ADD_STATE_BY_SELF_COST                                = 0x131,
    EF_ADD_REGION_STATE_BY_SELF_COST                         = 0x132,
    EF_ADD_STATE_BY_TARGET_TYPE                              = 0x133,
    EF_ADD_STATES_WITH_EACH_DIFF_LV                          = 0x134,
    EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION                 = 0x135,
    EF_ADD_STATE_STEP_BY_STEP                                = 0x136,
    EF_ADD_STATE_TO_CASTER_AND_TARGET                        = 0x137,
    EF_ADD_RANDOM_STATE                                      = 0x138,
    EF_ADD_RANDOM_REGION_STATE                               = 0x139,
    EF_ADD_STATE_BY_USING_ITEM                               = 0x13A,
    EF_AREA_EFFECT_MAGIC_DAMAGE_OLD                          = 0x160,
    EF_AREA_EFFECT_HEAL                                      = 0x161,
    EF_TRAP_PHYSICAL_DAMAGE                                  = 0x17D,
    EF_TRAP_MAGICAL_DAMAGE                                   = 0x17E,
    EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE                         = 0x17F,
    EF_TRAP_MULTIPLE_MAGICAL_DAMAGE                          = 0x180,
    EF_REMOVE_BAD_STATE                                      = 0x191,
    EF_REMOVE_GOOD_STATE                                     = 0x192,
    EF_ADD_HP                                                = 0x1F5,
    EF_ADD_MP                                                = 0x1F6,
    EF_RESURRECTION                                          = 0x1F8,
    EF_ADD_HP_MP                                             = 0x1F9,
    EF_ADD_HP_MP_BY_SUMMON_DAMAGE                            = 0x1FA,
    EF_ADD_HP_MP_BY_SUMMON_DEAD                              = 0x1FB,
    EF_ADD_REGION_HP_MP                                      = 0x1FC,
    EF_ADD_HP_BY_ITEM                                        = 0x1FD,
    EF_ADD_MP_BY_ITEM                                        = 0x1FE,
    EF_CORPSE_ABSORB                                         = 0x1FF,
    EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP                       = 0x200,
    EF_ADD_HP_MP_WITH_LIMIT_PERCENT                          = 0x201,
    EF_ADD_REGION_HP                                         = 0x209,
    EF_ADD_REGION_MP                                         = 0x20A,
    EF_SUMMON                                                = 0x259,
    EF_UNSUMMON                                              = 0x25A,
    EF_UNSUMMON_AND_ADD_STATE                                = 0x25D,
    EF_TOGGLE_AURA                                           = 0x2BD,
    EF_TOGGLE_DIFFERENTIAL_AURA                              = 0x2BE,
    EF_TAUNT                                                 = 0x384,
    EF_REGION_TAUNT                                          = 0x385,
    EF_REMOVE_HATE                                           = 0x386,
    EF_REGION_REMOVE_HATE                                    = 0x387,
    EF_REGION_REMOVE_HATE_OF_TARGET                          = 0x388,
    EF_CORPSE_EXPLOSION                                      = 0x3E9,
    EF_CREATE_ITEM                                           = 0x2329,
    EF_ACTIVATE_FIELD_PROP                                   = 0x251D,
    EF_REGION_HEAL_BY_FIELD_PROP                             = 0x251E,
    EF_AREA_EFFECT_HEAL_BY_FIELD_PROP                        = 0x251F,
    EF_WEAPON_MASTERY                                        = 0x2711,
    EF_BATTLE_PARAMTER_INCREASE                              = 0x2712,
    EF_BLOCK_INCREASE                                        = 0x2713,
    EF_ATTACK_RANGE_INCREASE                                 = 0x2714,
    EF_RESISTANCE_INCREASE                                   = 0x2715,
    EF_MAGIC_REGISTANCE_INCREASE                             = 0x2716,
    EF_SPECIALIZE_ARMOR                                      = 0x2717,
    EF_INCREASE_BASE_ATTRIBUTE                               = 0x2718,
    EF_INCREASE_EXTENSION_ATTRIBUTE                          = 0x2719,
    EF_SPECIALIZE_ARMOR_AMP                                  = 0x271A,
    EF_AMPLIFY_BASE_ATTRIBUTE                                = 0x271B,
    EF_MAGIC_TRAINING                                        = 0x271C,
    EF_HUNTING_TRAINING                                      = 0x271D,
    EF_BOW_TRAINING                                          = 0x271E,
    EF_INCREASE_STAT                                         = 0x271F,
    EF_AMPLIFY_STAT                                          = 0x2720,
    EF_INCREASE_HP_MP                                        = 0x2725,
    EF_AMPLIFY_HP_MP                                         = 0x2726,
    EF_HEALING_AMPLIFY                                       = 0x2727,
    EF_HEALING_AMPLIFY_BY_ITEM                               = 0x2728,
    EF_HEALING_AMPLIFY_BY_REST                               = 0x2729,
    EF_HATE_AMPLIFY                                          = 0x272A,
    EF_INCREASE_SUMMON_HP_MP_SP                              = 0x272F,
    EF_AMPLIFY_SUMMON_HP_MP_SP                               = 0x2730,
    EF_CREATURE_ASSIGNMENT_INCREASE                          = 0x2731,
    EF_BELT_ON_PARAMETER_INC                                 = 0x2733,
    EF_BELT_ON_ATTRIBUTE_INC                                 = 0x2734,
    EF_BELT_ON_ATTRIBUTE_EX_INC                              = 0x2735,
    EF_BELT_ON_ATTRIBUTE_EX2_INC                             = 0x2736,
    EF_UNIT_EXPERT                                           = 0x2739,
    EF_BELT_ON_PARAMETER_AMP                                 = 0x273A,
    EF_BELT_ON_ATTRIBUTE_AMP                                 = 0x273B,
    EF_BELT_ON_ATTRIBUTE_EX_AMP                              = 0x273C,
    EF_BELT_ON_ATTRIBUTE_EX2_AMP                             = 0x273D,
    EF_SUMMON_ITEM_EXPERT                                    = 0x273E,
    EF_ADD_STATE_ON_ATTACK                                   = 0x2740,
    EF_ADD_STATE_BY_SELF_ON_ATTACK                           = 0x2741,
    EF_ADD_STATE_ON_BEING_ATTACKED                           = 0x2742,
    EF_ADD_STATE_BY_SELF_ON_BEING_ATTACKED                   = 0x2743,
    EF_ADD_STATE_BY_SELF_ON_KILL                             = 0x2744,
    EF_ADD_STATE_ON_CRITICAL_ATTACK                          = 0x2745,
    EF_ADD_STATE_BY_SELF_ON_CRITICAL_ATTACK                  = 0x2746,
    EF_ADD_STATE_ON_BEING_CRITICAL_ATTACKED                  = 0x2747,
    EF_ADD_STATE_BY_SELF_ON_BEING_CRITICAL_ATTACKED          = 0x2748,
    EF_ADD_STATE_ON_AVOID                                    = 0x2749,
    EF_ADD_STATE_BY_SELF_ON_AVOID                            = 0x274A,
    EF_ADD_STATE_ON_BLOCK                                    = 0x274B,
    EF_ADD_STATE_BY_SELF_ON_BLOCK                            = 0x274C,
    EF_ADD_STATE_ON_PERFECT_BLOCK                            = 0x274D,
    EF_ADD_STATE_BY_SELF_ON_PERFECT_BLOCK                    = 0x274E,
    EF_ADD_ENERGY_ON_ATTACK                                  = 0x7E06,
    EF_ADD_ENERGY_ON_BEING_ATTACKED                          = 0x7E07,
    EF_INC_SKILL_COOL_TIME_ON_ATTACK                         = 0x274F,
    EF_INC_SKILL_COOL_TIME_ON_BEING_ATTACKED                 = 0x2750,
    EF_INC_SKILL_COOL_TIME_ON_KILL                           = 0x2751,
    EF_INC_SKILL_COOL_TIME_ON_CRITICAL_ATTACK                = 0x2752,
    EF_INC_SKILL_COOL_TIME_ON_BEING_CRITICAL_ATTACKED        = 0x2753,
    EF_INC_SKILL_COOL_TIME_ON_AVOID                          = 0x2754,
    EF_INC_SKILL_COOL_TIME_ON_BLOCK                          = 0x2755,
    EF_INC_SKILL_COOL_TIME_ON_PERFECT_BLOCK                  = 0x2756,
    EF_INC_SKILL_COOL_TIME_ON_SKILL_OF_ID                    = 0x7E19,
    EF_PHYSICAL_SINGLE_DAMAGE                                = 0x7531,
    EF_PHYSICAL_SINGLE_DAMAGE_ABSORB                         = 0x7532,
    EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY                     = 0x7533,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH                           = 0x7534,
    EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK                 = 0x7535,
    EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK                      = 0x7536,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK               = 0x7537,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF          = 0x7538,
    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE                     = 0x7539,
    EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK                = 0x753A,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE                         = 0x753B,
    EF_PHYSICAL_MULTIPLE_REGION_DAMAGE                       = 0x753C,
    EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE                 = 0x753D,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE               = 0x753E,
    EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF          = 0x753F,
    EF_PHYSICAL_MULTIPLE_DAMAGE                              = 0x7540,
    EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK           = 0x7541,
    EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE              = 0x7542,
    EF_PHYSICAL_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE     = 0x7543,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_ADDING_MAGICAL_DAMAGE   = 0x7544,
    EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL        = 0x754E,
    EF_RESURRECTION_WITH_RECOVER                             = 0x7725,
    EF_REMOVE_STATE_GROUP                                    = 0x7789,
    EF_LOTTO                                                 = 0x77ED,
    EF_WEAPON_TRAINING                                       = 0x7919,
    EF_AMPLIFY_BASE_ATTRIBUTE_OLD                            = 0x791A,
    EF_AMPLIFY_EXT_ATTRIBUTE                                 = 0x791B,
    EF_AMPLIFY_EXP_FOR_SUMMON                                = 0x7D01,
    EF_ENHANCE_SKILL                                         = 0x7D0B,
    EF_MAGIC_SINGLE_DAMAGE_WITH_PHYSICAL_DAMAGE              = 0x7D15,
    EF_INC_DAMAGE_BY_TARGET_STATE                            = 0x7D1F,
    EF_AMP_DAMAGE_BY_TARGET_STATE                            = 0x7D20,
    EF_TRANFER_HEALING                                       = 0x7D33,
    EF_PHYSICAL_CHAIN_DAMAGE                                 = 0x7D3D,
    EF_MAGIC_CHAIN_DAMAGE                                    = 0x7D3E,
    EF_CHAIN_HEAL                                            = 0x7D3F,
    EF_PHYSICAL_SINGLE_DAMAGE_DEMINISHED_HP_MP               = 0x7D8D,
    EF_MODIFY_SKILL_COST                                     = 0x7DAB,
    EF_RESIST_HARMFUL_STATE                                  = 0x7DB7,
    EF_INC_SKILL_COOL_TIME                                   = 0x7DBF,
    EF_AMP_SKILL_COOL_TIME                                   = 0x7DC0,
    EF_INC_DAMAGE_INC_CRIT_RATE_BY_TARGET_HP_RATIO           = 0x7DC9,
    EF_AMP_DAMAGE_INC_CRIT_RATE_BY_TARGET_HP_RATIO           = 0x7DCA,
    EF_ABSORB_DAMAGE                                         = 0x7DD3,
    EF_STEAL_HP_MP                                           = 0x7DD4,
    EF_PHYSICAL_SINGLE_DAMAGE_PROP_REMAIN_MP                 = 0x7DFB,
    EF_REPLENISH_ENERGY_HP_MP                                = 0x7E05,
    EF_INCREASE_ENERGY_UNCONSUMPTION_RATE                    = 0x7E08,
    EF_INC_PARAM_AMPLIFY_HEAL                                = 0x7E0F,
    EF_AMP_PARAM_AMPLIFY_HEAL                                = 0x7E10,
    EF_INC_PARAM_BY_STATE                                    = 0x7E23,
    EF_AMP_PARAM_BY_STATE                                    = 0x7E24,
    EF_INC_PARAM_BASED_PARAM                                 = 0x7E2D,
    EF_INC_SUMMON_PARAM_BASED_PARAM                          = 0x7E2E,
    EF_INC_SUMMON_PARAM_BASED_SUMMON_PARAM                   = 0x7E2F,
    EF_INC_PARAM_BASED_SUMMON_PARAM                          = 0x7E30
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

    int GetNeedJobPoint(int skill_lv);
    bool IsUseableWeapon(ItemClass cl);
    int GetStateSecond(int skill_lv, int enhance_lv);
    int GetHitBonus(int enhance, int level_diff) const;
    int GetStateLevel(int skill_lv, int enhance_lv);
    uint GetCastDelay(int skill_lv, int enhance);
    uint GetCoolTime(int enhance) const;
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