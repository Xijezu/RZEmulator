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

#include "Server/XPacket.h"

namespace NGemity
{
    enum class Packets
    {
            TS_SC_RESULT                                    = 0,
            TS_CS_LOGIN                                     = 1,
            TS_TIMESYNC                                     = 2,
            TS_SC_ENTER                                     = 3,
            TS_SC_LOGIN_RESULT                              = 4,
            TS_CS_MOVE_REQUEST                              = 5,
            TS_SC_MOVE_ACK                                  = 6,
            TS_CS_REGION_UPDATE                             = 7,
            TS_SC_MOVE                                      = 8,
            TS_SC_LEAVE                                     = 9,
            TS_SC_SET_TIME                                  = 10,
            TS_SC_REGION_ACK                                = 11,
            TS_SC_WARP                                      = 12,
            TS_CS_QUERY                                     = 13,
            TS_CS_ENTER_EVENT_AREA                          = 15,
            TS_CS_LEAVE_EVENT_AREA                          = 16,
            TS_CS_CHAT_REQUEST                              = 20,
            TS_SC_CHAT_LOCAL                                = 21,
            TS_SC_CHAT                                      = 22,
            TS_CS_RETURN_LOBBY                              = 23,
            TS_SC_CHAT_RESULT                               = 24,
            TS_CS_REQUEST_RETURN_LOBBY                      = 25,
            TS_CS_REQUEST_LOGOUT                            = 26,
            TS_CS_LOGOUT                                    = 27,
            TS_SC_DISCONNECT_DESC                           = 28,
            TS_SC_CHANGE_NAME                               = 30,
            TS_CS_CHANGE_ALIAS                              = 31,
            TS_CS_UNKN                                      = 50,
            TS_CS_VERSION                                   = 51,
            TS_CS_VERSION2                                  = 52,
            TS_SC_ANTI_HACK                                 = 53,
            TS_CS_ANTI_HACK                                 = 54,
            TS_SC_GAME_GUARD_AUTH_QUERY                     = 55,
            TS_CS_GAME_GUARD_AUTH_ANSWER                    = 56,
            TS_CS_CHECK_ILLEGAL_USER                        = 57,
            TS_SC_XTRAP_CHECK                               = 58,
            TS_CS_XTRAP_CHECK                               = 59,
            TS_CA_RSA_PUBLIC_KEY                            = 71,
            TS_AC_AES_KEY_IV                                = 72,
            TS_CS_ATTACK_REQUEST                            = 100,
            TS_SC_ATTACK_EVENT                              = 101,
            TS_SC_CANT_ATTACK                               = 102,
            TS_CS_CANCEL_ACTION                             = 150,
            TS_CS_PUTON_ITEM                                = 200,
            TS_CS_PUTOFF_ITEM                               = 201,
            TS_SC_WEAR_INFO                                 = 202,
            TS_CS_DROP_ITEM                                 = 203,
            TS_CS_TAKE_ITEM                                 = 204,
            TS_SC_DROP_RESULT                               = 205,
            TS_SC_INVENTORY                                 = 207,
            TS_CS_ERASE_ITEM                                = 208,
            TS_SC_ERASE_ITEM                                = 209,
            TS_SC_TAKE_ITEM_RESULT                          = 210,
            TS_SC_OPEN_STORAGE                              = 211,
            TS_CS_STORAGE                                   = 212,
            TS_SC_GET_CHAOS                                 = 213,
            TS_CS_PUTON_CARD                                = 214,
            TS_CS_PUTOFF_CARD                               = 215,
            TS_SC_BELT_SLOT_INFO                            = 216,
            TS_SC_ITEM_COOL_TIME                            = 217,
            TS_CS_CHANGE_ITEM_POSITION                      = 218,
            TS_CS_ARRANGE_ITEM                              = 219,
            TS_SC_HAIR_INFO                                 = 220,
            TS_CS_HIDE_EQUIP_INFO                           = 221,
            TS_SC_HIDE_EQUIP_INFO                           = 222,
            TS_CS_SWAP_EQUIP                                = 223,
            TS_SC_NPC_TRADE_INFO                            = 240,
            TS_SC_MARKET                                    = 250,
            TS_CS_BUY_ITEM                                  = 251,
            TS_CS_SELL_ITEM                                 = 252,
            TS_CS_USE_ITEM                                  = 253,
            TS_SC_DESTROY_ITEM                              = 254,
            TS_SC_UPDATE_ITEM_COUNT                         = 255,
            TS_CS_MIX                                       = 256,
            TS_SC_MIX_RESULT                                = 257,
            TS_CS_DONATE_ITEM                               = 258,
            TS_CS_DONATE_REWARD                             = 259,
            TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW               = 259,
            TS_CS_SOULSTONE_CRAFT                           = 260,
            TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW              = 261,
            TS_CS_REPAIR_SOULSTONE                          = 262,
            TS_CS_TRANSMIT_ETHEREAL_DURABILITY              = 263,
            TS_CS_TRANSMIT_ETHEREAL_DURABILITY_TO_EQUIPMENT = 264,
            TS_CS_DECOMPOSE                                 = 265,
            TS_SC_DECOMPOSE_RESULT                          = 266,
            TS_TRADE                                        = 280,
            TS_CS_PUTON_ITEM_SET                            = 281,
            TS_SC_ITEM_DROP_INFO                            = 282,
            TS_SC_USE_ITEM_RESULT                           = 283,
            TS_CS_BIND_SKILLCARD                            = 284,
            TS_CS_UNBIND_SKILLCARD                          = 285,
            TS_SC_SKILLCARD_INFO                            = 286,
            TS_SC_ITEM_WEAR_INFO                            = 287,
            TS_SC_ADD_SUMMON_INFO                           = 301,
            TS_SC_REMOVE_SUMMON_INFO                        = 302,
            TS_EQUIP_SUMMON                                 = 303,
            TS_CS_SUMMON                                    = 304,
            TS_SC_UNSUMMON                                  = 305,
            TS_SC_UNSUMMON_NOTICE                           = 306,
            TS_SC_SUMMON_EVOLUTION                          = 307,
            TS_SC_TAMING_INFO                               = 310,
            TS_SC_MOUNT_SUMMON                              = 320,
            TS_SC_UNMOUNT_SUMMON                            = 321,
            TS_SC_SHOW_SUMMON_NAME_CHANGE                   = 322,
            TS_CS_CHANGE_SUMMON_NAME                        = 323,
            TS_CS_GET_SUMMON_SETUP_INFO                     = 324,
            TS_SC_UNSUMMON_PET                              = 350,
            TS_SC_ADD_PET_INFO                              = 351,
            TS_SC_REMOVE_PET_INFO                           = 352,
            TS_SC_SHOW_SET_PET_NAME                         = 353,
            TS_CS_SET_PET_NAME                              = 354,
            TS_CS_SKILL                                     = 400,
            TS_SC_SKILL                                     = 401,
            TS_CS_LEARN_SKILL                               = 402,
            TS_SC_SKILL_LIST                                = 403,
            TS_SC_ADDED_SKILL_LIST                          = 404,
            TS_SC_STATE_RESULT                              = 406,
            TS_SC_AURA                                      = 407,
            TS_CS_REQUEST_REMOVE_STATE                      = 408,
            TS_CS_JOB_LEVEL_UP                              = 410,
            TS_SC_SKILL_LEVEL_LIST                          = 451,
            TS_CS_SUMMON_CARD_SKILL_LIST                    = 452,
            TS_SC_STATUS_CHANGE                             = 500,
            TS_CS_UPDATE                                    = 503,
            TS_SC_STATE                                     = 505,
            TS_SC_PROPERTY                                  = 507,
            TS_CS_SET_PROPERTY                              = 508,
            TS_SC_HPMP                                      = 509,
            TS_SC_REGEN_INFO                                = 510,
            TS_CS_TARGETING                                 = 511,
            TS_SC_TARGET                                    = 512,
            TS_CS_RESURRECTION                              = 513,
            TS_SC_SP                                        = 514,
            TS_SC_ENERGY                                    = 515,
            TS_SC_REGEN_HPMP                                = 516,
            TS_CS_MONSTER_RECOGNIZE                         = 517,
            TS_CS_GET_REGION_INFO                           = 550,
            TS_SC_QUEST_LIST                                = 600,
            TS_SC_QUEST_STATUS                              = 601,
            TS_SC_QUEST_INFOMATION                          = 602,
            TS_CS_DROP_QUEST                                = 603,
            TS_CS_QUEST_INFO                                = 604,
            TS_CS_END_QUEST                                 = 605,
            TS_SC_TITLE_LIST                                = 625,
            TS_SC_TITLE_CONDITION_LIST                      = 626,
            TS_SC_REMAIN_TITLE_TIME                         = 627,
            TS_CS_SET_MAIN_TITLE                            = 628,
            TS_SC_SET_MAIN_TITLE                            = 629,
            TS_CS_SET_SUB_TITLE                             = 630,
            TS_SC_SET_SUB_TITLE                             = 631,
            TS_CS_BOOKMARK_TITLE                            = 632,
            TS_SC_BOOKMARK_TITLE                            = 633,
            TS_SC_ACHIEVE_TITLE                             = 634,
            TS_SC_OPEN_TITLE                                = 635,
            TS_SC_CHANGE_TITLE_CONDITION                    = 636,
            TS_SC_SHOW_CREATE_GUILD                         = 650,
            TS_SC_OPEN_GUILD_WINDOW                         = 651,
            TS_SC_SHOW_CREATE_ALLIANCE                      = 660,
            TS_CS_START_BOOTH                               = 700,
            TS_CS_STOP_BOOTH                                = 701,
            TS_CS_WATCH_BOOTH                               = 702,
            TS_SC_WATCH_BOOTH                               = 703,
            TS_CS_STOP_WATCH_BOOTH                          = 704,
            TS_CS_BUY_FROM_BOOTH                            = 705,
            TS_CS_SELL_TO_BOOTH                             = 706,
            TS_CS_GET_BOOTHS_NAME                           = 707,
            TS_SC_GET_BOOTHS_NAME                           = 708,
            TS_SC_BOOTH_CLOSED                              = 709,
            TS_SC_BOOTH_TRADE_INFO                          = 710,
            TS_CS_CHECK_BOOTH_STARTABLE                     = 711,
            TS_CS_TURN_ON_PK_MODE                           = 800,
            TS_CS_TURN_OFF_PK_MODE                          = 801,
            TS_CS_CHANGE_LOCATION                           = 900,
            TS_SC_CHANGE_LOCATION                           = 901,
            TS_SC_WEATHER_INFO                              = 902,
            TS_CS_GET_WEATHER_INFO                          = 903,
            TS_SC_STAT_INFO                                 = 1000,
            TS_SC_GOLD_UPDATE                               = 1001,
            TS_SC_LEVEL_UPDATE                              = 1002,
            TS_SC_EXP_UPDATE                                = 1003,
            TS_SC_BONUS_EXP_JP                              = 1004,
            TS_SC_DETECT_RANGE_UPDATE                       = 1005,
            TS_CS_GAME_TIME                                 = 1100,
            TS_SC_GAME_TIME                                 = 1101,
            TS_SC_EMOTION                                   = 1201,
            TS_CS_EMOTION                                   = 1202,
            TS_CS_AUCTION_SEARCH                            = 1300,
            TS_SC_AUCTION_SEARCH                            = 1301,
            TS_CS_AUCTION_SELLING_LIST                      = 1302,
            TS_SC_AUCTION_SELLING_LIST                      = 1303,
            TS_CS_AUCTION_BIDDED_LIST                       = 1304,
            TS_SC_AUCTION_BIDDED_LIST                       = 1305,
            TS_CS_AUCTION_BID                               = 1306,
            TS_CS_AUCTION_INSTANT_PURCHASE                  = 1308,
            TS_CS_AUCTION_REGISTER                          = 1309,
            TS_CS_AUCTION_CANCEL                            = 1310,
            TS_CS_ITEM_KEEPING_LIST                         = 1350,
            TS_SC_ITEM_KEEPING_LIST                         = 1351,
            TS_CS_ITEM_KEEPING_TAKE                         = 1352,
            TS_CS_CHARACTER_LIST                            = 2001,
            TS_CS_CREATE_CHARACTER                          = 2002,
            TS_CS_DELETE_CHARACTER                          = 2003,
            TS_SC_CHARACTER_LIST                            = 2004,
            TS_CS_ACCOUNT_WITH_AUTH                         = 2005,
            TS_CS_CHECK_CHARACTER_NAME                      = 2006,
            TS_SC_DIALOG                                    = 3000,
            TS_CS_DIALOG                                    = 3001,
            TS_CS_CONTACT                                   = 3002,
            TS_SC_SHOW_WINDOW                               = 3003,
            TS_SC_GENERAL_MESSAGE_BOX                       = 3004,
            TS_CS_HUNTAHOLIC_INSTANCE_LIST                  = 4000,
            TS_SC_HUNTAHOLIC_INSTANCE_LIST                  = 4001,
            TS_SC_HUNTAHOLIC_INSTANCE_INFO                  = 4002,
            TS_CS_HUNTAHOLIC_CREATE_INSTANCE                = 4003,
            TS_CS_HUNTAHOLIC_JOIN_INSTANCE                  = 4004,
            TS_CS_HUNTAHOLIC_LEAVE_INSTANCE                 = 4005,
            TS_SC_HUNTAHOLIC_HUNTING_SCORE                  = 4006,
            TS_SC_HUNTAHOLIC_UPDATE_SCORE                   = 4007,
            TS_CS_HUNTAHOLIC_LEAVE_LOBBY                    = 4008,
            TS_SC_HUNTAHOLIC_BEGIN_HUNTING                  = 4009,
            TS_SC_HUNTAHOLIC_MAX_POINT_ACHIEVED             = 4010,
            TS_CS_HUNTAHOLIC_BEGIN_HUNTING                  = 4011,
            TS_SC_HUNTAHOLIC_BEGIN_COUNTDOWN                = 4012,
            TS_CS_INSTANCE_GAME_ENTER                       = 4250,
            TS_CS_INSTANCE_GAME_EXIT                        = 4251,
            TS_CS_INSTANCE_GAME_SCORE_REQUEST               = 4252,
            TS_SC_INSTANCE_GAME_SCORE_REQUEST               = 4253,
            TS_CS_COMPETE_REQUEST                           = 4500,
            TS_SC_COMPETE_REQUEST                           = 4501,
            TS_CS_COMPETE_ANSWER                            = 4502,
            TS_SC_COMPETE_ANSWER                            = 4503,
            TS_SC_COMPETE_COUNTDOWN                         = 4504,
            TS_SC_COMPETE_START                             = 4505,
            TS_SC_COMPETE_END                               = 4506,
            TS_SC_BATTLE_ARENA_PENALTY_INFO                 = 4700,
            TS_CS_BATTLE_ARENA_JOIN_QUEUE                   = 4701,
            TS_SC_BATTLE_ARENA_JOIN_QUEUE                   = 4702,
            TS_SC_BATTLE_ARENA_UPDATE_WAIT_USER_COUNT       = 4703,
            TS_CS_BATTLE_ARENA_LEAVE                        = 4704,
            TS_SC_BATTLE_ARENA_LEAVE                        = 4705,
            TS_SC_BATTLE_ARENA_BATTLE_INFO                  = 4706,
            TS_CS_BATTLE_ARENA_ENTER_WHILE_COUNTDOWN        = 4707,
            TS_CS_BATTLE_ARENA_EXERCISE_READY               = 4708,
            TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS        = 4709,
            TS_CS_BATTLE_ARENA_EXERCISE_START               = 4710,
            TS_SC_BATTLE_ARENA_BATTLE_STATUS                = 4711,
            TS_SC_BATTLE_ARENA_BATTLE_SCORE                 = 4712,
            TS_SC_BATTLE_ARENA_JOIN_BATTLE                  = 4713,
            TS_SC_BATTLE_ARENA_DISCONNECT_BATTLE            = 4714,
            TS_SC_BATTLE_ARENA_RECONNECT_BATTLE             = 4715,
            TS_SC_BATTLE_ARENA_RESULT                       = 4716,
            TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST        = 4717,
            TS_SC_BATTLE_ARENA_ABSENCE_CHECK                = 4718,
            TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER         = 4719,
            TS_CS_RANKING_TOP_RECORD                        = 5000,
            TS_SC_RANKING_TOP_RECORD                        = 5001,
            TS_CS_REQUEST_FARM_INFO                         = 6000,
            TS_SC_FARM_INFO                                 = 6001,
            TS_CS_FOSTER_CREATURE                           = 6002,
            TS_SC_RESULT_FOSTER                             = 6003,
            TS_CS_RETRIEVE_CREATURE                         = 6004,
            TS_SC_RESULT_RETRIEVE                           = 6005,
            TS_CS_NURSE_CREATURE                            = 6006,
            TS_SC_RESULT_NURSE                              = 6007,
            TS_CS_REQUEST_FARM_MARKET                       = 6008,
            TS_CS_GROUP_FINDER_LIST                         = 7000,
            TS_SC_GROUP_FINDER_LIST                         = 7001,
            TS_SC_GROUP_FINDER_DETAIL                       = 7002,
            TS_CS_REPORT                                    = 8000,
            TS_SC_OPEN_URL                                  = 9000,
            TS_SC_URL_LIST                                  = 9001,
            TS_SC_REQUEST_SECURITY_NO                       = 9004,
            TS_CS_SECURITY_NO                               = 9005,
            TS_CS_PING                                      = 9999,
            TS_CS_OPEN_ITEM_SHOP                            = 10000,
            TS_SC_OPEN_ITEM_SHOP     = 10001,
            // Between Game & Auth
            TS_GA_LOGIN              = 20001,
            TS_AG_LOGIN_RESULT       = 20002,
            TS_GA_CLIENT_LOGIN       = 20010,
            TS_AG_CLIENT_LOGIN       = 20011,
            TS_GA_CLIENT_LOGOUT      = 20012,
            TS_AG_KICK_CLIENT        = 20013,
            TS_GA_CLIENT_KICK_FAILED = 20014,
            // Between Auth & Client
            TS_AC_RESULT             = 10000,
            TS_CA_VERSION            = 10001,
            TS_CA_ACCOUNT            = 10010,
            TS_CA_SERVER_LIST        = 10021,
            TS_AC_SERVER_LIST        = 10022,
            TS_CA_SELECT_SERVER      = 10023,
            TS_AC_SELECT_SERVER      = 10024
    };
}

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push, N), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif
typedef struct TS_CS_ACCOUNT_WITH_AUTH
{
#if EPIC == 4
    char account[19];
#else
    char account[61];
#endif
    unsigned long long one_time_key;
} s_ClientWithAuth_CS;

typedef struct TS_CS_LOGIN
{
    char szName[19];
    char race;
} s_ClientLogin_CS;

typedef struct TS_CS_CHATREQUET
{
    char        szTarget[21];
    uint8_t     request_id;
    uint8_t     len;
    uint8_t     type;
    std::string szMsg;
} CS_CHATREQUEST;


// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif