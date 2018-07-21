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

enum AttributeFlag
{
    AF_ERASE_ON_DEAD           = 0x1,
    AF_ERASE_ON_LOGOUT         = 0x2,
    AF_TIME_DECREASE_ON_LOGOUT = 0x4,
    AF_NOT_ACTABLE_TO_BOSS     = 0x8,
    AF_AF_NOT_ERASABLE         = 0x10,
    AF_ERASE_ON_REQUEST        = 0x20,
    AF_ERASE_ON_DAMAGED        = 0x40,
    AF_ERASE_ON_RESURRECT      = 0x80,
};

enum StateCode
{
    SC_NONE                                = 0x000,
    SC_INC_ITEM_CHANCE                     = 0x3FC,
    SC_INC_BLOCK_CHANCE                    = 0x3FD,
    SC_HUNTING_CREATURE_CARD               = 0x803,
    SC_STAMINA_SAVE                        = 0xFA3,
    SC_HAVOC_BURST                         = 0x1195,
    SC_FRENZY                              = 0x1197,
    SC_ADD_ENERGY                          = 0x1198,
    SC_BLESS_OF_GODDESS                    = 0x119E,
    SC_STIGMA_OF_CRIME                     = 0x11A0,
    SC_NEMESIS_FOR_AUTO                    = 0x176D,
    SC_NEMESIS                             = 0x176F,
    SC_SLEEP                               = 0x1775,
    SC_STUN                                = 0x1776,
    SC_HOLD                                = 0x1777,
    SC_FEAR                                = 0x1778,
    SC_FROZEN                              = 0x1779,
    SC_STONECURSED                         = 0x177C,
    SC_MUTE                                = 0x177D,
    SC_HIDE                                = 0x1780,
    SC_FALL_FROM_SUMMON                    = 0x2329,
    SC_GAIA_MEMBER_SHIP                    = 0x232C,
    SC_PCBANG_MEMBER_SHIP                  = 0x232D,
    SC_PCBANG_PREMIUM_MEMBER_SHIP          = 0x232E,
    SC_BURNING_STYLE                       = 0x317F,
    SC_DUSK_STYLE                          = 0x3181,
    SC_AGILE_STYLE                         = 0x3180,
    SC_SQUALL_OF_ARROW                     = 0x3186,
    SC_FROZEN_SNARE                        = 0x32CB,
    SC_EARTH_RESTRICTION                   = 0x32DF,
    SC_NIGHTMARE                           = 0x3521,
    SC_PASS_DAMAGE                         = 0x36BC,
    SC_PET_SHOVELING_REWARD_INC_MOVE_SPEED = 0x3A98,
    SC_PET_SHOVELING_REWARD_DEC_MOVE_SPEED = 0x3A99,
    SC_PET_SHOVELING_REWARD_INC_STR_INT    = 0x3A9A,
    SC_PET_SHOVELING_REWARD_DEC_STR_INT    = 0x3A9B,
    SC_PET_SHOVELING_REWARD_INC_AGI_DEX    = 0x3A9C,
    SC_PET_SHOVELING_REWARD_DEC_AGI_DEX    = 0x3A9D,
    SC_PET_SHOVELING_REWARD_INC_VIT        = 0x3A9E,
    SC_PET_SHOVELING_REWARD_DEC_VIT        = 0x3A9F,
    SC_LIGHTNING_FORCE_CONGESTION          = 0x27E5C,
    SC_FUSION_WITH_SUMMON                  = 0x280A1,
    SC_SUMMON_FORM                         = 0x280A2,
    SC_SEAL                                = 0xF4236,
    SC_SHINE_WALL                          = 0xF4237,
};

enum StateType
{
    SG_NORMAL     = 0x0,
    SG_DUPLICATE  = 0x1,
    SG_DEPENDENCE = 0x2,
};

enum StateGroup
{
    GROUP_NONE          = 0x0,
    GROUP_PHYSICAL_BOMB = 0x65,
    GROUP_MAGICAL_BOMB  = 0x66,
    GROUP_ASSASSIN      = 0xC9,
    //GROUP_RIDING        = 0x12D,
    GROUP_MAGIC         = 0x191,
    GROUP_SKILL         = 0x192,
    GROUP_POISON        = 0x193,
    GROUP_CURSE         = 0x194,
    GROUP_DISEASE       = 0x195,
    GROUP_WOUND         = 0x196,
};

