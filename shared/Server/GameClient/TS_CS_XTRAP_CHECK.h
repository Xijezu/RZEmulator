#ifndef PACKETS_TS_CS_XTRAP_CHECK_H
#define PACKETS_TS_CS_XTRAP_CHECK_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_XTRAP_CHECK_DEF(_) \
	_(array)(uint8_t, pCheckBuffer, 128)

CREATE_PACKET(TS_CS_XTRAP_CHECK, 59);

#endif // PACKETS_TS_CS_XTRAP_CHECK_H
