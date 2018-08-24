#ifndef PACKETS_TS_CS_EMOTION_H
#define PACKETS_TS_CS_EMOTION_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_EMOTION_DEF(_) \
	_(simple)(int32_t, emotion)

CREATE_PACKET(TS_CS_EMOTION, 1202);

#endif // PACKETS_TS_CS_EMOTION_H
