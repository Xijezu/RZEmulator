#ifndef PACKETS_TS_CS_PUTON_CARD_H
#define PACKETS_TS_CS_PUTON_CARD_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_CARD_DEF(_) \
	_(simple)(int8_t, position) \
	_(simple)(uint32_t, item_handle)

CREATE_PACKET(TS_CS_PUTON_CARD, 214);

#endif // PACKETS_TS_CS_PUTON_CARD_H
