#ifndef PACKETS_TS_CS_MIX_H
#define PACKETS_TS_CS_MIX_H

#include "Packet/PacketDeclaration.h"

#define TS_MIX_INFO_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(uint16_t, count)

CREATE_STRUCT(TS_MIX_INFO);

#define TS_CS_MIX_DEF(_) \
	_(simple)(TS_MIX_INFO, main_item) \
	_(count)(uint16_t, sub_items) \
	_(dynarray)(TS_MIX_INFO, sub_items)

CREATE_PACKET(TS_CS_MIX, 256);

#endif // PACKETS_TS_CS_MIX_H
