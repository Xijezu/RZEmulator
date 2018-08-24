#ifndef PACKETS_TS_TRADE_H
#define PACKETS_TS_TRADE_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_TRADE_DEF(_) \
	_(simple)(uint32_t, target_player) \
	_(simple)(int8_t, mode) \
	_(simple)(TS_ITEM_INFO, item_info)

CREATE_PACKET(TS_TRADE, 280);

#endif // PACKETS_TS_TRADE_H
