#ifndef PACKETS_TS_CS_TAKE_ITEM_H
#define PACKETS_TS_CS_TAKE_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_TAKE_ITEM_DEF(_) \
	_(simple)(uint32_t, taker_handle, version >= EPIC_5_2) \
	_(simple)(uint32_t, item_handle)

CREATE_PACKET(TS_CS_TAKE_ITEM, 204);

#endif // PACKETS_TS_CS_TAKE_ITEM_H
