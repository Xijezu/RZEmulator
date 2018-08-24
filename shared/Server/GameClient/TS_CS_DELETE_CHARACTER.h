#ifndef PACKETS_TS_CS_DELETE_CHARACTER_H
#define PACKETS_TS_CS_DELETE_CHARACTER_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_DELETE_CHARACTER_DEF(_) \
	_(def)(string)(name, 38) \
	_(impl)(string)(name, 38, version >= EPIC_8_1) \
	_(impl)(string)(name, 19, version <  EPIC_8_1)

CREATE_PACKET(TS_CS_DELETE_CHARACTER, 2003);

#endif // PACKETS_TS_CS_DELETE_CHARACTER_H
