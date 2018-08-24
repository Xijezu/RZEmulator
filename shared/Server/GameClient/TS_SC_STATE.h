#ifndef PACKETS_TS_SC_STATE_H
#define PACKETS_TS_SC_STATE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_STATE_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(uint16_t, state_handle) \
	_(def)(simple) (uint16_t, state_level) \
	_(impl)(simple)(uint16_t, state_level, version >= EPIC_9_5_2) \
	_(simple)(uint32_t, state_code) \
	_(impl)(simple)(uint16_t, state_level, version >= EPIC_4_1 && version < EPIC_9_5_2) \
	_(impl)(simple)(int8_t, state_level, version < EPIC_4_1) \
	_(simple)(uint32_t, end_time) \
	_(simple)(uint32_t, start_time, version >= EPIC_3) \
	_(simple)(int32_t, state_value, version >= EPIC_4_1) \
	_(string)(state_string_value, 32, version >= EPIC_4_1 && version < EPIC_9_5_2)

CREATE_PACKET(TS_SC_STATE, 505);

#endif // PACKETS_TS_SC_STATE_H
