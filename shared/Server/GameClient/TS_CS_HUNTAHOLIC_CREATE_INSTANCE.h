#ifndef PACKETS_TS_CS_HUNTAHOLIC_CREATE_INSTANCE_H
#define PACKETS_TS_CS_HUNTAHOLIC_CREATE_INSTANCE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_HUNTAHOLIC_CREATE_INSTANCE_DEF(_) \
	_(string)(name, 31) \
	_(simple)(int8_t, max_member_count) \
	_(string)(password, 17)

CREATE_PACKET(TS_CS_HUNTAHOLIC_CREATE_INSTANCE, 4003);

#endif // PACKETS_TS_CS_HUNTAHOLIC_CREATE_INSTANCE_H
