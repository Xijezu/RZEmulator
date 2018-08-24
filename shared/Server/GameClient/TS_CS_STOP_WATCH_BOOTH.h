#ifndef PACKETS_TS_CS_STOP_WATCH_BOOTH_H
#define PACKETS_TS_CS_STOP_WATCH_BOOTH_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_STOP_WATCH_BOOTH_DEF(_) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_CS_STOP_WATCH_BOOTH, 704);

#endif // PACKETS_TS_CS_STOP_WATCH_BOOTH_H
