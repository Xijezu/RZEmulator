#ifndef PACKETS_TS_CS_CHAT_REQUEST_H
#define PACKETS_TS_CS_CHAT_REQUEST_H

#include "Packet/PacketDeclaration.h"

enum TS_CHAT_TYPE : uint8_t
{
	CHAT_NORMAL = 0x0,
	CHAT_YELL = 0x1,
	CHAT_ADV = 0x2,
	CHAT_WHISPER = 0x3,
	CHAT_GLOBAL = 0x4,
	CHAT_EMOTION = 0x5,
	CHAT_GM = 0x6,
	CHAT_GM_WHISPER = 0x7,
	CHAT_PARTY = 0xA,
	CHAT_GUILD = 0xB,
	CHAT_ATTACKTEAM = 0xC,
	CHAT_FRIEND = 0xD,
	CHAT_NOTICE = 0x14,
	CHAT_ANNOUNCE = 0x15,
	CHAT_CENTER_NOTICE = 0x16,
	CHAT_EXP = 0x1E,
	CHAT_DAMAGE = 0x1F,
	CHAT_ITEM = 0x20,
	CHAT_BATTLE = 0x21,
	CHAT_SUMMON = 0x22,
	CHAT_ETC = 0x23,
	CHAT_TITLE = 0x24,
	CHAT_NPC = 0x28,
	CHAT_DEBUG = 0x32,
	CHAT_PARTY_SYSTEM = 0x64,
	CHAT_GUILD_SYSTEM = 0x6E,
	CHAT_QUEST_SYSTEM = 0x78,
	CHAT_RAID_SYSTEM = 0x82,
	CHAT_FRIEND_SYSTEM = 0x8C,
	CHAT_ALLIANCE_SYSTEM = 0x96,
	CHAT_HUNTAHOLIC_SYSTEM = 0xA0,
	CHAT_DUNGEON_SYSTEM = 0xAA,
};

// No check of packet size and message size
// Message does not need to include null terminator (client does not include it)
#define TS_CS_CHAT_REQUEST_DEF(_) \
	_(string)(szTarget, 21) \
	_(simple)(int8_t, request_id) \
	_(count)(uint8_t, message) \
	_(simple)(TS_CHAT_TYPE, type) \
	_(dynstring)(message, false)

CREATE_PACKET(TS_CS_CHAT_REQUEST, 20);

#endif // PACKETS_TS_CS_CHAT_REQUEST_H
