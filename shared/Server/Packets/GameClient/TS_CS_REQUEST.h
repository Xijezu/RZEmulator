#ifndef PACKETS_TS_CS_REQUEST_H
#define PACKETS_TS_CS_REQUEST_H

#include "Server/Packets/PacketDeclaration.h"

#define TS_CS_REQUEST_DEF(_) \
    _(simple)                \
    (uint8_t, command_type)  \
        _(string)(command, 75)

CREATE_PACKET(TS_CS_REQUEST, 60);

#endif // PACKETS_TS_CS_REQUEST_H
