#ifndef PACKETS_TS_SC_DECOMPOSE_RESULT_H
#define PACKETS_TS_SC_DECOMPOSE_RESULT_H

#include "Packet/PacketDeclaration.h"

#define TS_DECOMPOSE_INFO_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(uint32_t, count)

CREATE_STRUCT(TS_DECOMPOSE_INFO);

#define TS_SC_DECOMPOSE_RESULT_DEF(_) \
	_(count)(uint32_t, decompositions) \
	_(dynarray)(TS_DECOMPOSE_INFO, decompositions)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_DECOMPOSE_RESULT, 266);

#endif // PACKETS_TS_SC_DECOMPOSE_RESULT_H
