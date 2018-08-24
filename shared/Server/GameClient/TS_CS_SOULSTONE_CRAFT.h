#ifndef PACKETS_TS_CS_SOULSTONE_CRAFT_H
#define PACKETS_TS_CS_SOULSTONE_CRAFT_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SOULSTONE_CRAFT_DEF(_) \
	_(simple)(uint32_t, craft_item_handle) \
	_(array)(uint32_t, soulstone_handle, 4)

CREATE_PACKET(TS_CS_SOULSTONE_CRAFT, 260);

#endif // PACKETS_TS_CS_SOULSTONE_CRAFT_H
