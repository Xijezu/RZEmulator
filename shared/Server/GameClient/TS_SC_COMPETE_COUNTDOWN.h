#ifndef PACKETS_TS_SC_COMPETE_COUNTDOWN_H
#define PACKETS_TS_SC_COMPETE_COUNTDOWN_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_COMPETE_COUNTDOWN_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(string)(competitor, 31) \
	_(simple)(uint32_t, handle_competitor)

CREATE_PACKET(TS_SC_COMPETE_COUNTDOWN, 4504);

#endif // PACKETS_TS_SC_COMPETE_COUNTDOWN_H
