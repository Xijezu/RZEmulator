#ifndef PACKETS_TS_CS_BUY_FROM_BOOTH_H
#define PACKETS_TS_CS_BUY_FROM_BOOTH_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_CS_BUY_FROM_BOOTH_DEF(_) \
	_(simple)(uint32_t, target) \
	_(count)(int16_t, items) \
	_(dynarray)(TS_ITEM_BASE_INFO, items)

CREATE_PACKET(TS_CS_BUY_FROM_BOOTH, 705);

#endif // PACKETS_TS_CS_BUY_FROM_BOOTH_H
