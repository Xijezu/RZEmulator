#ifndef PACKETS_TS_SC_BATTLE_ARENA_JOIN_BATTLE_H
#define PACKETS_TS_SC_BATTLE_ARENA_JOIN_BATTLE_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_BATTLE_ARENA_JOIN_BATTLE_DEF(_) \
	_(simple)(int32_t, nTeamNo) \
	_(simple)(uint32_t, handle) \
	_(simple)(int32_t, nJobID) \
	_(string)(szName, 19)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_JOIN_BATTLE, 4713);

#endif // PACKETS_TS_SC_BATTLE_ARENA_JOIN_BATTLE_H
