#ifndef PACKETS_TS_CS_PUTON_ITEM_SET_H
#define PACKETS_TS_CS_PUTON_ITEM_SET_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_ITEM_SET_DEF(_) \
	_(def)(array) (uint32_t, handle, 24) \
	_(impl)(array)(uint32_t, handle, 24, version >= EPIC_4_1) \
	_(impl)(array)(uint32_t, handle, 14, version < EPIC_4_1)

CREATE_PACKET(TS_CS_PUTON_ITEM_SET, 281);

#endif // PACKETS_TS_CS_PUTON_ITEM_SET_H
