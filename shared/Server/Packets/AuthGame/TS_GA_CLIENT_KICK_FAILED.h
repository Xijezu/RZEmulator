#ifndef PACKETS_TS_GA_CLIENT_KICK_FAILED_H
#define PACKETS_TS_GA_CLIENT_KICK_FAILED_H

#include "Packets/PacketDeclaration.h"

#define TS_GA_CLIENT_KICK_FAILED_DEF(_) \
    _(string)(account, 61)

CREATE_PACKET(TS_GA_CLIENT_KICK_FAILED, 20014);

#endif // PACKETS_TS_GA_CLIENT_KICK_FAILED_H
