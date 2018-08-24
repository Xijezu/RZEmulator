#ifndef PACKETS_TS_SC_ITEM_COOL_TIME_H
#define PACKETS_TS_SC_ITEM_COOL_TIME_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ITEM_COOL_TIME_DEF(_) \
	_(def)(array) (uint32_t, cool_time, 40) \
	_(impl)(array)(uint32_t, cool_time, 40, version >= EPIC_6_2) \
	_(impl)(array)(uint32_t, cool_time, 20, version < EPIC_6_2)

CREATE_PACKET(TS_SC_ITEM_COOL_TIME, 217);

#endif // PACKETS_TS_SC_ITEM_COOL_TIME_H
