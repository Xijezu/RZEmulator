#ifndef PACKETS_TS_SC_TITLE_CONDITION_LIST_H
#define PACKETS_TS_SC_TITLE_CONDITION_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_TITLE_CONDITION_INFO_DEF(_) \
	_(simple)(int32_t, type) \
	_(simple)(int64_t, count)

CREATE_STRUCT(TS_TITLE_CONDITION_INFO);

#define TS_SC_TITLE_CONDITION_LIST_DEF(_) \
	_(count)(uint16_t, title_condition_infos) \
	_(dynarray)(TS_TITLE_CONDITION_INFO, title_condition_infos)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_TITLE_CONDITION_LIST, 626);

#endif // PACKETS_TS_SC_TITLE_CONDITION_LIST_H
