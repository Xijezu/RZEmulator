#ifndef PACKETS_TS_SC_ENERGY_H
#define PACKETS_TS_SC_ENERGY_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_ENERGY_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int16_t, energy)

CREATE_PACKET(TS_SC_ENERGY, 515);

#endif // PACKETS_TS_SC_ENERGY_H
