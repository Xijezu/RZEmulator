#ifndef PACKETS_TS_CS_SET_MAIN_TITLE_H
#define PACKETS_TS_CS_SET_MAIN_TITLE_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_SET_MAIN_TITLE_DEF(_) \
	_(simple)(int32_t, code)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_SET_MAIN_TITLE, 628);

#endif // PACKETS_TS_CS_SET_MAIN_TITLE_H
