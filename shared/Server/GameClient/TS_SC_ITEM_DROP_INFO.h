#ifndef PACKETS_TS_SC_ITEM_DROP_INFO_H
#define PACKETS_TS_SC_ITEM_DROP_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ITEM_DROP_INFO_DEF(_) \
	_(simple)(uint32_t, monster_handle) \
	_(simple)(uint32_t, item_handle)

CREATE_PACKET(TS_SC_ITEM_DROP_INFO, 282);

#endif // PACKETS_TS_SC_ITEM_DROP_INFO_H
