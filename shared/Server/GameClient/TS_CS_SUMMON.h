#ifndef PACKETS_TS_CS_SUMMON_H
#define PACKETS_TS_CS_SUMMON_H

#include "Packet/PacketDeclaration.h"

// Seems unused
#define TS_CS_SUMMON_DEF(_) \
	_(simple)(int8_t, is_summon) \
	_(simple)(uint32_t, card_handle)

CREATE_PACKET(TS_CS_SUMMON, 304);

#endif // PACKETS_TS_CS_SUMMON_H
