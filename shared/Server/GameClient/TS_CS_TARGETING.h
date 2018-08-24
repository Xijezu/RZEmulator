#ifndef PACKETS_TS_CS_TARGETING_H
#define PACKETS_TS_CS_TARGETING_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_TARGETING_DEF(_) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_CS_TARGETING, 511);

#endif // PACKETS_TS_CS_TARGETING_H
