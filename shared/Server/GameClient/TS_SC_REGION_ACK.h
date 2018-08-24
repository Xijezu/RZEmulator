#ifndef PACKETS_TS_SC_REGION_ACK_H
#define PACKETS_TS_SC_REGION_ACK_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_REGION_ACK_DEF(_) \
	_(simple)(int32_t, rx) \
	_(simple)(int32_t, ry)

CREATE_PACKET(TS_SC_REGION_ACK, 11);

#endif // PACKETS_TS_SC_REGION_ACK_H
