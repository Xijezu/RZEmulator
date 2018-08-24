#ifndef PACKETS_TS_SC_RANKING_TOP_RECORD_H
#define PACKETS_TS_SC_RANKING_TOP_RECORD_H

#include "Packet/PacketDeclaration.h"

#define TS_RANKING_RECORD_DEF(_) \
	_(simple)(uint16_t, rank) \
	_(string)(ranker_name, 31) \
	_(simple)(int64_t, score)

CREATE_STRUCT(TS_RANKING_RECORD);

#define TS_SC_RANKING_TOP_RECORD_DEF(_) \
	_(simple)(int8_t, ranking_type) \
	_(simple)(uint16_t, requester_rank) \
	_(simple)(int64_t, requester_score) \
	_(count)(uint16_t, records) \
	_(dynarray)(TS_RANKING_RECORD, records)

CREATE_PACKET(TS_SC_RANKING_TOP_RECORD, 5001);

#endif // PACKETS_TS_SC_RANKING_TOP_RECORD_H
