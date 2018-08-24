#ifndef PACKETS_TS_CS_SET_PET_NAME_H
#define PACKETS_TS_CS_SET_PET_NAME_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SET_PET_NAME_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(string)(name, 19)

CREATE_PACKET(TS_CS_SET_PET_NAME, 354);

#endif // PACKETS_TS_CS_SET_PET_NAME_H
