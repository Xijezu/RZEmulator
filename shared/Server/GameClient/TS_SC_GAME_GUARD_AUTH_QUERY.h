#ifndef PACKETS_TS_SC_GAME_GUARD_AUTH_QUERY_H
#define PACKETS_TS_SC_GAME_GUARD_AUTH_QUERY_H

#include "Packet/PacketDeclaration.h"

#define TS_GAME_GUARD_AUTH_V1_DEF(_) \
	_(simple)(uint32_t, dwIndex) \
	_(simple)(uint32_t, dwValue1) \
	_(simple)(uint32_t, dwValue2) \
	_(simple)(uint32_t, dwValue3)
CREATE_STRUCT(TS_GAME_GUARD_AUTH_V1);

#define TS_GAME_GUARD_AUTH_V2_DEF(_) \
	_(count)(uint16_t, data) \
	_(simple)(uint16_t, unknown) \
	_(dynarray)(uint8_t, data)
CREATE_STRUCT(TS_GAME_GUARD_AUTH_V2);

#define TS_GAME_GUARD_AUTH_DEF(_) \
	_(simple)(TS_GAME_GUARD_AUTH_V1, authv1, version < EPIC_9_1) \
	_(simple)(TS_GAME_GUARD_AUTH_V2, authv2, version >= EPIC_9_1)
CREATE_STRUCT(TS_GAME_GUARD_AUTH);

#define TS_SC_GAME_GUARD_AUTH_QUERY_DEF(_) \
	_(simple)(TS_GAME_GUARD_AUTH, auth)

CREATE_PACKET(TS_SC_GAME_GUARD_AUTH_QUERY, 55);

#endif // PACKETS_TS_SC_GAME_GUARD_AUTH_QUERY_H
