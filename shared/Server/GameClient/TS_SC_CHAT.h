#ifndef PACKETS_TS_SC_CHAT_H
#define PACKETS_TS_SC_CHAT_H

#include "Packet/PacketDeclaration.h"
#include "TS_CS_CHAT_REQUEST.h"

// Message len include null terminator
// Message must have null terminator
#define TS_SC_CHAT_DEF(_) \
	_(string)(szSender, 21) \
	_(count)(uint16_t, message) \
	_(simple)(TS_CHAT_TYPE, type) \
	_(dynstring)(message, true)

CREATE_PACKET(TS_SC_CHAT, 22);

#endif // PACKETS_TS_SC_CHAT_H
