#ifndef PACKETS_TS_SC_SET_MAIN_TITLE_H
#define PACKETS_TS_SC_SET_MAIN_TITLE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SET_MAIN_TITLE_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int32_t, code)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_SET_MAIN_TITLE, 629);

#endif // PACKETS_TS_SC_SET_MAIN_TITLE_H
