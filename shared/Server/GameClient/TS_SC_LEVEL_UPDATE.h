#ifndef PACKETS_TS_SC_LEVEL_UPDATE_H
#define PACKETS_TS_SC_LEVEL_UPDATE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_LEVEL_UPDATE_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(simple) (int32_t, level) \
	_(simple) (int32_t, job_level)

CREATE_PACKET(TS_SC_LEVEL_UPDATE, 1002);

#endif // PACKETS_TS_SC_LEVEL_UPDATE_H
