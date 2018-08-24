#ifndef PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_LIST_H
#define PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_LIST_H

#include "Packet/PacketDeclaration.h"
#include "TS_SC_HUNTAHOLIC_INSTANCE_INFO.h"

#define TS_SC_HUNTAHOLIC_INSTANCE_LIST_DEF(_) \
	_(simple)(int32_t, huntaholic_id) \
	_(simple)(int32_t, page) \
	_(count)(int32_t, infos) \
	_(simple)(int32_t, total_page) \
	_(dynarray)(TS_HUNTAHOLIC_INSTANCE_INFO, infos)

CREATE_PACKET(TS_SC_HUNTAHOLIC_INSTANCE_LIST, 4001);

#endif // PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_LIST_H
