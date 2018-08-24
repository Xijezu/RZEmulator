#ifndef PACKETS_TS_SC_SET_TIME_H
#define PACKETS_TS_SC_SET_TIME_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SET_TIME_DEF(_) \
	_(simple)(int32_t, gap)

CREATE_PACKET(TS_SC_SET_TIME, 10);

#endif // PACKETS_TS_SC_SET_TIME_H
