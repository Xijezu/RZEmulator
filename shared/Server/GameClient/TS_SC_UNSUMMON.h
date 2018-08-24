#ifndef PACKETS_TS_SC_UNSUMMON_H
#define PACKETS_TS_SC_UNSUMMON_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_UNSUMMON_DEF(_) \
	_(simple)(uint32_t, summon_handle)

CREATE_PACKET(TS_SC_UNSUMMON, 305);

#endif // PACKETS_TS_SC_UNSUMMON_H
