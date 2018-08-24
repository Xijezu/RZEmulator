#ifndef PACKETS_TS_SC_LEAVE_H
#define PACKETS_TS_SC_LEAVE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_LEAVE_DEF(_) \
	_(simple)(uint32_t, handle)

CREATE_PACKET(TS_SC_LEAVE, 9);

#endif // PACKETS_TS_SC_LEAVE_H
