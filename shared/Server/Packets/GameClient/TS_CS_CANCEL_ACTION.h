#ifndef PACKETS_TS_CS_CANCEL_ACTION_H
#define PACKETS_TS_CS_CANCEL_ACTION_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_CANCEL_ACTION_DEF(_) \
	_(simple)(uint32_t, handle)

CREATE_PACKET(TS_CS_CANCEL_ACTION, 150);

#endif // PACKETS_TS_CS_CANCEL_ACTION_H
