#ifndef PACKETS_TS_CS_DIALOG_H
#define PACKETS_TS_CS_DIALOG_H

#include "Packet/PacketDeclaration.h"

// Server check size - 9 == trigger_length
// No null terminator allowed
#define TS_CS_DIALOG_DEF(_) \
	_(count)(uint16_t, trigger) \
	_(dynstring)(trigger, false)

CREATE_PACKET(TS_CS_DIALOG, 3001);

#endif // PACKETS_TS_CS_DIALOG_H
