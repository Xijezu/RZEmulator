//
// Created by xijezu on 29.11.17.
//

#ifndef __AUTHGAMEPACKETS_H
#define __AUTHGAMEPACKETS_H

#include "Server/TS_MESSAGE.h"

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

enum AGPACKETS {
    TS_AG_LOGIN_RESULT = 20002,
    TS_AG_CLIENT_LOGIN = 20011,
    TS_AG_KICK_CLIENT  = 20013
};


typedef struct TS_AG_LOGIN_RESULT : public TS_MESSAGE {
    uint16_t result;
} AG_LOGIN_RESULT;

typedef struct TS_AG_CLIENT_LOGIN : public TS_MESSAGE {
    char   account[61];
    uint32 nAccountID;
    uint16 result;
    char   nPCBangUser;
    uint32 nEventCode;
    uint32 nAge;
    uint32 nContinuousPlayTime;
    uint32 nContinuousLogoutTime;
} AG_CLIENT_LOGIN;


// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif // __AUTHGAMEPACKETS_H
