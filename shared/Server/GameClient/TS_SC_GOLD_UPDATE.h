#ifndef PACKETS_TS_SC_GOLD_UPDATE_H
#define PACKETS_TS_SC_GOLD_UPDATE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_GOLD_UPDATE_DEF(_) \
	_(def)(simple) (uint64_t, gold) \
	_(impl)(simple)(uint64_t, gold, version >= EPIC_4_1_1) \
	_(impl)(simple)(uint32_t, gold, version < EPIC_4_1_1) \
	_(simple) (uint32_t, chaos, version > EPIC_4_1_1)

CREATE_PACKET(TS_SC_GOLD_UPDATE, 1001);

#endif // PACKETS_TS_SC_GOLD_UPDATE_H
