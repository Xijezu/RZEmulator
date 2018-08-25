#ifndef PACKETS_TS_CA_RSA_PUBLIC_KEY_H
#define PACKETS_TS_CA_RSA_PUBLIC_KEY_H

#include "Packets/PacketDeclaration.h"

#define TS_CA_RSA_PUBLIC_KEY_DEF(_) \
    _(count)(int32_t, key) \
    _(dynarray)(uint8_t, key)
CREATE_PACKET(TS_CA_RSA_PUBLIC_KEY, 71);

#endif // PACKETS_TS_CA_RSA_PUBLIC_KEY_H
