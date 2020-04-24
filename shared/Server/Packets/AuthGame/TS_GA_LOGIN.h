#ifndef PACKETS_TS_GA_LOGIN_H
#define PACKETS_TS_GA_LOGIN_H

#include "Packets/PacketDeclaration.h"

#define TS_GA_LOGIN_DEF(_) \
    _(simple)(uint16_t, server_idx) \
    _(string)(server_name, 21) \
    _(string)(server_screenshot_url, 256) \
    _(simple)(uint8_t, is_adult_server) \
    _(string)(server_ip, 16) \
    _(string)(server_external_ip, 16) \
    _(simple)(int32_t, server_port)
CREATE_PACKET(TS_GA_LOGIN, 20001);

#endif // PACKETS_TS_GA_LOGIN_H
