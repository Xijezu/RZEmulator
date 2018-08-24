#ifndef PACKETS_TS_CS_PUTOFF_ITEM_H
#define PACKETS_TS_CS_PUTOFF_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTOFF_ITEM_DEF(_) \
	_(simple) (int8_t, position) \
	_(simple) (uint32_t, target_handle)

CREATE_PACKET(TS_CS_PUTOFF_ITEM, 201);

#endif // PACKETS_TS_CS_PUTOFF_ITEM_H
