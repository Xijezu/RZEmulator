#ifndef PACKETS_TS_SC_GET_BOOTHS_NAME_H
#define PACKETS_TS_SC_GET_BOOTHS_NAME_H

#include "Packet/PacketDeclaration.h"

#define TS_BOOTH_NAME_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(string)(name, 49)
CREATE_STRUCT(TS_BOOTH_NAME);

#define TS_SC_GET_BOOTHS_NAME_DEF(_) \
	_(count)(int32_t, booths) \
	_(dynarray)(TS_BOOTH_NAME, booths)

CREATE_PACKET(TS_SC_GET_BOOTHS_NAME, 708);

#endif // PACKETS_TS_SC_GET_BOOTHS_NAME_H
