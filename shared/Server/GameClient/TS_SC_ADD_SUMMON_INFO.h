#ifndef PACKETS_TS_SC_ADD_SUMMON_INFO_H
#define PACKETS_TS_SC_ADD_SUMMON_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ADD_SUMMON_INFO_DEF(_) \
	_(simple)(uint32_t, card_handle) \
	_(simple)(uint32_t, summon_handle) \
	_(string)(name, 19) \
	_(simple)(int32_t, code) \
	_(simple)(int32_t, level) \
	_(simple)(int32_t, sp)

CREATE_PACKET(TS_SC_ADD_SUMMON_INFO, 301);

#endif // PACKETS_TS_SC_ADD_SUMMON_INFO_H
