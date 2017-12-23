//
// Created by xijezu on 29.11.17.
//

#ifndef __AUTHCLIENTPACKETS_H
#define __AUTHCLIENTPACKETS_H

#include "Server/TS_MESSAGE.h"

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

enum ACPACKETS {
	TS_AC_RESULT						= 10000,
	TS_AC_SERVER_LIST					= 10022,
	TS_AC_SELECT_SERVER					= 10024
};

typedef struct TS_AC_RESULT : public TS_MESSAGE {
	uint16 request_msg_id;
	uint16 result;
	int32 login_flag;
}AC_RESULT;

struct TS_SERVER_INFO {
	uint16 idx;
	char server_name[21];
	char isAdultServer;
	char server_screenshot_url[256];
	char server_ip[16];
	int32 server_port;
	uint16 user_ratio;
};

typedef struct TS_AC_SERVER_LIST :public TS_MESSAGE {
	uint16 last_login_server_idx;
	uint16 count;
}AC_SERVER_LIST;


// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif // __AUTHCLIENTPACKETS_H
