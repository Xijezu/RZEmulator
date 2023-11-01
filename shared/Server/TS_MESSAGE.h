#pragma once
/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include "ClientPackets.h"

enum TS_UNIT_FLAG {
    FLAG_BATTLE_MODE = 1 << 0,
    FLAG_INVISIBLE = 1 << 1,
};

enum TS_NPC_FLAG {
    FLAG_HAS_STARTABLE_QUEST = 1 << 8,
    FLAG_HAS_IN_PROGRESS_QUEST = 1 << 9,
    FLAG_HAS_FINISHABLE_QUEST = 1 << 10,
};

enum TS_PLAYER_FLAG {
    FLAG_SITDOWN = 1 << 8,
    FLAG_BUY_BOOTH = 1 << 9,
    FLAG_SELL_BOOTH = 1 << 10,
    FLAG_PK_ON = 1 << 11,
    FLAG_BLOODY = 1 << 12,
    FLAG_DEMONIAC = 1 << 13,
    FLAG_GM = 1 << 14,
    FLAG_WALKING = 1 << 16,
    FLAG_SHOVELING_SEARCH = 1 << 18,
    FLAG_SHOVELING_APPROACH = 1 << 19,
    FLAG_SHOVELING_DIG = 1 << 20,
    FLAG_COMPETING = 1 << 21,
    FLAG_BATTLE_ARENA_TEAM_0 = 1 << 22,
    FLAG_BATTLE_ARENA_TEAM_1 = 1 << 23,
};

enum TS_MONSTER_FLAG {
    FLAG_DEAD = 1 << 8,
    FLAG_DUNGEON_ORIGINAL_OWNER = 1 << 15,
    FLAG_DUNGEON_ORIGINAL_SIEGER = 1 << 17,
};

