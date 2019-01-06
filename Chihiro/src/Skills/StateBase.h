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

enum AttributeFlag
{
    AF_ERASE_ON_DEAD = 0x1,
    AF_ERASE_ON_LOGOUT = 0x2,
    AF_TIME_DECREASE_ON_LOGOUT = 0x4,
    AF_NOT_ACTABLE_TO_BOSS = 0x8,
    AF_AF_NOT_ERASABLE = 0x10,
    AF_ERASE_ON_REQUEST = 0x20,
    AF_ERASE_ON_DAMAGED = 0x40,
    AF_ERASE_ON_RESURRECT = 0x80,
};

enum StateCode
{
    SC_NONE = 0x000,
    SC_INC_ITEM_CHANCE = 0x3FC,
    SC_INC_BLOCK_CHANCE = 0x3FD,
    SC_HUNTING_CREATURE_CARD = 0x803,
    SC_STAMINA_SAVE = 0xFA3,
    SC_HAVOC_BURST = 0x1195,
    SC_FRENZY = 0x1197,
    SC_ADD_ENERGY = 0x1198,
    SC_BLESS_OF_GODDESS = 0x119E,
    SC_STIGMA_OF_CRIME = 0x11A0,
    SC_NEMESIS_FOR_AUTO = 0x176D,
    SC_NEMESIS = 0x176F,
    SC_SLEEP = 0x1775,
    SC_STUN = 0x1776,
    SC_HOLD = 0x1777,
    SC_FEAR = 0x1778,
    SC_FROZEN = 0x1779,
    SC_STONECURSED = 0x177C,
    SC_MUTE = 0x177D,
    SC_HIDE = 0x1780,
    SC_FALL_FROM_SUMMON = 0x2329,
    SC_GAIA_MEMBER_SHIP = 0x232C,
    SC_PCBANG_MEMBER_SHIP = 0x232D,
    SC_PCBANG_PREMIUM_MEMBER_SHIP = 0x232E,
    SC_BURNING_STYLE = 0x317F,
    SC_DUSK_STYLE = 0x3181,
    SC_AGILE_STYLE = 0x3180,
    SC_SQUALL_OF_ARROW = 0x3186,
    SC_FROZEN_SNARE = 0x32CB,
    SC_EARTH_RESTRICTION = 0x32DF,
    SC_NIGHTMARE = 0x3521,
    SC_PASS_DAMAGE = 0x36BC,
    SC_PET_SHOVELING_REWARD_INC_MOVE_SPEED = 0x3A98,
    SC_PET_SHOVELING_REWARD_DEC_MOVE_SPEED = 0x3A99,
    SC_PET_SHOVELING_REWARD_INC_STR_INT = 0x3A9A,
    SC_PET_SHOVELING_REWARD_DEC_STR_INT = 0x3A9B,
    SC_PET_SHOVELING_REWARD_INC_AGI_DEX = 0x3A9C,
    SC_PET_SHOVELING_REWARD_DEC_AGI_DEX = 0x3A9D,
    SC_PET_SHOVELING_REWARD_INC_VIT = 0x3A9E,
    SC_PET_SHOVELING_REWARD_DEC_VIT = 0x3A9F,
    SC_LIGHTNING_FORCE_CONGESTION = 0x27E5C,
    SC_FUSION_WITH_SUMMON = 0x280A1,
    SC_SUMMON_FORM = 0x280A2,
    SC_SEAL = 0xF4236,
    SC_SHINE_WALL = 0xF4237,
    SC_STONECURSE = 6012,
    SC_STONECURSE_MORTAL = 6019,
    SC_PROTECTING_FORCE_OF_BEGINNING = 201085,
    SC_TRACE_OF_FUGITIVE = 201084,
};

enum StateType
{
    SG_NORMAL = 0x0,
    SG_DUPLICATE = 0x1,
    SG_DEPENDENCE = 0x2,
};

enum StateGroup
{
    GROUP_NONE = 0x0,
    GROUP_PHYSICAL_BOMB = 0x65,
    GROUP_MAGICAL_BOMB = 0x66,
    GROUP_ASSASSIN = 0xC9,
    //GROUP_RIDING        = 0x12D,
    GROUP_MAGIC = 0x191,
    GROUP_SKILL = 0x192,
    GROUP_POISON = 0x193,
    GROUP_CURSE = 0x194,
    GROUP_DISEASE = 0x195,
    GROUP_WOUND = 0x196,
};

