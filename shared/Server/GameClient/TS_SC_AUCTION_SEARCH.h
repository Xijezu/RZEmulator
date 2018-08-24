#ifndef PACKETS_TS_SC_AUCTION_SEARCH_H
#define PACKETS_TS_SC_AUCTION_SEARCH_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_AUCTION_INFO_DEF(_) \
	_(simple) (int32_t , auction_uid) \
	_(simple) (TS_ITEM_BASE_INFO, item_info) \
	_(simple) (uint8_t, duration_type) \
	_(simple) (uint64_t, bidded_price) \
	_(simple) (uint64_t, instant_purchase_price)
CREATE_STRUCT(TS_AUCTION_INFO);

#define TS_SEARCHED_AUCTION_INFO_DEF(_) \
	_(simple) (TS_AUCTION_INFO, auction_details) \
	_(string) (seller_name, 31) \
	_(simple) (uint8_t, flag)
CREATE_STRUCT(TS_SEARCHED_AUCTION_INFO);

#define TS_SC_AUCTION_SEARCH_DEF(_) \
	_(simple)(int32_t, page_num) \
	_(simple)(int32_t, total_page_count) \
	_(simple)(int32_t, auction_info_count) \
	_(array) (TS_SEARCHED_AUCTION_INFO, auction_info, 40)

CREATE_PACKET(TS_SC_AUCTION_SEARCH, 1301);

#endif // PACKETS_TS_SC_AUCTION_SEARCH_H
