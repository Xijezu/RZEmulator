#ifndef PACKETS_TS_CS_UPDATE_H
#define PACKETS_TS_CS_UPDATE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_UPDATE_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(simple) (uint32_t, time, version >= EPIC_9_2, 0) \
	_(simple) (uint32_t, epoch_time, version >= EPIC_9_2, 0)

CREATE_PACKET(TS_CS_UPDATE, 503);

#endif // PACKETS_TS_CS_UPDATE_H
