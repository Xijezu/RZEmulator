#ifndef PACKETS_TS_SC_XTRAP_CHECK_H
#define PACKETS_TS_SC_XTRAP_CHECK_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_XTRAP_CHECK_DEF(_) \
	_(array)(uint8_t, pCheckBuffer, 128)

CREATE_PACKET(TS_SC_XTRAP_CHECK, 58);

#endif // PACKETS_TS_SC_XTRAP_CHECK_H
