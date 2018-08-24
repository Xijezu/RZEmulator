#ifndef PACKETS_TS_SC_REMOVE_SUMMON_INFO_H
#define PACKETS_TS_SC_REMOVE_SUMMON_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_REMOVE_SUMMON_INFO_DEF(_) \
	_(simple)(uint32_t, card_handle)

CREATE_PACKET(TS_SC_REMOVE_SUMMON_INFO, 302);

#endif // PACKETS_TS_SC_REMOVE_SUMMON_INFO_H
