#ifndef PACKETS_TS_AC_AES_KEY_IV_H
#define PACKETS_TS_AC_AES_KEY_IV_H

#include "Packets/PacketDeclaration.h"

#define TS_AC_AES_KEY_IV_DEF(_) _(count)(uint32_t, data) _(dynarray)(uint8_t, data)
CREATE_PACKET(TS_AC_AES_KEY_IV, 72);

#endif // PACKETS_TS_AC_AES_KEY_IV_H
