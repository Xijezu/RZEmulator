#ifndef PACKETS_TS_SC_BOOTH_CLOSED_H
#define PACKETS_TS_SC_BOOTH_CLOSED_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_BOOTH_CLOSED_DEF(_) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_SC_BOOTH_CLOSED, 709);

#endif // PACKETS_TS_SC_BOOTH_CLOSED_H
