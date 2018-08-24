#ifndef PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_INFO_H
#define PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_HUNTAHOLIC_INSTANCE_INFO_DEF(_) \
	_(simple)(int32_t, instance_no) \
	_(string)(name, 31) \
	_(simple)(int8_t, current_member_count) \
	_(simple)(int8_t, max_member_count) \
	_(simple)(bool, require_password)

CREATE_STRUCT(TS_HUNTAHOLIC_INSTANCE_INFO);

#define TS_SC_HUNTAHOLIC_INSTANCE_INFO_DEF(_) \
	_(simple)(TS_HUNTAHOLIC_INSTANCE_INFO, info)

CREATE_PACKET(TS_SC_HUNTAHOLIC_INSTANCE_INFO, 4002);

#endif // PACKETS_TS_SC_HUNTAHOLIC_INSTANCE_INFO_H
