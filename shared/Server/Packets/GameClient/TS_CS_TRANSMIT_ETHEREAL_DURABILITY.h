#ifndef PACKETS_TS_CS_TRANSMIT_ETHEREAL_DURABILITY_H
#define PACKETS_TS_CS_TRANSMIT_ETHEREAL_DURABILITY_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_TRANSMIT_ETHEREAL_DURABILITY_DEF(_) \
	_(simple)(uint32_t, handle)

// Since EPIC_7_2
CREATE_PACKET(TS_CS_TRANSMIT_ETHEREAL_DURABILITY, 263);

#endif // PACKETS_TS_CS_TRANSMIT_ETHEREAL_DURABILITY_H