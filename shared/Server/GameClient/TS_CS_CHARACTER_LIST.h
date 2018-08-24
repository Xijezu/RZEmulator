#ifndef PACKETS_TS_CS_CHARACTER_LIST_H
#define PACKETS_TS_CS_CHARACTER_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_CHARACTER_LIST_DEF(_) \
	_(def)(string)(account, 61) \
	_(impl)(string)(account, 61, version >= EPIC_5_2) \
	_(impl)(string)(account, 19, version <  EPIC_5_2)

// test server use packet id 2008 ?
#define TS_CS_CHARACTER_LIST_ID(X) \
	X(2001, version < EPIC_9_4) \
	X(2007, version >= EPIC_9_4 && version != EPIC_9_4_AR) \
	X(2179, version == EPIC_9_4_AR)

CREATE_PACKET_VER_ID(TS_CS_CHARACTER_LIST);

#endif // PACKETS_TS_CS_CHARACTER_LIST_H
