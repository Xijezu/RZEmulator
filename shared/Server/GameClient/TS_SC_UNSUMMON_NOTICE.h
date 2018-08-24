#ifndef PACKETS_TS_SC_UNSUMMON_NOTICE_H
#define PACKETS_TS_SC_UNSUMMON_NOTICE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_UNSUMMON_NOTICE_DEF(_) \
	_(simple)(uint32_t, summon_handle) \
	_(simple)(uint32_t, unsummon_duration)

CREATE_PACKET(TS_SC_UNSUMMON_NOTICE, 306);

#endif // PACKETS_TS_SC_UNSUMMON_NOTICE_H
