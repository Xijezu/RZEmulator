#ifndef PACKETS_TS_CS_NURSE_CREATURE_H
#define PACKETS_TS_CS_NURSE_CREATURE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_NURSE_CREATURE_DEF(_) \
	_(simple)(uint32_t, creature_card_handle)

// Since EPIC_7_3
CREATE_PACKET(TS_CS_NURSE_CREATURE, 6006);

#endif // PACKETS_TS_CS_NURSE_CREATURE_H
