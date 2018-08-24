#ifndef PACKETS_TS_SC_SHOW_WINDOW_H
#define PACKETS_TS_SC_SHOW_WINDOW_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SHOW_WINDOW_DEF(_) \
	_(count)(uint16_t, window) \
	_(count)(uint16_t, argument, version >= EPIC_5_1) \
	_(count)(uint16_t, trigger) \
	_(dynstring)(window, false) \
	_(dynstring)(argument, false, version >= EPIC_5_1) \
	_(dynstring)(trigger, false)

CREATE_PACKET(TS_SC_SHOW_WINDOW, 3003);

#endif // PACKETS_TS_SC_SHOW_WINDOW_H
