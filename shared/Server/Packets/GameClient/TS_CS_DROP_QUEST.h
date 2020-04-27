#ifndef PACKETS_TS_CS_DROP_QUEST_H
#define PACKETS_TS_CS_DROP_QUEST_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_DROP_QUEST_DEF(_) \
	_(simple)(int32_t, code)

CREATE_PACKET(TS_CS_DROP_QUEST, 603);

#endif // PACKETS_TS_CS_DROP_QUEST_H
