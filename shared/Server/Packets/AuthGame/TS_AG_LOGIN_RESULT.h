#ifndef PACKETS_TS_AG_LOGIN_RESULT_H
#define PACKETS_TS_AG_LOGIN_RESULT_H

#include "Packets/PacketDeclaration.h"

#define TS_AG_LOGIN_RESULT_DEF(_) _(simple)(uint16_t, result)

CREATE_PACKET(TS_AG_LOGIN_RESULT, 20002);

#endif // PACKETS_TS_AG_LOGIN_RESULT_H
