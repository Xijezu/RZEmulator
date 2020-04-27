#ifndef PACKETS_TS_GA_CLIENT_LOGOUT_H
#define PACKETS_TS_GA_CLIENT_LOGOUT_H

#include "Packets/PacketDeclaration.h"

#define TS_GA_CLIENT_LOGOUT_DEF(_) \
    _(string)(account, 61) \
    _(simple)(uint32_t, nContinuousPlayTime)
CREATE_PACKET(TS_GA_CLIENT_LOGOUT, 20012);

#endif // PACKETS_TS_GA_CLIENT_LOGOUT_H
