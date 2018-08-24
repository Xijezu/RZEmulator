#ifndef PACKETS_TS_SC_STAT_INFO_H
#define PACKETS_TS_SC_STAT_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_STAT_INFO_BASE_DEF(_) \
  _(simple)(int16_t, stat_id) \
  _(simple)(int16_t, strength) \
  _(simple)(int16_t, vital) \
  _(simple)(int16_t, dexterity) \
  _(simple)(int16_t, agility) \
  _(simple)(int16_t, intelligence) \
  _(simple)(int16_t, mentality) \
  _(simple)(int16_t, luck)
CREATE_STRUCT(TS_STAT_INFO_BASE);

// Attacks changed to 32 bits on 14/06/2016 (during epic 9.3)
#define TS_STAT_INFO_ATTRIB_DEF(_) \
  _(simple)(int16_t, nCritical) \
  _(simple)(int16_t, nCriticalPower) \
  _(def)(simple)(int32_t, nAttackPointRight) \
  _(impl)(simple)(int32_t, nAttackPointRight, version >= EPIC_9_3) \
  _(impl)(simple)(int16_t, nAttackPointRight, version < EPIC_9_3) \
  _(def)(simple)(int32_t, nAttackPointLeft) \
  _(impl)(simple)(int32_t, nAttackPointLeft, version >= EPIC_9_3) \
  _(impl)(simple)(int16_t, nAttackPointLeft, version < EPIC_9_3) \
  _(def)(simple)(int32_t, nDefence) \
  _(impl)(simple)(int32_t, nDefence, version >= EPIC_9_5_1) \
  _(impl)(simple)(int16_t, nDefence, version < EPIC_9_5_1) \
  _(def)(simple)(int32_t, nBlockDefence) \
  _(impl)(simple)(int32_t, nBlockDefence, version >= EPIC_9_5_1) \
  _(impl)(simple)(int16_t, nBlockDefence, version < EPIC_9_5_1) \
  _(def)(simple)(int32_t, nMagicPoint) \
  _(impl)(simple)(int32_t, nMagicPoint, version >= EPIC_9_3) \
  _(impl)(simple)(int16_t, nMagicPoint, version < EPIC_9_3) \
  _(def)(simple)(int32_t, nMagicDefence) \
  _(impl)(simple)(int32_t, nMagicDefence, version >= EPIC_9_5_1) \
  _(impl)(simple)(int16_t, nMagicDefence, version < EPIC_9_5_1) \
  _(simple)(int16_t, nAccuracyRight) \
  _(simple)(int16_t, nAccuracyLeft) \
  _(simple)(int16_t, nMagicAccuracy) \
  _(simple)(int16_t, nAvoid) \
  _(simple)(int16_t, nMagicAvoid) \
  _(simple)(int16_t, nBlockChance) \
  _(simple)(int16_t, nMoveSpeed) \
  _(simple)(int16_t, nAttackSpeed) \
  _(simple)(int16_t, nAttackRange) \
  _(def)(simple)(int32_t, nMaxWeight) \
  _(impl)(simple)(int32_t, nMaxWeight, version >= EPIC_7_4) \
  _(impl)(simple)(int16_t, nMaxWeight, version < EPIC_7_4) \
  _(simple)(int16_t, nCastingSpeed) \
  _(simple)(int16_t, nCoolTimeSpeed) \
  _(simple)(int16_t, nItemChance) \
  _(simple)(int16_t, nHPRegenPercentage) \
  _(simple)(int16_t, nHPRegenPoint) \
  _(simple)(int16_t, nMPRegenPercentage) \
  _(simple)(int16_t, nMPRegenPoint) \
  _(simple)(int16_t, nPerfectBlock, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nMagicalDefIgnore, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nMagicalDefIgnoreRatio, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nPhysicalDefIgnore, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nPhysicalDefIgnoreRatio, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nMagicalPenetration, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nMagicalPenetrationRatio, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nPhysicalPenetration, version >= EPIC_7_3, 20) \
  _(simple)(int16_t, nPhysicalPenetrationRatio, version >= EPIC_7_3, 20)
CREATE_STRUCT(TS_STAT_INFO_ATTRIB);

#define TS_SC_STAT_INFO_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(simple) (TS_STAT_INFO_BASE, stat) \
	_(simple) (TS_STAT_INFO_ATTRIB, attribute) \
	_(simple) (uint8_t, type)

CREATE_PACKET(TS_SC_STAT_INFO, 1000);

#endif // PACKETS_TS_SC_STAT_INFO_H
