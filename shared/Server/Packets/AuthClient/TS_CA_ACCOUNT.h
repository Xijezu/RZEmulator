#ifndef PACKETS_TS_CA_ACCOUNT_H
#define PACKETS_TS_CA_ACCOUNT_H

#include "Packets/PacketDeclaration.h"

enum TS_ADDITIONAL_INFO_TYPE : int8_t
{
    TAIT_BLACKBOX = 0,
    TAIT_MACADDRESS = 1
};

#define TS_ACCOUNT_ADDITIONAL_INFO_DEF(_) _(simple)(TS_ADDITIONAL_INFO_TYPE, type) _(count)(uint16_t, data) _(dynarray)(uint8_t, data)
CREATE_STRUCT(TS_ACCOUNT_ADDITIONAL_INFO);

#define TS_ACCOUNT_PASSWORD_DES_DEF(_) _(def)(array)(uint8_t, password, 61)_(impl)(array)(uint8_t, password, 61, version >= EPIC_5_2) _(impl)(array)(uint8_t, password, 32, version < EPIC_5_2)
CREATE_STRUCT(TS_ACCOUNT_PASSWORD_DES);

#define TS_ACCOUNT_PASSWORD_AES_DEF(_) _(simple)(uint32_t, password_size) _(array)(uint8_t, password, 77)
CREATE_STRUCT(TS_ACCOUNT_PASSWORD_AES);

#define TS_CA_ACCOUNT_DEF(_)                                                                                                                                                                          \
    _(def)                                                                                                                                                                                            \
    (string)(account, 61)_(impl)(string)(account, 61, version >= EPIC_5_2) _(impl)(string)(account, 19, version < EPIC_5_2) _(simple)(TS_ACCOUNT_PASSWORD_DES, passwordDes, version < EPIC_8_1_1_RSA) \
        _(simple)(TS_ACCOUNT_PASSWORD_AES, passwordAes, version >= EPIC_8_1_1_RSA) _(endarray)(TS_ACCOUNT_ADDITIONAL_INFO, additionalInfos)
CREATE_PACKET(TS_CA_ACCOUNT, 10010);

#endif // PACKETS_TS_CA_ACCOUNT_H
