#ifndef PACKETS_TS_SC_CHAT_RESULT_H
#define PACKETS_TS_SC_CHAT_RESULT_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_CHAT_RESULT_DEF(_) \
	_(simple)(int8_t, type) \
	_(simple)(int8_t, percentage) \
	_(simple)(int32_t, result) \
	_(simple)(int32_t, reserved)

CREATE_PACKET(TS_SC_CHAT_RESULT, 24);

#endif // PACKETS_TS_SC_CHAT_RESULT_H
