#ifndef PACKETS_TS_SC_GENERAL_MESSAGE_BOX_H
#define PACKETS_TS_SC_GENERAL_MESSAGE_BOX_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_GENERAL_MESSAGE_BOX_DEF(_) \
	_(count)(uint16_t, text) \
	_(dynstring)(text, false)

// Since EPIC_7_4
CREATE_PACKET(TS_SC_GENERAL_MESSAGE_BOX, 3004);

#endif // PACKETS_TS_SC_GENERAL_MESSAGE_BOX_H
