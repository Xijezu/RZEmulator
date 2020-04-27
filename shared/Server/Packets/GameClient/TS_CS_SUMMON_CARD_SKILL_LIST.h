#ifndef PACKETS_TS_CS_SUMMON_CARD_SKILL_LIST_H
#define PACKETS_TS_CS_SUMMON_CARD_SKILL_LIST_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_SUMMON_CARD_SKILL_LIST_DEF(_) \
	_(simple)(uint32_t, item_handle)

// Since EPIC_7_3
CREATE_PACKET(TS_CS_SUMMON_CARD_SKILL_LIST, 452);

#endif // PACKETS_TS_CS_SUMMON_CARD_SKILL_LIST_H
