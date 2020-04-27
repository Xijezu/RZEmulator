#ifndef PACKETS_TS_AG_PCBANG_EXPIRE_H
#define PACKETS_TS_AG_PCBANG_EXPIRE_H

#include "Packets/PacketDeclaration.h"

#define TS_AG_PCBANG_EXPIRE_DEF(_) _(string)(account, 61)

CREATE_PACKET(TS_AG_PCBANG_EXPIRE, 20021);

#endif // PACKETS_TS_AG_PCBANG_EXPIRE_H
