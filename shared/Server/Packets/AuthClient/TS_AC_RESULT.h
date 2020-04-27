#ifndef PACKETS_TS_AC_RESULT_H
#define PACKETS_TS_AC_RESULT_H

#include "Packets/PacketDeclaration.h"

enum TS_LOGIN_SUCCESS_FLAG
{
    LSF_EULA_ACCEPTED              = 0x1,
    LSF_ACCOUNT_BLOCK_WARNING      = 0x2,
    LSF_DISTRIBUTION_CODE_REQUIRED = 0x4
};

#define TS_AC_RESULT_DEF(_) \
    _(simple)(uint16_t, request_msg_id) \
    _(simple)(uint16_t, result) \
    _(simple)(int32_t, login_flag)
CREATE_PACKET(TS_AC_RESULT, 10000);

#endif // PACKETS_TS_AC_RESULT_H
