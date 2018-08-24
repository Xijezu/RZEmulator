#ifndef PACKETS_TS_SC_SKILL_H
#define PACKETS_TS_SC_SKILL_H

#include "Packet/PacketDeclaration.h"

// Some enum with specified underlying type (after ":")
enum TS_SKILL__TYPE : uint8_t
{
	ST_Fire = 0,
	ST_Casting = 1,
	ST_CastingUpdate = 2,
	ST_Cancel = 3,
	ST_RegionFire = 4,
	ST_Complete = 5
};

enum TS_SKILL__HIT_TYPE : uint8_t
{
	SHT_DAMAGE = 0,
	SHT_MAGIC_DAMAGE = 1,
	SHT_DAMAGE_WITH_KNOCK_BACK = 2,
	SHT_RESULT = 10,
	SHT_ADD_HP = 20,
	SHT_ADD_MP = 21,
	SHT_ADD_HP_MP_SP = 22,
	SHT_REBIRTH = 23,
	SHT_RUSH = 30,
	SHT_CHAIN_DAMAGE = 40,
	SHT_CHAIN_MAGIC_DAMAGE = 41,
	SHT_CHAIN_HEAL = 42,
	SHT_NOT_USE = 100
};

enum TS_SKILL__DAMAGE_TYPE : uint8_t
{
	SDT_TYPE_NONE = 0,
	SDT_TYPE_FIRE = 1,
	SDT_TYPE_WATER = 2,
	SDT_TYPE_WIND = 3,
	SDT_TYPE_EARTH = 4,
	SDT_TYPE_LIGHT = 5,
	SDT_TYPE_DARK = 6,
	SDT_TYPE_COUNT = 7
};

enum TS_SKILL_RESULT_SUCESS_TYPE : int32_t {
	SRST_AddState = 10,
	SRST_RemoveState = 11,
	SRST_AddHate = 12,
	SRST_TurnOn = 21,
	SRST_TurnOff = 22,
	SRST_SummonDead = 30,
	SRST_TargetDead = 31,
	SRST_CreateItem = 40,
	SRST_RespawnMonster = 41
};

// Structure definitions
#define TS_SC_SKILL__HIT_DAMAGE_INFO_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (TS_SKILL__DAMAGE_TYPE, damage_type) \
	_(simple) (int32_t, damage) \
	_(simple) (int32_t, flag) \
	_(array)  (uint16_t, elemental_damage, 7)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE_INFO);

#define TS_SC_SKILL__HIT_DAMAGE_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE);

#define TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple)(int8_t, speed) \
	_(simple) (uint32_t, knock_back_time)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK);

#define TS_SC_SKILL__HIT_RESULT_DEF(_) \
	_(simple) (bool, bResult) \
	_(simple) (TS_SKILL_RESULT_SUCESS_TYPE, success_type)
CREATE_STRUCT(TS_SC_SKILL__HIT_RESULT);

#define TS_SC_SKILL__HIT_ADD_STAT_DEF(_) \
	_(simple) (int32_t, target_stat) \
	_(simple) (int32_t, nIncStat)
CREATE_STRUCT(TS_SC_SKILL__HIT_ADD_STAT);

#define TS_SC_SKILL__HIT_ADDHPMPSP_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (int32_t, nIncMP) \
	_(simple) (int32_t, nIncSP) \
	_(def)(simple)(int32_t, target_mp) \
	  _(impl)(simple)(int32_t, target_mp, version >= EPIC_7_1) \
	  _(impl)(simple)(int16_t, target_mp, version < EPIC_7_1)
CREATE_STRUCT(TS_SC_SKILL__HIT_ADDHPMPSP);

#define TS_SC_SKILL__HIT_REBIRTH_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (int32_t, nIncMP) \
	_(simple) (int32_t, nRecoveryEXP) \
	_(def)(simple)(int32_t, target_mp) \
	  _(impl)(simple)(int32_t, target_mp, version >= EPIC_7_1) \
	  _(impl)(simple)(int16_t, target_mp, version < EPIC_7_1)
CREATE_STRUCT(TS_SC_SKILL__HIT_REBIRTH);

#define TS_SC_SKILL__HIT_RUSH_DEF(_) \
	_(simple) (bool, bResult) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple) (float, face) \
	_(simple) (int8_t, speed)
CREATE_STRUCT(TS_SC_SKILL__HIT_RUSH);

