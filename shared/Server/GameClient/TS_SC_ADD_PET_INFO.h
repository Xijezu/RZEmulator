#ifndef PACKETS_TS_SC_ADD_PET_INFO_H
#define PACKETS_TS_SC_ADD_PET_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ADD_PET_INFO_DEF(_) \
	_(simple)(uint32_t, cage_handle) \
	_(simple)(uint32_t, pet_handle) \
	_(string)(name, 19) \
	_(simple)(int32_t, code)

CREATE_PACKET(TS_SC_ADD_PET_INFO, 351);

#endif // PACKETS_TS_SC_ADD_PET_INFO_H
