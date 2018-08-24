#ifndef PACKETS_TS_CS_QUERY_H
#define PACKETS_TS_CS_QUERY_H

#include "Packet/PacketDeclaration.h"

// Sent by the client when receiving a packet about an unknown handle.
// This trigger a TS_SC_ENTER from the GS
#define TS_CS_QUERY_DEF(_) \
	_(simple)(uint32_t, handle) 

CREATE_PACKET(TS_CS_QUERY, 13);

#endif // PACKETS_TS_CS_QUERY_H
