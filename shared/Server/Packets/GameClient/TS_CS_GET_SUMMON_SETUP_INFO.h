#ifndef PACKETS_TS_CS_GET_SUMMON_SETUP_INFO_H
#define PACKETS_TS_CS_GET_SUMMON_SETUP_INFO_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_GET_SUMMON_SETUP_INFO_DEF(_) \
	_(simple)(bool, show_dialog)

CREATE_PACKET(TS_CS_GET_SUMMON_SETUP_INFO, 324);

#endif // PACKETS_TS_CS_GET_SUMMON_SETUP_INFO_H
