#ifndef PACKETS_TS_CS_ITEM_KEEPING_LIST_H
#define PACKETS_TS_CS_ITEM_KEEPING_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_ITEM_KEEPING_LIST_DEF(_) \
	_(simple)(int32_t, page_num)

CREATE_PACKET(TS_CS_ITEM_KEEPING_LIST, 1350);

#endif // PACKETS_TS_CS_ITEM_KEEPING_LIST_H
