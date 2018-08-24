#ifndef PACKETS_TS_SC_BELT_SLOT_INFO_H
#define PACKETS_TS_SC_BELT_SLOT_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_BELT_SLOT_INFO_DEF(_) \
	_(def)(array) (uint32_t, handle, 8) \
	  _(impl)(array) (uint32_t, handle, 6, version < EPIC_9_5) \
	  _(impl)(array) (uint32_t, handle, 8, version >= EPIC_9_5) \

CREATE_PACKET(TS_SC_BELT_SLOT_INFO, 216);

#endif // PACKETS_TS_SC_BELT_SLOT_INFO_H
