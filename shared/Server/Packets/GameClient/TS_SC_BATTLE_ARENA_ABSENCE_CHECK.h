#ifndef PACKETS_TS_SC_BATTLE_ARENA_ABSENCE_CHECK_H
#define PACKETS_TS_SC_BATTLE_ARENA_ABSENCE_CHECK_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_SC_BATTLE_ARENA_ABSENCE_CHECK_DEF(_) _(simple)(uint32_t, nLimitTime)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_ABSENCE_CHECK, 4718);

#endif // PACKETS_TS_SC_BATTLE_ARENA_ABSENCE_CHECK_H
