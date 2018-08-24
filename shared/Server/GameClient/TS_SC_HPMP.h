#ifndef PACKETS_TS_SC_HPMP_H
#define PACKETS_TS_SC_HPMP_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_HPMP_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int32_t, add_hp) \
	_(simple)(int32_t, hp) \
	_(simple)(int32_t, max_hp) \
	_(def)(simple) (int32_t, add_mp) \
	_(impl)(simple)(int32_t, add_mp, version >= EPIC_4_1) \
	_(impl)(simple)(uint16_t, add_mp, version < EPIC_4_1) \
	_(def)(simple) (int32_t, mp) \
	_(impl)(simple)(int32_t, mp, version >= EPIC_4_1) \
	_(impl)(simple)(uint16_t, mp, version < EPIC_4_1) \
	_(def)(simple) (int32_t, max_mp) \
	_(impl)(simple)(int32_t, max_mp, version >= EPIC_4_1) \
	_(impl)(simple)(uint16_t, max_mp, version < EPIC_4_1) \
	_(simple)(bool, need_to_display, version >= EPIC_4_1)

CREATE_PACKET(TS_SC_HPMP, 509);

#endif // PACKETS_TS_SC_HPMP_H
