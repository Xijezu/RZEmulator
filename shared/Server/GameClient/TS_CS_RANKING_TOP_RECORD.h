#ifndef PACKETS_TS_CS_RANKING_TOP_RECORD_H
#define PACKETS_TS_CS_RANKING_TOP_RECORD_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_RANKING_TOP_RECORD_DEF(_) \
	_(simple)(int8_t, ranking_type)

CREATE_PACKET(TS_CS_RANKING_TOP_RECORD, 5000);

#endif // PACKETS_TS_CS_RANKING_TOP_RECORD_H
