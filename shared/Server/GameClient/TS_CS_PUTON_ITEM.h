#ifndef PACKETS_TS_CS_PUTON_ITEM_H
#define PACKETS_TS_CS_PUTON_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_ITEM_DEF(_) \
	_(simple) (int8_t, position) \
	_(simple) (uint32_t, item_handle) \
	_(simple) (uint32_t, target_handle)

CREATE_PACKET(TS_CS_PUTON_ITEM, 200);

#endif // PACKETS_TS_CS_PUTON_ITEM_H
