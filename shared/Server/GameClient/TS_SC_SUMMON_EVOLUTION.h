#ifndef PACKETS_TS_SC_SUMMON_EVOLUTION_H
#define PACKETS_TS_SC_SUMMON_EVOLUTION_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_SUMMON_EVOLUTION_DEF(_) \
	_(simple)(uint32_t, card_handle) \
	_(simple)(uint32_t, summon_handle) \
	_(string)(name, 19) \
	_(simple)(int32_t, code)

CREATE_PACKET(TS_SC_SUMMON_EVOLUTION, 307);

#endif // PACKETS_TS_SC_SUMMON_EVOLUTION_H
