#ifndef PACKETS_TS_CS_AUCTION_BID_H
#define PACKETS_TS_CS_AUCTION_BID_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_AUCTION_BID_DEF(_) \
	_(simple)(int32_t, auction_uid) \
	_(simple)(int64_t, price)

CREATE_PACKET(TS_CS_AUCTION_BID, 1306);

#endif // PACKETS_TS_CS_AUCTION_BID_H
