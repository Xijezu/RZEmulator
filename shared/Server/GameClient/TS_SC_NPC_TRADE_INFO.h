#ifndef PACKETS_TS_SC_NPC_TRADE_INFO_H
#define PACKETS_TS_SC_NPC_TRADE_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_NPC_TRADE_INFO_DEF(_) \
	_(simple)(bool, is_sell) \
	_(simple)(int32_t, code) \
	_(simple)(int64_t, count) \
	_(simple)(int64_t, price) \
	_(simple)(int32_t, huntaholic_point, version >= EPIC_5_2) \
	_(simple)(int32_t, arena_point, version >= EPIC_8_1) \
	_(simple)(uint32_t, target)

CREATE_PACKET(TS_SC_NPC_TRADE_INFO, 240);

#endif // PACKETS_TS_SC_NPC_TRADE_INFO_H
