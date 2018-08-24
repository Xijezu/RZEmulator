#ifndef PACKETS_TS_SC_SP_H
#define PACKETS_TS_SC_SP_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SP_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int16_t, sp) \
	_(simple)(int16_t, max_sp)

CREATE_PACKET(TS_SC_SP, 514);

#endif // PACKETS_TS_SC_SP_H
