#ifndef PACKETS_TS_SC_TARGET_H
#define PACKETS_TS_SC_TARGET_H

#include "Packet/PacketDeclaration.h"

// Seems unused
#define TS_SC_TARGET_DEF(_) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_SC_TARGET, 512);

#endif // PACKETS_TS_SC_TARGET_H