#define TS_SC_SKILL__HIT_CHAIN_DAMAGE_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage) \
	_(simple) (uint32_t, hFrom)
CREATE_STRUCT(TS_SC_SKILL__HIT_CHAIN_DAMAGE);

#define TS_SC_SKILL__HIT_CHAIN_HEAL_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (uint32_t, hFrom)
CREATE_STRUCT(TS_SC_SKILL__HIT_CHAIN_HEAL);

#define TS_SC_SKILL__HIT_DETAILS_DEF(_) \
	_(simple) (TS_SKILL__HIT_TYPE, type) \
	_(simple) (uint32_t, hTarget) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE               , hitDamage             , type == SHT_DAMAGE || type == SHT_MAGIC_DAMAGE) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK, hitDamageWithKnockBack, type == SHT_DAMAGE_WITH_KNOCK_BACK) \
	_(simple) (TS_SC_SKILL__HIT_RESULT               , hitResult             , type == SHT_RESULT) \
	_(simple) (TS_SC_SKILL__HIT_ADD_STAT             , hitAddStat            , type == SHT_ADD_HP || type == SHT_ADD_MP) \
	_(simple) (TS_SC_SKILL__HIT_ADDHPMPSP            , hitAddHPMPSP          , type == SHT_ADD_HP_MP_SP) \
	_(simple) (TS_SC_SKILL__HIT_REBIRTH              , hitRebirth            , type == SHT_REBIRTH) \
	_(simple) (TS_SC_SKILL__HIT_RUSH                 , hitRush               , type == SHT_RUSH) \
	_(simple) (TS_SC_SKILL__HIT_CHAIN_DAMAGE         , hitChainDamage        , type == SHT_CHAIN_DAMAGE || type == SHT_CHAIN_MAGIC_DAMAGE) \
	_(simple) (TS_SC_SKILL__HIT_CHAIN_HEAL           , hitChainHeal          , type == SHT_CHAIN_HEAL)
CREATE_STRUCT(TS_SC_SKILL__HIT_DETAILS);

#define TS_SC_SKILL__FIRE_DEF(_) \
	_(simple) (bool, bMultiple) \
	_(simple) (float, range) \
	_(simple) (int8_t, target_count) \
	_(simple) (int8_t, fire_count) \
	_(count)  (uint16_t, hits) \
	_(padmarker)(before_hit_marker) \
	_(dynarray)(TS_SC_SKILL__HIT_DETAILS, hits) \
	_(pad)    (45*(uint32_t)hits.size(), before_hit_marker) /* weird fixed padding with fire struct in assembleMessage */
CREATE_STRUCT(TS_SC_SKILL__FIRE);

#define TS_SC_SKILL__CAST_DEF(_) \
	_(simple) (uint32_t, tm) \
	_(simple) (uint16_t, nErrorCode)
CREATE_STRUCT(TS_SC_SKILL__CAST);

// Packet definition
#define TS_SC_SKILL_DEF(_) \
	_(simple) (uint16_t, skill_id) \
	_(simple) (uint8_t, skill_level) \
	_(simple) (uint32_t, caster) \
	_(simple) (uint32_t, target) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple) (float, z) \
	_(simple) (uint8_t, layer) \
	_(simple) (TS_SKILL__TYPE, type) \
	_(def)(simple)(int32_t, hp_cost) \
	  _(impl)(simple)(int32_t, hp_cost, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, hp_cost, version < EPIC_7_3) \
	_(def)(simple)(int32_t, mp_cost) \
	  _(impl)(simple)(int32_t, mp_cost, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, mp_cost, version < EPIC_7_3) \
	_(simple) (int32_t, caster_hp) \
	_(def)(simple)(int32_t, caster_mp) \
	  _(impl)(simple)(int32_t, caster_mp, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, caster_mp, version < EPIC_7_3) \
	_(padmarker)(skill_type_marker) \
	_(simple) (TS_SC_SKILL__FIRE , fire   , type == ST_Fire || type == ST_RegionFire) \
	_(simple) (TS_SC_SKILL__CAST , casting, type == ST_Casting || type == ST_CastingUpdate) \
	_(pad)    (9, skill_type_marker) /* padding to match fire size */

CREATE_PACKET(TS_SC_SKILL, 401);

#endif // PACKETS_TS_SC_ENTER_H
