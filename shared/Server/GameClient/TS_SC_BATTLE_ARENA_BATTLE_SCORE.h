#ifndef PACKETS_TS_SC_BATTLE_ARENA_BATTLE_SCORE_H
#define PACKETS_TS_SC_BATTLE_ARENA_BATTLE_SCORE_H

#include "Packet/PacketDeclaration.h"

#define TS_BATTLE_ARENA_TEAM_SCORE_DEF(_) \
	_(simple)(int16_t, nTotalKillCount) \
	_(simple)(int16_t, nTotalDeathCount) \
	_(simple)(int16_t, nScore)

CREATE_STRUCT(TS_BATTLE_ARENA_TEAM_SCORE);

#define TS_BATTLE_ARENA_PLAYER_SCORE_DEF(_) \
	_(string)(szName, 19) \
	_(simple)(int16_t, nKillCount) \
	_(simple)(int16_t, nDeathCount) \
	_(simple)(int16_t, nPropActivateCount) \
	_(simple)(int32_t, nTotalGainAP)

CREATE_STRUCT(TS_BATTLE_ARENA_PLAYER_SCORE);

#define TS_SC_BATTLE_ARENA_BATTLE_SCORE_DEF(_) \
	_(array)(TS_BATTLE_ARENA_TEAM_SCORE, aTeamScore, 2) \
	_(count)(int8_t, nPlayerScore) \
	_(dynarray)(TS_BATTLE_ARENA_PLAYER_SCORE, nPlayerScore)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_BATTLE_SCORE, 4712);

#endif // PACKETS_TS_SC_BATTLE_ARENA_BATTLE_SCORE_H