enum StateBaseEffect : int
{
    SEF_MISC                               = 0x0,
    SEF_PARAMETER_INC                      = 0x1,
    SEF_PARAMETER_AMP                      = 0x2,
    SEF_PARAMETER_INC_WHEN_EQUIP_SHIELD    = 0x3,
    SEF_PARAMETER_AMP_WHEN_EQUIP_SHIELD    = 0x4,
    SEF_PARAMETER_INC_WHEN_EQUIP           = 0x5,
    SEF_PARAMETER_AMP_WHEN_EQUIP           = 0x6,
    SEF_DOUBLE_ATTACK                      = 0x15,
    SEF_ADDITIONAL_DAMAGE_ON_ATTACK        = 0x16,
    SEF_AMP_ADDITIONAL_DAMAGE_ON_ATTACK    = 0x17,
    SEF_ADDITIONAL_DAMAGE_ON_SKILL         = 0x18,
    SEF_AMP_ADDTIONAL_DAMAGE_ON_SKILL      = 0x19,
    SEF_ADD_STATE_ON_ATTACK                = 0x1A,
    SEF_ADD_HP_ON_ATTACK                   = 0x1B,
    SEF_AMP_RECEIVE_DAMAGE                 = 0x1E,
    SEF_ADD_HP_MP_ON_CRITICAL              = 0x22,
    SEF_HEALING_AMPLIFY                    = 0x23,
    SEF_ABSORB                             = 0x29,
    SEF_STEAL                              = 0x2A,
    SEF_DAMAGE_REFLECT_PERCENT             = 0x2B,
    SEF_DAMAGE_REFLECT                     = 0x2C,
    SEF_DAMAGE_REFLECT_WHEN_EQUIP_SHIELD   = 0x2D,
    SEF_DAMAGE_REDUCE_WITH_RACE_BY_PERCENT = 0x2E,
    SEF_DAMAGE_REDUCE_WITH_RACE_BY_VALUE   = 0x2F,
    SEF_MANA_SHIELD                        = 0x31,
    SEF_HEAL                               = 0x3D,
    SEF_REGEN_ADD                          = 0x3E,
    SEF_HEAL_BY_ITEM                       = 0x3F,
    SEF_HEAL_HPMP_PER                      = 0x40,
    SEF_FORCE_CHIP                         = 0x47,
    SEF_SOUL_CHIP                          = 0x48,
    SEF_HEALING_CHIP                       = 0x49,
    SEF_LUNAR_CHIP                         = 0x4A,
    SEF_NEUTRALIZE                         = 0x51,
    SEF_MEZZ                               = 0x52,
    SEF_PROVOKE                            = 0x53,
    SEF_DECREASE_STATE_EFFECT              = 0x54,
    SEF_INC_HATE                           = 0x55,
    SEF_AMP_HATE                           = 0x56,
    SEF_SKILL_INTERRUPTION                 = 0x57,
    SEF_ADD_REGION_STATE                   = 0x58,
    SEF_MP_COST_INC                        = 0x5B,
    SEF_ADD_PARAMETER_ON_NORMAL_ATTACK     = 0x5C,
    SEF_ADD_PARAMETER_ON_SKILL             = 0x5D,
    SEF_KNOCKBACK_ON_DEFAULT_ATTACK        = 0x5E,
    SEF_REMOVE_GOOD_STATE                  = 0x61,
    SEF_TRANSFORMATION                     = 0x68,
    SEF_DEAL_DAMAGE                        = 0x69,
    SEF_AMPLIFY_CASTING_TIME               = 0x6A,
    SEF_RESURRECTION                       = 0x6D,
    SEF_DETECT_HIDE                        = 0x6F,
    SEF_CREATURE_PARAMETER_AMP             = 0x70,
    SEF_GIVE_EXP_JP_TO_CREATURE            = 0x71,
    SEF_CHANGING_FORM                      = 0x72,
    SEF_RIDING                             = 0xC8,
    SEF_AMP_AND_INC_ITEM_CHANCE            = 0xC9,
};

struct StateTemplate
{
    int   state_id;
    int   text_id;
    int   tooltip_id;
    uint8 is_harmful;
    int   state_time_type;
    int   state_group;
    int   duplicate_group[3]{0};
    uint8 uf_avatar;
    uint8 uf_summon;
    uint8 uf_monster;
    int   base_effect_id;
    int   fire_interval;
    int   elemental_type;
    float amplify_base;
    float amplify_per_skl;
    int   add_damage_base;
    int   add_damage_per_skl;
    int   effect_type;
    float value[20]{0};
};