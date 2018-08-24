#ifndef PACKETS_TS_SC_OPEN_STORAGE_H
#define PACKETS_TS_SC_OPEN_STORAGE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_OPEN_STORAGE_DEF(_) \
	_(simple)(int32_t, maxStorageItemCount, version >= EPIC_7_4, 10000)

CREATE_PACKET(TS_SC_OPEN_STORAGE, 211);

#endif // PACKETS_TS_SC_OPEN_STORAGE_H
