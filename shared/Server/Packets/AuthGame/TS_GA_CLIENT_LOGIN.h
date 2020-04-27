#ifndef PACKETS_TS_GA_CLIENT_LOGIN_H
#define PACKETS_TS_GA_CLIENT_LOGIN_H

#include "Packets/PacketDeclaration.h"

#define TS_GA_CLIENT_LOGIN_DEF(_) _(string)(account, 61) _(simple)(uint64_t, one_time_key)

CREATE_PACKET(TS_GA_CLIENT_LOGIN, 20010);

#endif // PACKETS_TS_GA_CLIENT_LOGIN_H
