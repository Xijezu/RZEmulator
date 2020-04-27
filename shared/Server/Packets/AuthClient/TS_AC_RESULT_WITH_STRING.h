#ifndef PACKETS_TS_AC_RESULT_WITH_STRING_H
#define PACKETS_TS_AC_RESULT_WITH_STRING_H

#include "Packets/PacketDeclaration.h"
#include "TS_AC_RESULT.h"

// not in 5.2 client

#define TS_AC_RESULT_WITH_STRING_DEF(_) _(simple)(uint16_t, request_msg_id) _(simple)(uint16_t, result) _(simple)(int32_t, login_flag) _(count)(int32_t, string) _(dynstring)(string, false)
CREATE_PACKET(TS_AC_RESULT_WITH_STRING, 10002);

#endif // PACKETS_TS_AC_RESULT_WITH_STRING_H
