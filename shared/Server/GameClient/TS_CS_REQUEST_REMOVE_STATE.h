#ifndef PACKETS_TS_CS_REQUEST_REMOVE_STATE_H
#define PACKETS_TS_CS_REQUEST_REMOVE_STATE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_REQUEST_REMOVE_STATE_DEF(_) \
	_(simple)(uint32_t, target) \
	_(simple)(int32_t, state_code)

CREATE_PACKET(TS_CS_REQUEST_REMOVE_STATE, 408);

#endif // PACKETS_TS_CS_REQUEST_REMOVE_STATE_H
