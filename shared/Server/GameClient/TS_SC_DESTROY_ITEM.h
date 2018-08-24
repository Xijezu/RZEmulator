#ifndef PACKETS_TS_SC_DESTROY_ITEM_H
#define PACKETS_TS_SC_DESTROY_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_DESTROY_ITEM_DEF(_) \
	_(simple)(uint32_t, item_handle)

CREATE_PACKET(TS_SC_DESTROY_ITEM, 254);

#endif // PACKETS_TS_SC_DESTROY_ITEM_H
