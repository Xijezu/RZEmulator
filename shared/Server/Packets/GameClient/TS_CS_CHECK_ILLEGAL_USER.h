#ifndef PACKETS_TS_CS_CHECK_ILLEGAL_USER_H
#define PACKETS_TS_CS_CHECK_ILLEGAL_USER_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_CHECK_ILLEGAL_USER_DEF(_) \
	_(simple)(uint32_t, log_code)

CREATE_PACKET(TS_CS_CHECK_ILLEGAL_USER, 57);

#endif // PACKETS_TS_CS_CHECK_ILLEGAL_USER_H
