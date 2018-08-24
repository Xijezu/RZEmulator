#ifndef PACKETS_TS_SC_HAIR_INFO_H
#define PACKETS_TS_SC_HAIR_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_HAIR_INFO_DEF(_) \
	_(simple)(uint32_t, hPlayer) \
	_(simple)(int32_t, nHairID) \
	_(simple)(int32_t, nHairColorIndex) \
	_(simple)(uint32_t, nHairColorRGB)

CREATE_PACKET(TS_SC_HAIR_INFO, 220);

#endif // PACKETS_TS_SC_HAIR_INFO_H
