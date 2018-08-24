#ifndef PACKETS_TS_SC_ITEM_WEAR_INFO_H
#define PACKETS_TS_SC_ITEM_WEAR_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ITEM_WEAR_INFO_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(int16_t, wear_position) \
	_(simple)(uint32_t, target_handle) \
	_(simple)(int32_t, enhance) \
	_(simple)(int8_t, elemental_effect_type, version >= EPIC_6_1) \
	_(simple)(int32_t, appearance_code, version >= EPIC_7_4)

CREATE_PACKET(TS_SC_ITEM_WEAR_INFO, 287);

#endif // PACKETS_TS_SC_ITEM_WEAR_INFO_H
