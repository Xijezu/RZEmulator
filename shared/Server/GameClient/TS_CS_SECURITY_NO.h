#ifndef PACKETS_TS_CS_SECURITY_NO_H
#define PACKETS_TS_CS_SECURITY_NO_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SECURITY_NO_DEF(_) \
	_(simple)(int32_t, mode) \
	_(string)(security_no, 19)

CREATE_PACKET(TS_CS_SECURITY_NO, 9005);

#endif // PACKETS_TS_CS_SECURITY_NO_H
