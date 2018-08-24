#ifndef PACKETS_TS_CS_MOVE_REQUEST_H
#define PACKETS_TS_CS_MOVE_REQUEST_H

#include "Packet/PacketDeclaration.h"

#define MOVE_REQUEST_INFO_DEF(_) \
	_(simple)(float, tx) \
	_(simple)(float, ty)

CREATE_STRUCT(MOVE_REQUEST_INFO);

#define TS_CS_MOVE_REQUEST_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(uint32_t, cur_time) \
	_(simple)(bool, speed_sync) \
	_(count) (uint16_t, move_infos) \
	_(dynarray)(MOVE_REQUEST_INFO, move_infos)

#define TS_CS_MOVE_REQUEST_ID(X) \
	X(5, version < EPIC_9_2) \
	X(65, version >= EPIC_9_2 && version < EPIC_9_4_2) \
	X(63, version >= EPIC_9_4_2)

CREATE_PACKET_VER_ID(TS_CS_MOVE_REQUEST);

#endif // PACKETS_TS_CS_MOVE_REQUEST_H
