#ifndef PACKETS_TS_SC_REMAIN_TITLE_TIME_H
#define PACKETS_TS_SC_REMAIN_TITLE_TIME_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_REMAIN_TITLE_TIME_DEF(_) \
	_(simple)(uint32_t, remain_title_time)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_REMAIN_TITLE_TIME, 627);

#endif // PACKETS_TS_SC_REMAIN_TITLE_TIME_H
