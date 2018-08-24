#ifndef PACKETS_TS_SC_COMPETE_START_H
#define PACKETS_TS_SC_COMPETE_START_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_COMPETE_START_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(string)(competitor, 31)

CREATE_PACKET(TS_SC_COMPETE_START, 4505);

#endif // PACKETS_TS_SC_COMPETE_START_H
