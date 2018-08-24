#ifndef PACKETS_TS_CS_RESURRECTION_H
#define PACKETS_TS_CS_RESURRECTION_H

#include "Packet/PacketDeclaration.h"

enum TS_RESURRECTION_TYPE : int8_t
{
	RT_UseNone = 0,
	RT_UseState = 1,
	RT_UsePotion = 2,
	RT_Compete = 3,
	RT_Deathmatch = 4
};

#define TS_CS_RESURRECTION_DEF(_) \
	_(simple)(uint32_t, handle, version >= EPIC_4_1) \
	_(simple)(TS_RESURRECTION_TYPE, type, version >= EPIC_6_1) \
	_(simple)(bool, use_state, version < EPIC_6_1) \
	_(simple)(bool, use_potion, version >= EPIC_4_1 && version < EPIC_6_1)

CREATE_PACKET(TS_CS_RESURRECTION, 513);

#endif // PACKETS_TS_CS_RESURRECTION_H
