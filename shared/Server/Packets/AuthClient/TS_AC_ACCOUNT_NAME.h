#ifndef PACKETS_TS_AC_ACCOUNT_NAME_H
#define PACKETS_TS_AC_ACCOUNT_NAME_H

#include "PacketDeclaration.h"

#define TS_AC_ACCOUNT_NAME_DEF(_) \
    _(string)(account, 61) \
    _(simple)(uint32_t, unknown_id)
CREATE_PACKET(TS_AC_ACCOUNT_NAME, 10014);

#endif // PACKETS_TS_AC_ACCOUNT_NAME_H
