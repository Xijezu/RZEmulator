#ifndef PACKETS_TS_TIMESYNC_H
#define PACKETS_TS_TIMESYNC_H

#include "Packet/PacketDeclaration.h"

#define TS_TIMESYNC_DEF(_) \
	_(simple) (uint32_t, time)

CREATE_PACKET(TS_TIMESYNC, 2);

#endif // PACKETS_TS_TIMESYNC_H
