#ifndef PACKETS_TS_SC_MOUNT_SUMMON_H
#define PACKETS_TS_SC_MOUNT_SUMMON_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_MOUNT_SUMMON_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(uint32_t, summon_handle) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(bool, success)

CREATE_PACKET(TS_SC_MOUNT_SUMMON, 320);

#endif // PACKETS_TS_SC_MOUNT_SUMMON_H
