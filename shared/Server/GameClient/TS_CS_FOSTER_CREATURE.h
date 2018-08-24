#ifndef PACKETS_TS_CS_FOSTER_CREATURE_H
#define PACKETS_TS_CS_FOSTER_CREATURE_H

#include "Packet/PacketDeclaration.h"

#define TS_TICKET_INFO_DEF(_) \
	_(simple)(uint32_t, ticket_handle) \
	_(simple)(int32_t, ticket_count)

CREATE_STRUCT(TS_TICKET_INFO);

#define TS_CRACKER_INFO_DEF(_) \
	_(simple)(uint32_t, cracker_handle) \
	_(simple)(int32_t, cracker_count)

CREATE_STRUCT(TS_CRACKER_INFO);

#define TS_CS_FOSTER_CREATURE_DEF(_) \
	_(simple)(uint32_t, creature_card_handle) \
	_(count)(int32_t, ticket_info) \
	_(count)(int32_t, cracker_info) \
	_(dynarray)(TS_TICKET_INFO, ticket_info) \
	_(dynarray)(TS_CRACKER_INFO, cracker_info)

// Since EPIC_7_3
CREATE_PACKET(TS_CS_FOSTER_CREATURE, 6002);

#endif // PACKETS_TS_CS_FOSTER_CREATURE_H
