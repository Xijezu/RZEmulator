#ifndef PACKETS_TS_SC_WATCH_BOOTH_H
#define PACKETS_TS_SC_WATCH_BOOTH_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_BOOTH_ITEM_INFO_DEF(_) \
	_(simple)(TS_ITEM_BASE_INFO, item) \
	_(def)(simple)(int64_t, gold) \
	_(impl)(simple)(int64_t, gold, version >= EPIC_4_1_1) \
	_(impl)(simple)(int32_t, gold, version < EPIC_4_1_1)
CREATE_STRUCT(TS_BOOTH_ITEM_INFO);

#define TS_SC_WATCH_BOOTH_DEF(_) \
	_(simple)(uint32_t, target) \
	_(simple)(uint8_t, type) \
	_(count) (uint16_t, items) \
	_(dynarray)(TS_BOOTH_ITEM_INFO, items)

CREATE_PACKET(TS_SC_WATCH_BOOTH, 703);

#endif // PACKETS_TS_SC_WATCH_BOOTH_H
