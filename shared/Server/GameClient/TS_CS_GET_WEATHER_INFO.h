#ifndef PACKETS_TS_CS_GET_WEATHER_INFO_H
#define PACKETS_TS_CS_GET_WEATHER_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_GET_WEATHER_INFO_DEF(_) \
	_(simple)(uint32_t, region_id)

CREATE_PACKET(TS_CS_GET_WEATHER_INFO, 903);

#endif // PACKETS_TS_CS_GET_WEATHER_INFO_H
