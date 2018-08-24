#ifndef PACKETS_TS_SC_COMMERCIAL_STORAGE_LIST_H
#define PACKETS_TS_SC_COMMERCIAL_STORAGE_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_COMMERCIAL_ITEM_INFO_DEF(_) \
	_(simple)(uint32_t, commercial_item_uid) \
	_(simple)(int32_t, code) \
	_(simple)(uint16_t, count)

CREATE_STRUCT(TS_COMMERCIAL_ITEM_INFO);

#define TS_SC_COMMERCIAL_STORAGE_LIST_DEF(_) \
	_(count)(uint16_t, items) \
	_(dynarray)(TS_COMMERCIAL_ITEM_INFO, items)

CREATE_PACKET(TS_SC_COMMERCIAL_STORAGE_LIST, 10004);

#endif // PACKETS_TS_SC_COMMERCIAL_STORAGE_LIST_H
