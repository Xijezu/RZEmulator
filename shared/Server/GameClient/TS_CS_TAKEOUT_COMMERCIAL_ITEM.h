#ifndef PACKETS_TS_CS_TAKEOUT_COMMERCIAL_ITEM_H
#define PACKETS_TS_CS_TAKEOUT_COMMERCIAL_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_TAKEOUT_COMMERCIAL_ITEM_DEF(_) \
	_(simple)(uint32_t, commercial_item_uid) \
	_(simple)(uint16_t, count)

CREATE_PACKET(TS_CS_TAKEOUT_COMMERCIAL_ITEM, 10005);

#endif // PACKETS_TS_CS_TAKEOUT_COMMERCIAL_ITEM_H
