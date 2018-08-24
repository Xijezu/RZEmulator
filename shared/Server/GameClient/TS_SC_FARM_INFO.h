#ifndef PACKETS_TS_SC_FARM_INFO_H
#define PACKETS_TS_SC_FARM_INFO_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

#define TS_FARM_SUMMON_INFO_DEF(_) \
	_(simple)(int32_t, index) \
	_(simple)(int64_t, exp) \
	_(string)(name, 19) \
	_(simple)(int32_t, duration) \
	_(simple)(int32_t, elasped_time) \
	_(simple)(int32_t, refresh_time) \
	_(simple)(int8_t, using_cash) \
	_(simple)(int8_t, using_cracker) \
	_(simple)(TS_ITEM_BASE_INFO, card_info)

CREATE_STRUCT(TS_FARM_SUMMON_INFO);

#define TS_SC_FARM_INFO_DEF(_) \
	_(count)(int8_t, summons) \
	_(dynarray)(TS_FARM_SUMMON_INFO, summons)

// Since EPIC_7_3
CREATE_PACKET(TS_SC_FARM_INFO, 6001);

#endif // PACKETS_TS_SC_FARM_INFO_H
