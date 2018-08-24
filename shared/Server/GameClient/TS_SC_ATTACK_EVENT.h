#ifndef PACKETS_TS_SC_ATTACK_EVENT_H
#define PACKETS_TS_SC_ATTACK_EVENT_H

#include "Packet/PacketDeclaration.h"

enum ATTACK_EVENT__ATTACK_FLAG : uint8_t {
	AEAF_None = 0,
	AEAF_UsingBow = 1,
	AEAF_UsingCrossBow = 2,
	AEAF_DoubleWeapon = 4,
	AEAF_DoubleAttack = 8
};

enum ATTACK_EVENT__ATTACK_ACTION : uint8_t {
	AEAA_EndAttack = 1,
	AEAA_Aiming = 2,
	AEAA_Attack = 3,
	AEAA_CancelAttack = 4
};

enum ATTACK_INFO__FLAG : int8_t {
	AIF_PerfectBlock = 1,
	AIF_Block = 2,
	AIF_Miss = 4,
	AIF_Critical = 8
};

#define ATTACK_INFO_DEF(_) \
	_(def)(simple)(int32_t, damage) \
	  _(impl)(simple)(int32_t, damage, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, damage, version < EPIC_7_3) \
	_(def)(simple)(int32_t, mp_damage) \
	  _(impl)(simple)(int32_t, mp_damage, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, mp_damage, version < EPIC_7_3) \
	_(simple) (ATTACK_INFO__FLAG, flag) \
	_(def)(array)(int32_t, elemental_damage, 7) \
	  _(impl)(array)(int32_t, elemental_damage, 7, version >= EPIC_7_3) \
	  _(impl)(array)(uint16_t, elemental_damage, 7, version < EPIC_7_3) \
	_(simple) (int32_t, target_hp) \
	_(def)(simple)(int32_t, target_mp) \
	  _(impl)(simple)(int32_t, target_mp, version >= EPIC_7_3) \
	  _(impl)(simple)(uint16_t, target_mp, version < EPIC_7_3) \
	_(def)(simple)(int32_t, attacker_damage) \
	  _(impl)(simple)(int32_t, attacker_damage, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, attacker_damage, version < EPIC_7_3) \
	_(def)(simple)(int32_t, attacker_mp_damage) \
	  _(impl)(simple)(int32_t, attacker_mp_damage, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, attacker_mp_damage, version < EPIC_7_3) \
	_(simple) (int32_t, attacker_hp) \
	_(def)(simple)(int32_t, attacker_mp) \
	  _(impl)(simple)(int32_t, attacker_mp, version >= EPIC_7_3) \
	  _(impl)(simple)(uint16_t, attacker_mp, version < EPIC_7_3)

CREATE_STRUCT(ATTACK_INFO);

#define TS_SC_ATTACK_EVENT_DEF(_) \
	_(simple)   (uint32_t, attacker_handle) \
	_(simple)   (uint32_t, target_handle) \
	_(simple)   (uint16_t, attack_speed) \
	_(simple)   (uint16_t, attack_delay) \
	_(simple)   (ATTACK_EVENT__ATTACK_ACTION, attack_action) \
	_(simple)   (ATTACK_EVENT__ATTACK_FLAG, attack_flag) \
	_(count)    (uint8_t, attack) \
	_(dynarray) (ATTACK_INFO, attack)

CREATE_PACKET(TS_SC_ATTACK_EVENT, 101);

#endif // PACKETS_TS_SC_ATTACK_EVENT_H
