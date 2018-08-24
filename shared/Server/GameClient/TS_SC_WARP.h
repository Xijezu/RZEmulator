#ifndef PACKETS_TS_SC_WARP_H
#define PACKETS_TS_SC_WARP_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_WARP_DEF(_) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(float, z) \
	_(simple)(int8_t, layer)

CREATE_PACKET(TS_SC_WARP, 12);

#endif // PACKETS_TS_SC_WARP_H
