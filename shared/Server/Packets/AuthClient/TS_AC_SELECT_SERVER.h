#ifndef PACKETS_TS_AC_SELECT_SERVER_H
#define PACKETS_TS_AC_SELECT_SERVER_H

#include "Packets/PacketDeclaration.h"

#define TS_AC_SELECT_SERVER_DEF(_) \
    _(simple)(uint16_t, result) \
    _(simple)(int64_t, one_time_key, version < EPIC_8_1_1_RSA) \
    _(simple)(int32_t, encrypted_data_size, version >= EPIC_8_1_1_RSA) \
    _(array)(uint8_t, encrypted_data, 24, version >= EPIC_8_1_1_RSA) \
    _(simple)(uint32_t, pending_time)
CREATE_PACKET(TS_AC_SELECT_SERVER, 10024);

#endif // PACKETS_TS_AC_SELECT_SERVER_H
