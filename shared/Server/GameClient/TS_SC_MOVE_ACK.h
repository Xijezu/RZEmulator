#ifndef PACKETS_TS_SC_MOVE_ACK_H
#define PACKETS_TS_SC_MOVE_ACK_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_MOVE_ACK_DEF(_) \
	_(simple)(uint32_t, time) \
	_(simple)(int8_t, speed)

CREATE_PACKET(TS_SC_MOVE_ACK, 6);

#endif // PACKETS_TS_SC_MOVE_ACK_H
