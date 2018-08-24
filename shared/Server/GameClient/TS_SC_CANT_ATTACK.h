#ifndef PACKETS_TS_SC_CANT_ATTACK_H
#define PACKETS_TS_SC_CANT_ATTACK_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_CANT_ATTACK_DEF(_) \
	_(simple)(uint32_t, attacker_handle) \
	_(simple)(uint32_t, target_handle) \
	_(simple)(int32_t, reason)

CREATE_PACKET(TS_SC_CANT_ATTACK, 102);

#endif // PACKETS_TS_SC_CANT_ATTACK_H
