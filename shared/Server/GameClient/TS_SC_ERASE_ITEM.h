#ifndef PACKETS_TS_SC_ERASE_ITEM_H
#define PACKETS_TS_SC_ERASE_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_ERASE_ITEM_INFO_RESULT_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(int64_t, count)

CREATE_STRUCT(TS_ERASE_ITEM_INFO_RESULT);

#define TS_SC_ERASE_ITEM_DEF(_) \
	_(count)(int8_t, erased_items) \
	_(dynarray)(TS_ERASE_ITEM_INFO_RESULT, erased_items)

// Since EPIC_7_2, was TS_SC_RESULT before
CREATE_PACKET(TS_SC_ERASE_ITEM, 209);

#endif // PACKETS_TS_SC_ERASE_ITEM_H
