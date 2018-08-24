#ifndef PACKETS_TS_CS_ACCOUNT_WITH_AUTH_H
#define PACKETS_TS_CS_ACCOUNT_WITH_AUTH_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_ACCOUNT_WITH_AUTH_DEF(_) \
	_(def)(string)(account, 61) \
	_(impl)(string)(account, 61, version >= EPIC_5_2) \
	_(impl)(string)(account, 19, version <  EPIC_5_2) \
	_(simple)(uint64_t, one_time_key)

CREATE_PACKET(TS_CS_ACCOUNT_WITH_AUTH, 2005);

#endif // PACKETS_TS_CS_ACCOUNT_WITH_AUTH_H
