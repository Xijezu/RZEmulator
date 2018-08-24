#ifndef PACKETS_TS_SC_BOOTH_TRADE_INFO_H
#define PACKETS_TS_SC_BOOTH_TRADE_INFO_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_BOOTH_TRADE_ITEM_INFO_DEF(_) \
	_(simple)(TS_ITEM_BASE_INFO, item) \
	_(simple)(int64_t, gold)
CREATE_STRUCT(TS_BOOTH_TRADE_ITEM_INFO);

#define TS_SC_BOOTH_TRADE_INFO_DEF(_) \
	_(simple)(uint32_t, target) \
	_(simple)(bool, is_sell) \
	_(count)(uint16_t, trades) \
	_(dynarray)(TS_BOOTH_TRADE_ITEM_INFO, trades)

CREATE_PACKET(TS_SC_BOOTH_TRADE_INFO, 710);

#endif // PACKETS_TS_SC_BOOTH_TRADE_INFO_H
