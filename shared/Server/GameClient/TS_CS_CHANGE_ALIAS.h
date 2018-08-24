#ifndef PACKETS_TS_CS_CHANGE_ALIAS_H
#define PACKETS_TS_CS_CHANGE_ALIAS_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_CHANGE_ALIAS_DEF(_) \
	_(string)(alias, 19)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_CHANGE_ALIAS, 31);

#endif // PACKETS_TS_CS_CHANGE_ALIAS_H
