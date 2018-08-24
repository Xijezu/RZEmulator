#ifndef PACKETS_TS_CS_AUCTION_SEARCH_H
#define PACKETS_TS_CS_AUCTION_SEARCH_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_AUCTION_SEARCH_DEF(_) \
	_(simple)(int32_t, category_id) \
	_(simple)(int32_t, sub_category_id) \
	_(string)(keyword, 31) \
	_(simple)(int32_t, page_num) \
	_(simple)(bool, is_equipable, version >= EPIC_7_2)

CREATE_PACKET(TS_CS_AUCTION_SEARCH, 1300);

#endif // PACKETS_TS_CS_AUCTION_SEARCH_H
