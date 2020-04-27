#ifndef PACKETS_TS_CA_OTP_ACCOUNT_H
#define PACKETS_TS_CA_OTP_ACCOUNT_H

#include "Packets/PacketDeclaration.h"

#define TS_OTP_ACCOUNT_PASSWORD_PLAIN_DEF(_) \
    _(array)(uint8_t, password, 48)
CREATE_STRUCT(TS_OTP_ACCOUNT_PASSWORD_PLAIN);

#define TS_OTP_ACCOUNT_PASSWORD_AES_DEF(_) \
    _(simple)(uint32_t, password_size) \
    _(array)(uint8_t, password, 64)
CREATE_STRUCT(TS_OTP_ACCOUNT_PASSWORD_AES);

#define TS_CA_OTP_ACCOUNT_DEF(_) \
    _(def)(string)(account, 61) \
    _(impl)(string)(account, 61, version >= EPIC_5_1) \
    _(impl)(string)(account, 19, version <  EPIC_5_1) \
    _(simple)(TS_OTP_ACCOUNT_PASSWORD_PLAIN, passwordDes, version < EPIC_8_1_1_RSA) \
    _(simple)(TS_OTP_ACCOUNT_PASSWORD_AES, passwordAes, version >= EPIC_8_1_1_RSA)
CREATE_PACKET(TS_CA_OTP_ACCOUNT, 10011);

#endif // PACKETS_TS_CA_OTP_ACCOUNT_H
