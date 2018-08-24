#ifndef PACKETS_TS_CS_VERSION_H
#define PACKETS_TS_CS_VERSION_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_VERSION_DEF(_) \
	_(string)(szVersion, 20) \
	_(array)(uint8_t, checksum, 256, version >= EPIC_9_5_2)

#define TS_CS_VERSION_ID(X) \
	X(50, version < EPIC_7_4) \
	X(51, version >= EPIC_7_4)

CREATE_PACKET_VER_ID(TS_CS_VERSION);

#endif // PACKETS_TS_CS_VERSION_H
