#ifndef PACKETS_TS_SC_COMPETE_REQUEST_H
#define PACKETS_TS_SC_COMPETE_REQUEST_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_COMPETE_REQUEST_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(string)(requester, 31)

CREATE_PACKET(TS_SC_COMPETE_REQUEST, 4501);

#endif // PACKETS_TS_SC_COMPETE_REQUEST_H
