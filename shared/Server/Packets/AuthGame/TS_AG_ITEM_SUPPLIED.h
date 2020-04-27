#ifndef PACKETS_TS_AG_ITEM_SUPPLIED_H
#define PACKETS_TS_AG_ITEM_SUPPLIED_H

#include "Packets/PacketDeclaration.h"

#define TS_AG_ITEM_SUPPLIED_DEF(_) _(string)(account, 61)

CREATE_PACKET(TS_AG_ITEM_SUPPLIED, 30001);

#endif // PACKETS_TS_AG_ITEM_SUPPLIED_H
