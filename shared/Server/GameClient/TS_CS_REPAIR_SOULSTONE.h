#ifndef PACKETS_TS_CS_REPAIR_SOULSTONE_H
#define PACKETS_TS_CS_REPAIR_SOULSTONE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_REPAIR_SOULSTONE_DEF(_) \
	_(array)(uint32_t, item_handle, 6)

CREATE_PACKET(TS_CS_REPAIR_SOULSTONE, 262);

#endif // PACKETS_TS_CS_REPAIR_SOULSTONE_H
