#ifndef PACKETS_TS_CS_DROP_ITEM_H
#define PACKETS_TS_CS_DROP_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_DROP_ITEM_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(def)(simple) (int32_t, count) \
	_(impl)(simple)(int32_t, count, version >= EPIC_4_1) \
	_(impl)(simple)(uint16_t, count, version < EPIC_4_1)

CREATE_PACKET(TS_CS_DROP_ITEM, 203);

#endif // PACKETS_TS_CS_DROP_ITEM_H
