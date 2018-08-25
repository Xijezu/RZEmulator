#ifndef PACKETS_TS_AC_UPDATE_PENDING_TIME_H
#define PACKETS_TS_AC_UPDATE_PENDING_TIME_H

#include "Packets/PacketDeclaration.h"

//Not used in 6.1 server ...
#define TS_AC_UPDATE_PENDING_TIME_DEF(_) \
    _(simple)(uint32_t, pending_time)
CREATE_PACKET(TS_AC_UPDATE_PENDING_TIME, 10025);

#endif // PACKETS_TS_AC_UPDATE_PENDING_TIME_H
