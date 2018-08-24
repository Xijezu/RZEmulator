#ifndef PACKETS_TS_SC_GROUP_FINDER_DETAIL_H
#define PACKETS_TS_SC_GROUP_FINDER_DETAIL_H

#include "Packet/PacketDeclaration.h"

// maybe one handle = master of group
#define TS_GROUP_FINDER_DETAIL_DEF(_) \
	_(simple)(uint16_t, job_id) \
	_(simple)(uint8_t, level) \
	_(simple)(uint8_t, flag) \
	_(simple)(uint32_t, handle) \
	_(string)(name, 20)
CREATE_STRUCT(TS_GROUP_FINDER_DETAIL);

#define TS_SC_GROUP_FINDER_DETAIL_DEF(_) \
	_(simple)(int32_t, index) \
	_(simple)(uint32_t, master_handle) \
	_(array)(TS_GROUP_FINDER_DETAIL, members, 10)

CREATE_PACKET(TS_SC_GROUP_FINDER_DETAIL, 7002);

#endif // PACKETS_TS_SC_GROUP_FINDER_DETAIL_H
