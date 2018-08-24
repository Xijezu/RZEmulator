#ifndef PACKETS_TS_SC_QUEST_STATUS_H
#define PACKETS_TS_SC_QUEST_STATUS_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_QUEST_STATUS_DEF(_) \
	_(simple)(int32_t, code) \
	_(def)(array) (uint32_t, status, 6) \
	_(impl)(array)(uint32_t, status, 6, version >= EPIC_6_1) \
	_(impl)(array)(uint32_t, status, 3, version < EPIC_6_1) \
	_(simple)(int8_t, nProgress, version >= EPIC_6_3) \
	_(simple)(uint32_t, nTimeLimit, version >= EPIC_6_3)

CREATE_PACKET(TS_SC_QUEST_STATUS, 601);

#endif // PACKETS_TS_SC_QUEST_STATUS_H
