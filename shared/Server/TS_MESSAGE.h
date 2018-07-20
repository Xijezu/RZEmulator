#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */
enum TS_ResultCode
{
    TS_RESULT_SUCCESS                               = 0x0,
    TS_RESULT_NOT_EXIST                             = 0x1,
    TS_RESULT_TOO_FAR                               = 0x2,
    TS_RESULT_NOT_OWN                               = 0x3,
    TS_RESULT_MISC                                  = 0x4,
    TS_RESULT_NOT_ACTABLE                           = 0x5,
    TS_RESULT_ACCESS_DENIED                         = 0x6,
    TS_RESULT_UNKNOWN                               = 0x7,
    TS_RESULT_DB_ERROR                              = 0x8,
    TS_RESULT_ALREADY_EXIST                         = 0x9,
    TS_RESULT_NOT_ENOUGH_MONEY                      = 0xA,
    TS_RESULT_TOO_HEAVY                             = 0xB,
    TS_RESULT_NOT_ENOUGH_JP                         = 0xC,
    TS_RESULT_NOT_ENOUGH_LEVEL                      = 0xD,
    TS_RESULT_NOT_ENOUGH_JOB_LEVEL                  = 0xE,
    TS_RESULT_NOT_ENOUGH_SKILL                      = 0xF,
    TS_RESULT_LIMIT_MAX                             = 0x10,
    TS_RESULT_LIMIT_MIN                             = 0x11,
    TS_RESULT_INVALID_PASSWORD                      = 0x12,
    TS_RESULT_INVALID_TEXT                          = 0x13,
    TS_RESULT_NOT_ENOUGH_HP                         = 0x14,
    TS_RESULT_NOT_ENOUGH_MP                         = 0x15,
    TS_RESULT_COOL_TIME                             = 0x16,
    TS_RESULT_LIMIT_WEAPON                          = 0x17,
    TS_RESULT_LIMIT_RACE                            = 0x18,
    TS_RESULT_LIMIT_JOB                             = 0x19,
    TS_RESULT_LIMIT_TARGET                          = 0x1A,
    TS_RESULT_NO_SKILL                              = 0x1B,
    TS_RESULT_INVALID_ARGUMENT                      = 0x1C,
    TS_RESULT_PK_LIMIT                              = 0x1D,
    TS_RESULT_NOT_ENOUGH_HAVOC                      = 0x1E,
    TS_RESULT_NOT_ENOUGH_ENERGY                     = 0x1F,
    TS_RESULT_NOT_ENOUGH_BULLET                     = 0x20,
    TS_RESULT_NOT_ENOUGH_EXP                        = 0x21,
    TS_RESULT_NOT_ENOUGH_ITEM                       = 0x22,
    TS_RESULT_LIMIT_RIDING                          = 0x23,
    TS_RESULT_NOT_ENOUGH_SP                         = 0x24,
    TS_RESULT_ALREADY_STAMINA_SAVED                 = 0x25,
    TS_RESULT_NOT_ENOUGH_AGE                        = 0x26,
    TS_RESULT_WITHDRAW_WAITING                      = 0x27,
    TS_RESULT_REALNAME_REQUIRED                     = 0x28,
    TS_RESULT_GAMETIME_TIRED_STAMINA_SAVER          = 0x29,
    TS_RESULT_GAMETIME_HARMFUL_STAMINA_SAVER        = 0x2A,
    TS_RESULT_NOT_ACTABLE_IN_SIEGE_OR_RAID          = 0x2C,
    TS_RESULT_NOT_ACTABLE_IN_SECROUTE               = 0x2D,
    TS_RESULT_NOT_ACTABLE_IN_EVENTMAP               = 0x2E,
    TS_RESULT_TARGET_IN_SIEGE_OR_RAID               = 0x2F,
    TS_RESULT_TARGET_IN_SECROUTE                    = 0x30,
    TS_RESULT_TARGET_IN_EVENTMAP                    = 0x31,
    TS_RESULT_TOO_CHEAP                             = 0x32,
    TS_RESULT_NOT_ACTABLE_WHILE_USING_STORAGE       = 0x33,
    TS_RESULT_NOT_ACTABLE_WHILE_TRADING             = 0x34,
    TS_RESULT_TOO_MUCH_MONEY                        = 0x35,
    TS_RESULT_PASSWORD_MISMATCH                     = 0x36,
    TS_RESULT_NOT_ACTABLE_WHILE_USING_BOOTH         = 0x37,
    TS_RESULT_NOT_ACTABLE_IN_HUNTAHOLIC             = 0x38,
    TS_RESULT_TARGET_IN_HUNTAHOLIC                  = 0x39,
    TS_RESULT_NOT_ENOUGH_HUNTAHOLIC_POINT           = 0x3A,
    TS_RESULT_ACTABLE_IN_ONLY_HUNTAHOLIC            = 0x3B,
    TS_RESULT_IP_BLOCKED                            = 0x3C,
    TS_RESULT_ALREADY_IN_COMPETE                    = 0x3D,
    TS_RESULT_NOT_IN_COMPETE                        = 0x3E,
    TS_RESULT_WAITING_COMPETE_REQUEST_ANSWER        = 0x3F,
    TS_RESULT_NOT_IN_COMPETIBLE_PLACE               = 0x40,
    TS_RESULT_TARGET_ALREADY_IN_COMPETE             = 0x41,
    TS_RESULT_TARGET_NOT_IN_COMPETE                 = 0x42,
    TS_RESULT_TARGET_WAITING_COMPETE_REQUEST_ANSWER = 0x43,
    TS_RESULT_TARGET_NOT_IN_COMPETIBLE_PLACE        = 0x44,
    TS_RESULT_NOT_ACTABLE_HERE                      = 0x45,
    TS_RESULT_ALREADY_TAMING                        = 0x46,
    TS_RESULT_GAMETIME_LIMITED                      = 0x47,
    TS_RESULT_ERROR_MAX                             = 0x48,

