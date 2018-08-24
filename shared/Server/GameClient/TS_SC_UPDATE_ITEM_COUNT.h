#ifndef PACKETS_TS_SC_UPDATE_ITEM_COUNT_H
#define PACKETS_TS_SC_UPDATE_ITEM_COUNT_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_UPDATE_ITEM_COUNT_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(def)(simple)(int64_t, count) \
	_(impl)(simple)(int64_t, count, version >= EPIC_4_1) \
	_(impl)(simple)(int32_t, count, version < EPIC_4_1)

CREATE_PACKET(TS_SC_UPDATE_ITEM_COUNT, 255);

#endif // PACKETS_TS_SC_UPDATE_ITEM_COUNT_H