enum StateStatFlag
{
    FLAG_STR = (1 << 0),
    FLAG_VIT = (1 << 1),
    FLAG_AGI = (1 << 2),
    FLAG_DEX = (1 << 3),
    FLAG_INT = (1 << 4),
    FLAG_MEN = (1 << 5),
    FLAG_LUK = (1 << 6),
    FLAG_ATTACK_POINT = (1 << 7),
    FLAG_MAGIC_POINT = (1 << 8),
    FLAG_DEFENCE = (1 << 9),
    FLAG_MAGIC_DEFENCE = (1 << 10),
    FLAG_ATTACK_SPEED = (1 << 11),
    FLAG_CAST_SPEED = (1 << 12),
    FLAG_MOVE_SPEED = (1 << 13),
    FLAG_ACCURACY = (1 << 14),
    FLAG_MAGIC_ACCURACY = (1 << 15),
    FLAG_CRITICAL = (1 << 16),
    FLAG_BLOCK = (1 << 17),
    FLAG_BLOCK_DEFENCE = (1 << 18),
    FLAG_AVOID = (1 << 19),
    FLAG_MAGIC_RESISTANCE = (1 << 20),
    FLAG_MAX_HP = (1 << 21),
    FLAG_MAX_MP = (1 << 22),
    FLAG_MAX_SP = (1 << 23),
    FLAG_HP_REGEN_ADD = (1 << 24),
    FLAG_MP_REGEN_ADD = (1 << 25),
    FLAG_SP_REGEN_ADD = (1 << 26),
    FLAG_HP_REGEN_RATIO = (1 << 27),
    FLAG_MP_REGEN_RATIO = (1 << 28),
    FLAG_MAX_WEIGHT = (1 << 30),
};

enum StateEquipFlag
{
    FLAG_EQUIP_ONEHAND_SWORD = (1 << 0),
    FLAG_EQUIP_TWOHAND_SWORD = (1 << 1),
    FLAG_EQUIP_DAGGER = (1 << 2),
    FLAG_EQUIP_TWOHAND_SPEAR = (1 << 3),
    FLAG_EQUIP_TWOHAND_AXE = (1 << 4),
    FLAG_EQUIP_ONEHAND_MACE = (1 << 5),
    FLAG_EQUIP_TWOHAND_MACE = (1 << 6),
    FLAG_EQUIP_HEAVY_BOW = (1 << 7),
    FLAG_EQUIP_LIGHT_BOW = (1 << 8),
    FLAG_EQUIP_CROSSBOW = (1 << 9),
    FLAG_EQUIP_ONEHAND_STAFF = (1 << 10),
    FLAG_EQUIP_TWOHAND_STAFF = (1 << 11),
    FLAG_EQUIP_DOUBLE_SWORD = (1 << 12),
    FLAG_EQUIP_DOUBLE_DAGGER = (1 << 13),
};

enum StateBaseEffect
{
    BEF_NONE = 0,
    BEF_PHYSICAL_STATE_DAMAGE = 1,
    BEF_PHYSICAL_IGNORE_DEFENCE_STATE_DAMAGE = 2,
    BEF_MAGICAL_STATE_DAMAGE = 3,
    BEF_MAGICAL_IGNORE_RESIST_STATE_DAMAGE = 4,

    BEF_PHYSICAL_IGNORE_DEFENCE_PER_STATE_DAMAGE = 6,

    BEF_HEAL_HP_BY_MAGIC = 11,
    BEF_HEAL_MP_BY_MAGIC = 12,
    BEF_HEAL_SP_BY_MAGIC = 13,
    BEF_HEAL_HP_BY_ITEM = 21,
    BEF_HEAL_MP_BY_ITEM = 22,
    BEF_HEAL_SP_BY_ITEM = 23,
    BEF_HEAL_HPMP_BY_ITEM = 24,
    BEF_HEAL_HPMP_PER_BY_ITEM = 25,

    BEF_POISON = 51,
    BEF_VENOM = 52,
    BEF_BLOODY = 53,
    BEF_SERIOUS_BLOODY = 54,
};

