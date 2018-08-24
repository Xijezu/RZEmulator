#ifndef PACKETS_TS_CS_AUCTION_CANCEL_H
#define PACKETS_TS_CS_AUCTION_CANCEL_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_AUCTION_CANCEL_DEF(_) \
	_(simple)(uint32_t, auction_uid)

CREATE_PACKET(TS_CS_AUCTION_CANCEL, 1310);

#endif // PACKETS_TS_CS_AUCTION_CANCEL_H