    //Client side errors
            TS_RESULT_CLIENT_SIDE_ERROR = 0xFFFF
};

enum TS_AG_GA_PACKETS
{
    TS_GA_LOGIN              = 20001,
    TS_AG_LOGIN_RESULT       = 20002,
    TS_GA_CLIENT_LOGIN       = 20010,
    TS_AG_CLIENT_LOGIN       = 20011,
    TS_GA_CLIENT_LOGOUT      = 20012,
    TS_AG_KICK_CLIENT        = 20013,
    TS_GA_CLIENT_KICK_FAILED = 20014
};

enum TS_AC_CA_PACKETS
{
    TS_CA_PING          = 9999,
    TS_AC_RESULT        = 10000,
    TS_CA_VERSION       = 10001,
    TS_CA_ACCOUNT       = 10010,
    TS_CA_SERVER_LIST   = 10021,
    TS_AC_SERVER_LIST   = 10022,
    TS_CA_SELECT_SERVER = 10023,
    TS_AC_SELECT_SERVER = 10024
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

struct TS_MESSAGE
{
    uint32_t size;
    uint16_t id;
    uint8_t  msg_check_sum;

    inline void SetChecksum()
    {

        msg_check_sum += size & 0xFF;
        msg_check_sum += (size >> 8) & 0xFF;
        msg_check_sum += (size >> 16) & 0xFF;
        msg_check_sum += (size >> 24) & 0xFF;

        msg_check_sum += id & 0xFF;
        msg_check_sum += (id >> 8) & 0xFF;
    }

    static inline uint8_t GetChecksum(int id, int size)
    {
        uint8_t value = 0;

        value += size & 0xFF;
        value += (size >> 8) & 0xFF;
        value += (size >> 16) & 0xFF;
        value += (size >> 24) & 0xFF;

        value += id & 0xFF;
        value += (id >> 8) & 0xFF;

        return value;
    }
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

enum class ReadDataHandlerResult
{
        Ok              = 0,
        Error           = 1,
        WaitingForQuery = 2
};