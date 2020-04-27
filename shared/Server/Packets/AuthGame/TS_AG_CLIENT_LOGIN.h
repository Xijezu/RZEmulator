#ifndef PACKETS_TS_AG_CLIENT_LOGIN_H
#define PACKETS_TS_AG_CLIENT_LOGIN_H

#include "Packets/PacketDeclaration.h"

#define TS_AG_CLIENT_LOGIN_DEF(_)                                                                                                                                             \
    _(string)                                                                                                                                                                 \
    (account, 61) _(simple)(uint32_t, nAccountID) _(simple)(uint16_t, result) _(simple)(uint32_t, permission) _(simple)(uint8_t, nPCBangUser) _(simple)(uint32_t, nEventCode) \
        _(simple)(uint32_t, nAge) _(simple)(uint32_t, nContinuousPlayTime) _(simple)(uint32_t, nContinuousLogoutTime)
CREATE_PACKET(TS_AG_CLIENT_LOGIN, 20011);

#endif // PACKETS_TS_AG_CLIENT_LOGIN_H
