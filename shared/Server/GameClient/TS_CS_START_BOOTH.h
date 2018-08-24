#ifndef PACKETS_TS_CS_START_BOOTH_H
#define PACKETS_TS_CS_START_BOOTH_H

#include "Packet/PacketDeclaration.h"

#define TS_BOOTH_OPEN_ITEM_INFO_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(int32_t, cnt) \
	_(def)(simple) (int64_t, gold) \
	_(impl)(simple)(int64_t, gold, version >= EPIC_4_1_1) \
	_(impl)(simple)(int32_t, gold, version < EPIC_4_1_1)
CREATE_STRUCT(TS_BOOTH_OPEN_ITEM_INFO);

#define TS_CS_START_BOOTH_DEF(_) \
	_(string)(name, 49) \
	_(simple)(uint8_t, type) \
	_(count) (uint16_t, items) \
	_(dynarray)(TS_BOOTH_OPEN_ITEM_INFO, items)

CREATE_PACKET(TS_CS_START_BOOTH, 700);

#endif // PACKETS_TS_CS_START_BOOTH_H
