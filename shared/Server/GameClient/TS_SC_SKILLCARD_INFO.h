#ifndef PACKETS_TS_SC_SKILLCARD_INFO_H
#define PACKETS_TS_SC_SKILLCARD_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SKILLCARD_INFO_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(uint32_t, target_handle)

CREATE_PACKET(TS_SC_SKILLCARD_INFO, 286);

#endif // PACKETS_TS_SC_SKILLCARD_INFO_H
