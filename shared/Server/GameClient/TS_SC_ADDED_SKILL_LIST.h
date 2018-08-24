#ifndef PACKETS_TS_SC_ADDED_SKILL_LIST_H
#define PACKETS_TS_SC_ADDED_SKILL_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_ADDED_SKILL_LIST_DEF(_) \
	_(simple) (uint32_t, skill_id) \
	_(simple) (bool, restricted_to_type) \
	_(simple) (uint8_t, added_skill_level)
CREATE_STRUCT(TS_ADDED_SKILL_LIST);

#define TS_SC_ADDED_SKILL_LIST_DEF(_) \
	_(simple)  (uint32_t, target) \
	_(count)   (uint16_t, skills) \
	_(dynarray)(TS_ADDED_SKILL_LIST, skills)

CREATE_PACKET(TS_SC_ADDED_SKILL_LIST, 404);

#endif // PACKETS_TS_SC_ADDED_SKILL_LIST_H
