#ifndef PACKETS_TS_SC_STATUS_CHANGE_H
#define PACKETS_TS_SC_STATUS_CHANGE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_STATUS_CHANGE_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(simple) (uint32_t, status)

CREATE_PACKET(TS_SC_STATUS_CHANGE, 500);

#endif // PACKETS_TS_SC_STATUS_CHANGE_H
