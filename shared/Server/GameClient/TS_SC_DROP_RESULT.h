#ifndef PACKETS_TS_SC_DROP_RESULT_H
#define PACKETS_TS_SC_DROP_RESULT_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_DROP_RESULT_DEF(_) \
	_(simple)(uint32_t, item_handle) \
	_(simple)(bool, isAccepted)

CREATE_PACKET(TS_SC_DROP_RESULT, 205);

#endif // PACKETS_TS_SC_DROP_RESULT_H
