#ifndef PACKETS_TS_SC_UNSUMMON_PET_H
#define PACKETS_TS_SC_UNSUMMON_PET_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_UNSUMMON_PET_DEF(_) \
	_(simple)(uint32_t, handle)

CREATE_PACKET(TS_SC_UNSUMMON_PET, 350);

#endif // PACKETS_TS_SC_UNSUMMON_PET_H
