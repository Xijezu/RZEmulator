#ifndef PACKETS_TS_SC_CHAT_LOCAL_H
#define PACKETS_TS_SC_CHAT_LOCAL_H

#include "Packet/PacketDeclaration.h"
#include "TS_CS_CHAT_REQUEST.h"

// Message len include null terminator
// Message must have null terminator
#define TS_SC_CHAT_LOCAL_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(count)(uint8_t, message) \
	_(simple)(TS_CHAT_TYPE, type) \
	_(dynstring)(message, true)

CREATE_PACKET(TS_SC_CHAT_LOCAL, 21);

#endif // PACKETS_TS_SC_CHAT_LOCAL_H
