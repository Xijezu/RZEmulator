#ifndef PACKETS_TS_SC_CHANGE_TITLE_CONDITION_H
#define PACKETS_TS_SC_CHANGE_TITLE_CONDITION_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_CHANGE_TITLE_CONDITION_DEF(_) \
	_(simple)(int32_t, condition_id) \
	_(simple)(int64_t, count)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_CHANGE_TITLE_CONDITION, 636);

#endif // PACKETS_TS_SC_CHANGE_TITLE_CONDITION_H
