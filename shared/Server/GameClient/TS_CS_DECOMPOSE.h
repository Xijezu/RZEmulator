#ifndef PACKETS_TS_CS_DECOMPOSE_H
#define PACKETS_TS_CS_DECOMPOSE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_DECOMPOSE_DEF(_) \
	_(count)(uint32_t, items_handle) \
	_(dynarray)(uint32_t, items_handle)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_DECOMPOSE, 265);

#endif // PACKETS_TS_CS_DECOMPOSE_H
