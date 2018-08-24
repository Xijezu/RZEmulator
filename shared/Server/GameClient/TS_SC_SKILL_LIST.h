#ifndef PACKETS_TS_SC_SKILL_LIST_H
#define PACKETS_TS_SC_SKILL_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_SKILL_INFO_DEF(_) \
	_(simple)(int32_t, skill_id) \
	_(simple)(int8_t, base_skill_level) \
	_(simple)(int8_t, current_skill_level) \
	_(simple)(uint32_t, total_cool_time) \
	_(simple)(uint32_t, remain_cool_time)

CREATE_STRUCT(TS_SKILL_INFO);

#define TS_SC_SKILL_LIST_DEF(_) \
	_(simple)(uint32_t, target) \
	_(count)(uint16_t, skills) \
	_(simple)(int8_t, modification_type, version >= EPIC_4_1) \
	_(dynarray)(TS_SKILL_INFO, skills)

CREATE_PACKET(TS_SC_SKILL_LIST, 403);

// modification_type: if true then refresh

#endif // PACKETS_TS_SC_SKILL_LIST_H
