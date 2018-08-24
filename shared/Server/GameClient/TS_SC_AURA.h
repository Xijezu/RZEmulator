#ifndef PACKETS_TS_SC_AURA_H
#define PACKETS_TS_SC_AURA_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_AURA_DEF(_) \
	_(simple)(uint32_t, caster) \
	_(simple)(uint16_t, skill_id) \
	_(simple)(bool, status)

CREATE_PACKET(TS_SC_AURA, 407);

#endif // PACKETS_TS_SC_AURA_H
