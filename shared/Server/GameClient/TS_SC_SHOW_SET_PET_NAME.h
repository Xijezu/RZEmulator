#ifndef PACKETS_TS_SC_SHOW_SET_PET_NAME_H
#define PACKETS_TS_SC_SHOW_SET_PET_NAME_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SHOW_SET_PET_NAME_DEF(_) \
	_(simple)(uint32_t, handle)

CREATE_PACKET(TS_SC_SHOW_SET_PET_NAME, 353);

#endif // PACKETS_TS_SC_SHOW_SET_PET_NAME_H
