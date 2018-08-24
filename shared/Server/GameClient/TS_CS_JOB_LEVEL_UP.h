#ifndef PACKETS_TS_CS_JOB_LEVEL_UP_H
#define PACKETS_TS_CS_JOB_LEVEL_UP_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_JOB_LEVEL_UP_DEF(_) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_CS_JOB_LEVEL_UP, 410);

#endif // PACKETS_TS_CS_JOB_LEVEL_UP_H
