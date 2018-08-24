#ifndef PACKETS_TS_SC_BATTLE_ARENA_BATTLE_INFO_H
#define PACKETS_TS_SC_BATTLE_ARENA_BATTLE_INFO_H

#include "Packet/PacketDeclaration.h"

enum TS_BATTLE_GRADE : int32_t
{
  BG_INVALID = -1,
  BG_ROOKIE = 0,
  BG_GROW = 1,
  BG_MAJOR = 2,
  MAX_BATTLE_GRADE = 3,
};

#define TS_BATTLE_ARENA_PLAYER_INFO_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int32_t, nJobID) \
	_(string)(szName, 19)

CREATE_STRUCT(TS_BATTLE_ARENA_PLAYER_INFO);

#define TS_SC_BATTLE_ARENA_BATTLE_INFO_DEF(_) \
	_(simple)(int32_t, nArenaID) \
	_(simple)(uint32_t, nForceEnterTime) \
	_(simple)(uint32_t, nStartTime) \
	_(simple)(TS_BATTLE_GRADE, eGrade) \
	_(simple)(uint32_t, nEndTime) \
	_(count)(int8_t, team_1) \
	_(count)(int8_t, team_2) \
	_(dynarray)(TS_BATTLE_ARENA_PLAYER_INFO, team_1) \
	_(dynarray)(TS_BATTLE_ARENA_PLAYER_INFO, team_2)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_BATTLE_INFO, 4706);

#endif // PACKETS_TS_SC_BATTLE_ARENA_BATTLE_INFO_H
