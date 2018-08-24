#ifndef PACKETS_TS_CS_SET_PROPERTY_H
#define PACKETS_TS_CS_SET_PROPERTY_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SET_PROPERTY_DEF(_) \
	_(string)(name, 16) \
	_(endstring)(string_value, true)

CREATE_PACKET(TS_CS_SET_PROPERTY, 508);

#endif // PACKETS_TS_CS_SET_PROPERTY_H
