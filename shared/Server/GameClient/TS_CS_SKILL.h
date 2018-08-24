#ifndef PACKETS_TS_CS_SKILL_H
#define PACKETS_TS_CS_SKILL_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SKILL_DEF(_) \
	_(simple)(uint16_t, skill_id) \
	_(simple)(uint32_t, caster) \
	_(simple)(uint32_t, target) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(float, z) \
	_(simple)(int8_t, layer) \
	_(simple)(int8_t, skill_level)

CREATE_PACKET(TS_CS_SKILL, 400);

#endif // PACKETS_TS_CS_SKILL_H
