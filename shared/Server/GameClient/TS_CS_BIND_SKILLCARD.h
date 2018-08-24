#ifndef PACKETS_TS_CS_BIND_SKILLCARD_H
#define PACKETS_TS_CS_BIND_SKILLCARD_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_BIND_SKILLCARD_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(uint32_t, target_handle)

CREATE_PACKET(TS_CS_BIND_SKILLCARD, 284);

#endif // PACKETS_TS_CS_BIND_SKILLCARD_H
