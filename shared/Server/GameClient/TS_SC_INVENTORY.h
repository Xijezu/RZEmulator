#ifndef PACKETS_TS_SC_INVENTORY_H
#define PACKETS_TS_SC_INVENTORY_H

#include "Packet/PacketDeclaration.h"

#define TS_ITEM_AWAKEN_OPTION_DEF(_) \
	_(array) (uint32_t, value, 5) \
	_(array) (int32_t, data, 5)
CREATE_STRUCT(TS_ITEM_AWAKEN_OPTION);

#define TS_ITEM_BASE_INFO_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(simple) (int32_t, code) \
	_(simple) (int64_t, uid) \
	_(def)(simple)(int64_t, count) \
	_(impl)(simple)(int64_t, count, version >= EPIC_4_1_1) \
	_(impl)(simple)(int32_t, count, version < EPIC_4_1_1) \
	_(simple) (int32_t, ethereal_durability, version >= EPIC_6_3) \
	_(def)(simple) (uint32_t, endurance) \
	_(impl)(simple)(uint32_t, endurance, version >= EPIC_4_1) \
	_(impl)(simple)(uint16_t, endurance, version < EPIC_4_1) \
	_(simple) (uint8_t, enhance) \
	_(simple) (uint8_t, level) \
	_(simple) (uint16_t, unknown3, version >= EPIC_9_2) \
	_(simple) (uint32_t, flag) \
	_(array)  (int32_t, socket, 4) \
	_(array)  (int32_t, dummy_socket, 2, version < EPIC_6_1, {0}) \
	_(simple) (TS_ITEM_AWAKEN_OPTION, awaken_option, version >= EPIC_8_1) \
	_(array)  (uint32_t, random_type, 10, version >= EPIC_8_2) \
	_(array)  (int64_t, random_value_1, 10, version >= EPIC_8_2) \
	_(array)  (int64_t, random_value_2, 10, version >= EPIC_8_2) \
	_(simple) (int32_t, remain_time) \
	_(simple) (uint8_t, elemental_effect_type, version >= EPIC_6_1) \
	_(simple) (int32_t, elemental_effect_remain_time, version >= EPIC_6_1) \
	_(simple) (int32_t, elemental_effect_attack_point, version >= EPIC_6_1) \
	_(simple) (int32_t, elemental_effect_magic_point, version >= EPIC_6_1) \
	_(simple) (int32_t, appearance_code, version >= EPIC_7_4) \
	_(simple) (uint32_t, summon_code, version >= EPIC_8_2)
CREATE_STRUCT(TS_ITEM_BASE_INFO);

#define TS_ITEM_INFO_DEF(_) \
	_(simple) (TS_ITEM_BASE_INFO, base_info) \
	_(simple) (int16_t , wear_position) \
	_(simple) (uint32_t, own_summon_handle) \
	_(simple) (int32_t , index, version >= EPIC_4_1_1)
CREATE_STRUCT(TS_ITEM_INFO);

#define TS_SC_INVENTORY_DEF(_) \
	_(count) (uint16_t, items) \
	_(dynarray) (TS_ITEM_INFO, items)

CREATE_PACKET(TS_SC_INVENTORY, 207);

#endif // PACKETS_TS_SC_INVENTORY_H
