#ifndef PACKETS_TS_SC_QUEST_INFOMATION_H
#define PACKETS_TS_SC_QUEST_INFOMATION_H

#include "Packet/PacketDeclaration.h"

enum TS_QUEST_PROGRESS : int32_t
{
	IS_STARTABLE = 0,
	IS_IN_PROGRESS = 1,
	IS_FINISHABLE = 2,
};

// Seems unused
#define TS_SC_QUEST_INFOMATION_DEF(_) \
	_(simple)(int32_t, code) \
	_(simple)(TS_QUEST_PROGRESS, nProgress) \
	_(simple)(uint16_t, trigger_length)

CREATE_PACKET(TS_SC_QUEST_INFOMATION, 602);

#endif // PACKETS_TS_SC_QUEST_INFOMATION_H
