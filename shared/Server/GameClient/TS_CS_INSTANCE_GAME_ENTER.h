#ifndef PACKETS_TS_CS_INSTANCE_GAME_ENTER_H
#define PACKETS_TS_CS_INSTANCE_GAME_ENTER_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_INSTANCE_GAME_ENTER_DEF(_) \
	_(simple)(int32_t, instance_game_type)

// Since EPIC_6_3
CREATE_PACKET(TS_CS_INSTANCE_GAME_ENTER, 4250);

#endif // PACKETS_TS_CS_INSTANCE_GAME_ENTER_H
