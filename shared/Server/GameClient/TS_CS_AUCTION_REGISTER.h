#ifndef PACKETS_TS_CS_AUCTION_REGISTER_H
#define PACKETS_TS_CS_AUCTION_REGISTER_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_AUCTION_REGISTER_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(int32_t, item_count) \
	_(simple)(int64_t, start_price) \
	_(simple)(int64_t, instant_purchase_price) \
	_(simple)(int8_t, duration_type)

CREATE_PACKET(TS_CS_AUCTION_REGISTER, 1309);

#endif // PACKETS_TS_CS_AUCTION_REGISTER_H
