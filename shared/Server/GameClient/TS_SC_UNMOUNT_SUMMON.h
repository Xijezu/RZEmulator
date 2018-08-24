#ifndef PACKETS_TS_SC_UNMOUNT_SUMMON_H
#define PACKETS_TS_SC_UNMOUNT_SUMMON_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_UNMOUNT_SUMMON_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(uint32_t, summon_handle) \
	_(simple)(int8_t, flag)

CREATE_PACKET(TS_SC_UNMOUNT_SUMMON, 321);

#endif // PACKETS_TS_SC_UNMOUNT_SUMMON_H
