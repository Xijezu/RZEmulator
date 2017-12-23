//
// Created by xijezu on 29.11.17.
//

#ifndef __CLIENTAUTHPACKETS_H
#define __CLIENTAUTHPACKETS_H

#include "Server/TS_MESSAGE.h"

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

enum CAPACKETS {
	TS_CA_PING							= 9999,
	TS_CA_VERSION						= 10001,
	TS_CA_ACCOUNT		                = 10010,
	TS_CA_SERVER_LIST					= 10021,
	TS_CA_SELECT_SERVER					= 10023
};

typedef struct TS_CA_ACCOUNT : public TS_MESSAGE {
#if EPIC == 4
	char account[19];
#else
	char account[61];
#endif
	unsigned char password[32];
} CA_ACCOUNT;

typedef struct TS_CA_SELECT_SERVER : public TS_MESSAGE {
	uint16 server_idx;
}CA_SELECT_SERVER;

typedef struct TS_CA_VERSION : public TS_MESSAGE {
	char szVersion[20];
}CA_VERSION;



// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif // __CLIENTAUTHPACKETS_H
