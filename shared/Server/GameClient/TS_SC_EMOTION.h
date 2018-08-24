#ifndef PACKETS_TS_SC_EMOTION_H
#define PACKETS_TS_SC_EMOTION_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_EMOTION_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(simple)(int32_t, emotion)

CREATE_PACKET(TS_SC_EMOTION, 1201);

#endif // PACKETS_TS_SC_EMOTION_H
