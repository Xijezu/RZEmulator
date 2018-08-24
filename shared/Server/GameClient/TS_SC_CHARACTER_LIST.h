#ifndef PACKETS_TS_SC_CHARACTER_LIST_H
#define PACKETS_TS_SC_CHARACTER_LIST_H

#include "Packet/PacketDeclaration.h"
#include "PacketEnums.h"

#define LOBBY_CHARACTER_INFO_DEF(_) \
	_(simple) (uint32_t, sex) \
	_(simple) (uint32_t, race) \
	_(array)  (uint32_t, model_id, 5) \
	_(simple) (uint32_t, hair_color_index, version >= EPIC_7_1) \
	_(simple) (uint32_t, hair_color_rgb, version >= EPIC_7_1) \
	_(simple) (uint32_t, hide_equip_flag, version >= EPIC_7_1) \
	_(simple) (uint32_t, texture_id, version >= EPIC_6_3) \
	_(def)(array)(uint32_t, wear_info, 28) \
	  _(impl)(array)(uint32_t, wear_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_info, 28, version >= EPIC_9_5) \
	_(simple) (uint32_t, level) \
	_(simple) (uint32_t, job) \
	_(simple) (uint32_t, job_level) \
	_(simple) (uint32_t, exp_percentage) \
	_(simple) (uint32_t, hp) \
	_(simple) (uint32_t, mp) \
	_(simple) (uint32_t, permission) \
	_(simple) (uint8_t, is_banned) \
	_(string) (name, 19) \
	_(simple) (uint32_t, skin_color, version >= EPIC_4_1) \
	_(string) (szCreateTime, 30) \
	_(string) (szDeleteTime, 30) \
	_(def)(array)(uint32_t, wear_item_enhance_info, 28) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 28, version >= EPIC_9_5) \
	_(def)(array)(uint32_t, wear_item_level_info, 28) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 28, version >= EPIC_9_5) \
	_(def)(array)(uint8_t, wear_item_elemental_type, 28) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 24, version >= EPIC_6_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 28, version >= EPIC_9_5) \
	_(def)(array)(uint32_t, wear_appearance_code, 28) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 24, version >= EPIC_7_4 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 28, version >= EPIC_9_5) \

CREATE_STRUCT(LOBBY_CHARACTER_INFO); // struct is 304 bytes long in epic2

#define TS_SC_CHARACTER_LIST_DEF(_) \
	_(simple)   (uint32_t, current_server_time) \
	_(simple)   (uint16_t, last_character_idx, version >= EPIC_4_1, 0) \
	_(count)    (uint16_t, characters) \
	_(dynarray) (LOBBY_CHARACTER_INFO, characters)

CREATE_PACKET(TS_SC_CHARACTER_LIST, 2004);

#endif // PACKETS_TS_SC_CHARACTER_LIST_H
