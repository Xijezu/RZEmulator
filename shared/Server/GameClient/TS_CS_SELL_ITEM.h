#ifndef PACKETS_TS_CS_SELL_ITEM_H
#define PACKETS_TS_CS_SELL_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SELL_ITEM_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(def)(simple) (int64_t, sell_count) \
	_(impl)(simple)(int64_t, sell_count, version >= EPIC_9_4) \
	_(impl)(simple)(uint16_t, sell_count, version >= EPIC_4_1 && version < EPIC_9_4) \
	_(impl)(simple)(uint8_t, sell_count, version < EPIC_4_1)

CREATE_PACKET(TS_CS_SELL_ITEM, 252);

#endif // PACKETS_TS_CS_SELL_ITEM_H