enum TS_ResultCode {
    TS_RESULT_SUCCESS = 0,
    TS_RESULT_NOT_EXIST = 1,
    TS_RESULT_TOO_FAR = 2,
    TS_RESULT_NOT_OWN = 3,
    TS_RESULT_MISC = 4,
    TS_RESULT_NOT_ACTABLE = 5,
    TS_RESULT_ACCESS_DENIED = 6,
    TS_RESULT_UNKNOWN = 7,
    TS_RESULT_DB_ERROR = 8,
    TS_RESULT_ALREADY_EXIST = 9,
    TS_RESULT_NOT_ENOUGH_MONEY = 10,
    TS_RESULT_TOO_HEAVY = 11,
    TS_RESULT_NOT_ENOUGH_JP = 12,
    TS_RESULT_NOT_ENOUGH_LEVEL = 13,
    TS_RESULT_NOT_ENOUGH_JOB_LEVEL = 14,
    TS_RESULT_NOT_ENOUGH_SKILL = 15,
    TS_RESULT_LIMIT_MAX = 16,
    TS_RESULT_LIMIT_MIN = 17,
    TS_RESULT_INVALID_PASSWORD = 18,
    TS_RESULT_INVALID_TEXT = 19,
    TS_RESULT_NOT_ENOUGH_HP = 20,
    TS_RESULT_NOT_ENOUGH_MP = 21,
    TS_RESULT_COOL_TIME = 22,
    TS_RESULT_LIMIT_WEAPON = 23,
    TS_RESULT_LIMIT_RACE = 24,
    TS_RESULT_LIMIT_JOB = 25,
    TS_RESULT_LIMIT_TARGET = 26,
    TS_RESULT_NO_SKILL = 27,
    TS_RESULT_INVALID_ARGUMENT = 28,
    TS_RESULT_PK_LIMIT = 29,
    TS_RESULT_NOT_ENOUGH_ENERGY = 31,
    TS_RESULT_NOT_ENOUGH_BULLET = 32,
    TS_RESULT_NOT_ENOUGH_EXP = 33,
    TS_RESULT_NOT_ENOUGH_ITEM = 34,
    TS_RESULT_LIMIT_RIDING = 35,
    TS_RESULT_NOT_ENOUGH_SP = 36,
    TS_RESULT_ALREADY_STAMINA_SAVED = 37,
    TS_RESULT_NOT_ENOUGH_AGE = 38,
    TS_RESULT_WITHDRAW_WAITING = 39,
    TS_RESULT_REALNAME_REQUIRED = 40,
    TS_RESULT_GAMETIME_TIRED_STAMINA_SAVER = 41,
    TS_RESULT_GAMETIME_HARMFUL_STAMINA_SAVER = 42,
    TS_RESULT_NOT_ACTABLE_IN_SIEGE_OR_RAID = 44,
    TS_RESULT_NOT_ACTABLE_IN_SECROUTE = 45,
    TS_RESULT_NOT_ACTABLE_IN_EVENTMAP = 46,
    TS_RESULT_TARGET_IN_SIEGE_OR_RAID = 47,
    TS_RESULT_TARGET_IN_SECROUTE = 48,
    TS_RESULT_TARGET_IN_EVENTMAP = 49,
    TS_RESULT_TOO_CHEAP = 50,
    TS_RESULT_NOT_ACTABLE_WHILE_USING_STORAGE = 51,
    TS_RESULT_NOT_ACTABLE_WHILE_TRADING = 52,
    TS_RESULT_TOO_MUCH_MONEY = 53,
    TS_RESULT_PASSWORD_MISMATCH = 54,
    TS_RESULT_NOT_ACTABLE_WHILE_USING_BOOTH = 55,
    TS_RESULT_NOT_ACTABLE_IN_HUNTAHOLIC = 56,
    TS_RESULT_TARGET_IN_HUNTAHOLIC = 57,
    TS_RESULT_NOT_ENOUGH_HUNTAHOLIC_POINT = 58,
    TS_RESULT_ACTABLE_IN_ONLY_HUNTAHOLIC = 59,
    TS_RESULT_IP_BLOCKED = 60,
    TS_RESULT_ALREADY_IN_COMPETE = 61,
    TS_RESULT_NOT_IN_COMPETE = 62,
    TS_RESULT_WAITING_COMPETE_REQUEST_ANSWER = 63,
    TS_RESULT_NOT_IN_COMPETIBLE_PLACE = 64,
    TS_RESULT_TARGET_ALREADY_IN_COMPETE = 65,
    TS_RESULT_TARGET_NOT_IN_COMPETE = 66,
    TS_RESULT_TARGET_WAITING_COMPETE_REQUEST_ANSWER = 67,
    TS_RESULT_TARGET_NOT_IN_COMPETIBLE_PLACE = 68,
    TS_RESULT_NOT_ACTABLE_HERE = 69,
    TS_RESULT_GAMETIME_LIMITED = 71,
    TS_RESULT_NOT_ACTABLE_IN_DEATHMATCH = 72,
    TS_RESULT_ACTABLE_IN_ONLY_DEATHMATCH = 73,
    TS_RESULT_BLOCK_CHAT = 74, 
    TS_RESULT_ENHANCE_LIMIT = 76,
    TS_RESULT_PENDING = 77,
    TS_RESULT_NOT_ACTABLE_IN_SECRET_DUNGEON = 78,
    TS_RESULT_TARGET_IN_SECRET_DUNGEON = 79,
    TS_RESULT_ALREADY_SUPER_SAVER = 80,
    TS_RESULT_GAMETIME_TIRED_SUPER_SAVER = 81,
    TS_RESULT_GAMETIME_HARMFUL_SUPER_SAVER = 82,
    TS_RESULT_NOT_ENOUGH_TP = 83,
    TS_RESULT_NOT_ACTABLE_IN_INSTANCE_DUNGEON = 84,
    TS_RESULT_ACTABLE_IN_ONLY_INSTANCE_DUNGEON = 85,
    TS_RESULT_TARGET_IN_INSTANCE_DUNGEON = 86,
    TS_RESULT_TARGET_IN_DEATHMATCH = 87,
    TS_RESULT_TARGET_IS_USING_STORAGE = 88,
    TS_RESULT_NOT_ENOUGH_AGE_PERIOD = 89,
    TS_RESULT_ALREADY_TAMING = 70,
    TS_RESULT_NOT_TAMABLE = 90,
    TS_RESULT_TARGET_ALREADY_BEING_TAMED = 91,
    TS_RESULT_NOT_ENOUGH_TARGET_HP = 92,
    TS_RESULT_NOT_ENOUGH_SUMMON_CARD = 93,
    TS_RESULT_NOT_ENOUGH_SOUL_TAMING_CARD = 94,
    TS_RESULT_NOT_ACTABLE_IN_BATTLE_ARENA = 95,
    TS_RESULT_NOT_READY = 96,
    TS_RESULT_TARGET_IN_BATTLE_ARENA = 97,
    TS_RESULT_NOT_ACTABLE_ON_STAND_UP = 98,
    TS_RESULT_NOT_ENOUGH_ARENA_POINT = 99,
    TS_RESULT_SUCCESS_WITHOUT_NOTICE = 101,
    TS_RESULT_WEBZEN_DUPLICATE_ACCOUNT = 102,
    TS_RESULT_WEBZEN_NEED_ACCEPT_EULA = 103,
    TS_RESULT_ERROR_MAX
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

struct TS_MESSAGE {
    uint32_t size;
    uint16_t id;
    uint8_t msg_check_sum;

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

enum class ReadDataHandlerResult { Ok = 0, Error = 1, WaitingForQuery = 2 };