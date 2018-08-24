#ifndef PACKETS_TS_CS_QUEST_INFO_H
#define PACKETS_TS_CS_QUEST_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_QUEST_INFO_DEF(_) \
	_(simple)(int32_t, code)

// Since EPIC_6_3
CREATE_PACKET(TS_CS_QUEST_INFO, 604);

#endif // PACKETS_TS_CS_QUEST_INFO_H
