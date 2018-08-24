#ifndef PACKETS_TS_SC_CHANGE_NAME_H
#define PACKETS_TS_SC_CHANGE_NAME_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_CHANGE_NAME_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(string) (name, 19)

CREATE_PACKET(TS_SC_CHANGE_NAME, 30);

#endif // PACKETS_TS_SC_CHANGE_NAME_H
