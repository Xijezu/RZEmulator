#ifndef PACKETS_TS_CS_GET_REGION_INFO_H
#define PACKETS_TS_CS_GET_REGION_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_GET_REGION_INFO_DEF(_) \
	_(simple)(float, x) \
	_(simple)(float, y)

CREATE_PACKET(TS_CS_GET_REGION_INFO, 550);

#endif // PACKETS_TS_CS_GET_REGION_INFO_H
