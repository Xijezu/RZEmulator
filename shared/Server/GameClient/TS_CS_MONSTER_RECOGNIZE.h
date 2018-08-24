#ifndef PACKETS_TS_CS_MONSTER_RECOGNIZE_H
#define PACKETS_TS_CS_MONSTER_RECOGNIZE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_MONSTER_RECOGNIZE_DEF(_) \
	_(simple)(uint32_t, recognizer_handle) \
	_(simple)(uint32_t, monster_handle)

// Since EPIC_6_3
CREATE_PACKET(TS_CS_MONSTER_RECOGNIZE, 517);

#endif // PACKETS_TS_CS_MONSTER_RECOGNIZE_H
