#ifndef PACKETS_TS_SC_ANTI_HACK_H
#define PACKETS_TS_SC_ANTI_HACK_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ANTI_HACK_DEF(_) \
	_(simple)(uint16_t, nLength) \
	_(array)(uint8_t, byBuffer, 400)

CREATE_PACKET(TS_SC_ANTI_HACK, 53);

#endif // PACKETS_TS_SC_ANTI_HACK_H
