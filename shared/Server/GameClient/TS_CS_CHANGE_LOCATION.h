#ifndef PACKETS_TS_CS_CHANGE_LOCATION_H
#define PACKETS_TS_CS_CHANGE_LOCATION_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_CHANGE_LOCATION_DEF(_) \
	_(simple) (float, x) \
	_(simple) (float, y)

CREATE_PACKET(TS_CS_CHANGE_LOCATION, 900);

#endif // PACKETS_TS_CS_CHANGE_LOCATION_H
