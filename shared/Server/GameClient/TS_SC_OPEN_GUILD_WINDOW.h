#ifndef PACKETS_TS_SC_OPEN_GUILD_WINDOW_H
#define PACKETS_TS_SC_OPEN_GUILD_WINDOW_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_OPEN_GUILD_WINDOW_DEF(_) \
	_(simple)(int32_t, client_id) \
	_(simple)(int32_t, account_id) \
	_(simple)(int32_t, one_time_password) \
	_(string)(raw_server_name, 32)

CREATE_PACKET(TS_SC_OPEN_GUILD_WINDOW, 651);

#endif // PACKETS_TS_SC_OPEN_GUILD_WINDOW_H
