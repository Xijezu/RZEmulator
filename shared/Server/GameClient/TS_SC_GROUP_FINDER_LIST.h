#ifndef PACKETS_TS_SC_GROUP_FINDER_LIST_H
#define PACKETS_TS_SC_GROUP_FINDER_LIST_H

#include "Packet/PacketDeclaration.h"

#define TS_GROUP_FINDER_GROUP_DEF(_) \
	_(simple)(int32_t, index) \
	_(simple)(int32_t, group_handle) \
	_(simple)(int16_t, zone) \
	_(simple)(int16_t, min_level) \
	_(simple)(int16_t, player_num) \
	_(simple)(int16_t, max_player_num) \
	_(string)(description, 32)
CREATE_STRUCT(TS_GROUP_FINDER_GROUP);

#define TS_SC_GROUP_FINDER_LIST_DEF(_) \
	_(simple)(int32_t, page_num) \
	_(simple)(int32_t, total_group_num) \
	_(array)(TS_GROUP_FINDER_GROUP, groups, 10)

CREATE_PACKET(TS_SC_GROUP_FINDER_LIST, 7001);

#endif // PACKETS_TS_SC_GROUP_FINDER_LIST_H
