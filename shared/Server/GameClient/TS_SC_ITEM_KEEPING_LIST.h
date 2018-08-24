#ifndef PACKETS_TS_SC_ITEM_KEEPING_LIST_H
#define PACKETS_TS_SC_ITEM_KEEPING_LIST_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_ITEM_KEEPING_INFO_DEF(_) \
	_(simple) (int32_t, keeping_uid) \
	_(simple) (TS_ITEM_BASE_INFO, item_info) \
	_(simple) (int32_t, duration) \
	_(simple) (uint8_t, keeping_type) \
	_(simple) (int32_t, related_item_code) \
	_(simple) (int32_t, related_item_enhance) \
	_(simple) (int32_t, related_item_level)
CREATE_STRUCT(TS_ITEM_KEEPING_INFO);

#define TS_SC_ITEM_KEEPING_LIST_DEF(_) \
	_(simple)(int32_t, page_num) \
	_(simple)(int32_t, total_page_count) \
	_(simple)(int32_t, keeping_info_count) \
	_(array) (TS_ITEM_KEEPING_INFO, keeping_info, 40)

CREATE_PACKET(TS_SC_ITEM_KEEPING_LIST, 1351);

#endif // PACKETS_TS_SC_ITEM_KEEPING_LIST_H
