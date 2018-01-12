//
// Created by xijezu on 29.11.17.
//

#ifndef __GAMEAUTHPACKETS_H
#define __GAMEAUTHPACKETS_H

#include "Server/TS_MESSAGE.h"

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif



typedef struct TS_GA_LOGIN : public TS_MESSAGE {
    uint16_t server_idx;
    char     server_name[21];
    char     server_screenshot_url[256];
    char     is_adult_server;
    char     server_ip[16];
    int32_t  server_port;
} GA_LOGIN;

typedef struct TS_GA_CLIENT_LOGIN : public TS_MESSAGE {
    char               account[61];
    unsigned long long one_time_key;
} GA_CLIENT_LOGIN;

typedef struct TS_GA_CLIENT_LOGOUT : public TS_MESSAGE {
    char     account[61];
    uint32_t nContinuousPlayTime;
} GA_CLIENT_LOGOUT;

typedef struct TS_GA_CLIENT_KICK_FAILED : public TS_MESSAGE {
    char account[61];
} GA_CLIENT_KICK_FAILED;


// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif // __GAMEAUTHPACKETS_H