enum StateEffect : int
{
    SEF_MISC = 0,
    SEF_PARAMETER_INC = 1,
    SEF_PARAMETER_AMP = 2,
    SEF_PARAMETER_INC_WHEN_EQUIP_SHIELD = 3,
    SEF_PARAMETER_AMP_WHEN_EQUIP_SHIELD = 4,
    SEF_PARAMETER_INC_WHEN_EQUIP = 5,
    SEF_PARAMETER_AMP_WHEN_EQUIP = 6,
    SEF_DOUBLE_ATTACK = 21,
    SEF_ADDITIONAL_DAMAGE_ON_ATTACK = 22,
    SEF_AMP_ADDITIONAL_DAMAGE_ON_ATTACK = 23,
    SEF_ADDITIONAL_DAMAGE_ON_SKILL = 24,
    SEF_AMP_ADDTIONAL_DAMAGE_ON_SKILL = 25,
    SEF_ADD_STATE_ON_ATTACK_OLD = 26,
    SEF_ADD_HP_ON_ATTACK = 27,
    SEF_AMP_RECEIVE_DAMAGE = 30,
    SEF_ADD_HP_MP_ON_CRITICAL = 34,
    SEF_HEALING_AMPLIFY = 35,
    SEF_ADD_STATE_ON_ATTACK = 36,
    SEF_ADD_STATE_BY_SELF_ON_ATTACK = 37,
    SEF_ADD_STATE_ON_BEING_ATTACKED = 38,
    SEF_ADD_STATE_BY_SELF_ON_BEING_ATTACKED = 39,
    SEF_STEAL_WITH_REGEN_STOP = 40,
    SEF_ABSORB = 41,
    SEF_STEAL = 42,
    SEF_DAMAGE_REFLECT_PERCENT = 43,
    SEF_DAMAGE_REFLECT = 44,
    SEF_DAMAGE_REFLECT_WHEN_EQUIP_SHIELD = 45,
    SEF_DAMAGE_REDUCE_WITH_RACE_BY_PERCENT = 46,
    SEF_DAMAGE_REDUCE_WITH_RACE_BY_VALUE = 47,
    //SEF_DAMAGE_REDUCE						= 48,
    SEF_MANA_SHIELD = 49,
    SEF_HEAL = 61,
    SEF_REGEN_ADD = 62,
    SEF_HEAL_BY_ITEM = 63,
    SEF_HEAL_HPMP_PER = 64,
    SEF_FORCE_CHIP = 71,
    SEF_SOUL_CHIP = 72,
    SEF_HEALING_CHIP = 73,
    SEF_LUNAR_CHIP = 74,
    SEF_NEUTRALIZE = 81,
    SEF_MEZZ = 82,
    SEF_PROVOKE = 83,
    SEF_DECREASE_STATE_EFFECT = 84,
    SEF_INC_HATE = 85,
    SEF_AMP_HATE = 86,
    SEF_SKILL_INTERRUPTION = 87,
    SEF_ADD_REGION_STATE = 88,
    SEF_MP_COST_INC = 91,
    SEF_ADD_PARAMETER_ON_NORMAL_ATTACK = 92,
    SEF_ADD_PARAMETER_ON_SKILL = 93,
    SEF_KNOCKBACK_ON_DEFAULT_ATTACK = 94,
    SEF_REMOVE_GOOD_STATE = 97,
    SEF_TRANSFORMATION = 104,
    SEF_DEAL_DAMAGE = 105,
    SEF_AMPLIFY_CASTING_TIME = 106,
    SEF_RESURRECTION = 109,
    SEF_DETECT_HIDE = 111,
    SEF_CREATURE_PARAMETER_AMP = 112,
    SEF_GIVE_EXP_JP_TO_CREATURE = 113,
    SEF_CHANGING_FORM = 114,
    SEF_RIDING = 200,
    SEF_AMP_AND_INC_ITEM_CHANCE = 201,
};

enum StateTagType
{
    STT_NORMAL_ATTACK = 1 << 0,
    STT_PHYSICAL_SKILL = 1 << 1,
    STT_MAGICAL_SKILL = 1 << 2,
    STT_HELPFUL = 1 << 3,
    STT_HARMFUL = 1 << 4,
};

struct StateTemplate
{
    int state_id;
    int text_id;
    int tooltip_id;
    uint8 is_harmful;
    int state_time_type;
    int state_group;
    int duplicate_group[3]{0};
    uint8 uf_avatar;
    uint8 uf_summon;
    uint8 uf_monster;
    int base_effect_id;
    int fire_interval;
    int elemental_type;
    float amplify_base;
    float amplify_per_skl;
    int add_damage_base;
    int add_damage_per_skl;
    int effect_type;
    float value[20]{0};
};