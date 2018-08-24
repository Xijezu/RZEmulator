#ifndef PACKETS_TS_CS_COMPETE_REQUEST_H
#define PACKETS_TS_CS_COMPETE_REQUEST_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_COMPETE_REQUEST_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(string)(requestee, 31)

CREATE_PACKET(TS_CS_COMPETE_REQUEST, 4500);

#endif // PACKETS_TS_CS_COMPETE_REQUEST_H
