#ifndef PACKETS_TS_CS_HUNTAHOLIC_JOIN_INSTANCE_H
#define PACKETS_TS_CS_HUNTAHOLIC_JOIN_INSTANCE_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_HUNTAHOLIC_JOIN_INSTANCE_DEF(_) \
	_(simple)(int32_t, instance_no) \
	_(string)(password, 17)

CREATE_PACKET(TS_CS_HUNTAHOLIC_JOIN_INSTANCE, 4004);

#endif // PACKETS_TS_CS_HUNTAHOLIC_JOIN_INSTANCE_H
