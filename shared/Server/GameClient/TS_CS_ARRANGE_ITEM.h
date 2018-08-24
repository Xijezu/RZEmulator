#ifndef PACKETS_TS_CS_ARRANGE_ITEM_H
#define PACKETS_TS_CS_ARRANGE_ITEM_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_ARRANGE_ITEM_DEF(_) \
	_(simple)(bool, bIsStorage)

CREATE_PACKET(TS_CS_ARRANGE_ITEM, 219);

#endif // PACKETS_TS_CS_ARRANGE_ITEM_H
