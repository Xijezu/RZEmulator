#ifndef PROJECT_MESSAGES_H
#define PROJECT_MESSAGES_H

#include "Common.h"
#include "Object.h"

class Unit;
class Player;
class Summon;
class Monster;
class Item;
class XPacket;
class WorldSession;
struct MarketInfo;
class WorldObject;
class State;
class Quest;

struct scramble_map {
    scramble_map()
    {
        int  v3;
        int  i;
        uint8_t v5;

        for (i = 0; i < 32; ++i) {
            map[i] = (uint8_t ) i;
        }

        v3     = 3;
        for (i = 0; i < 32; ++i) {
            v5 = map[i];
            if (v3 >= 32)
                v3 += -32 * (v3 >> 5);
            map[i]  = map[v3];
            map[v3] = v5;
            v3 += i + 3;
        }
    }

    uint8_t map[32]{0};
};

static scramble_map map{};

static int bits_scramble(int c)
{
    int  result;
    uint v2;

    result = 0;
    v2     = 0;
    do {
        if ((((uint) 1 << (int) v2) & c) != 0)
            result |= 1 << map.map[v2];
        ++v2;
    } while (v2 < 32);
    return result;
}


class Messages
{
    public:
        static void GetEncodedInt(XPacket &, uint32);
        static uint GetStatusCode(WorldObject *pObj, Player *pClient);
        static void SendEXPMessage(Player *, Unit *);
        static void SendHPMPMessage(Player *, Unit *, int, float, bool);
        static void SendLevelMessage(Player *, Unit *);
        static void SendStatInfo(Player *, Unit *);
        static void SendAddSummonMessage(Player *, Summon *);
        static void SendCreatureEquipMessage(Player *, bool);
        static void SendPropertyMessage(Player *, Unit *, const std::string &, int64);
        static void SendDialogMessage(Player *, uint32_t, int, const std::string &, const std::string &, const std::string &);
        static void SendSkillList(Player *, Unit *, int);
        static void SendChatMessage(int, const std::string &, Player *, const std::string &);
        static void SendMarketInfo(Player *, uint32_t, const std::vector<MarketInfo> &);
        static void SendItemList(Player *, bool);
        static void SendItemMessage(Player *, Item *);
        static void SendItemCountMessage(Player *, Item *);
        static void SendItemDestroyMessage(Player *, Item *);
        static void SendSkillCastFailMessage(Player *, uint caster, uint target, uint16 skill_id, uint8 skill_level, Position pos, int error_code);
        static void SendGameTime(Player *);
        static void SendResult(Player *, uint16, uint16, uint32);
        static void SendResult(WorldSession *worldSession, uint16, uint16, uint32);
        static void sendEnterMessage(Player *, WorldObject *, bool);
        static void SendMoveMessage(Player *, Unit *);
        static void SendTimeSynch(Player *);
        static void SendWearInfo(Player *, Unit *);
        static void BroadcastHPMPMessage(Unit *, int, float, bool);
        static void BroadcastLevelMsg(Unit *);
        static void BroadcastStatusMessage(WorldObject *obj);
        static void BroadcastStateMessage(Unit *pUnit, State &pState, bool bIsCancel);
        static void BroadcastTamingMessage(Player *pPlayer, Monster *pMonster, int mode);
        static void SendWarpMessage(Player *);
        static void SendCantAttackMessage(Player *, uint, uint, int);
        static void SendQuestInformation(Player *pPlayer, int code, int text, int type);
        static void SendQuestList(Player *pPlayer);
        static void SendGlobalChatMessage(int chatType, const std::string &szSenderName, const std::string &szString, uint len);
        static void SendLocalChatMessage(int nChatType, uint handle, const std::string &szMessage, uint len);
        static void SendQuestMessage(int nChatType, Player *pTarget, const std::string &szString);
        static void SendNPCStatusInVisibleRange(Player *pPlayer);
        static void SendQuestStatus(Player *pPlayer, Quest *pQuest);
        static void SendItemCoolTimeInfo(Player *pPlayer);
        static void SendMixResult(Player *pPlayer, std::vector<uint> *pHandles);
        static void SendItemWearInfoMessage(Player *pPlayer, Unit *pTarget, Item *pItem);
        static void ShowSoulStoneRepairWindow(Player *);
        static void ShowSoulStoneCraftWindow(Player *);
        static void SendPartyInfo(Player *);
        static void SendRegionAckMessage(Player* pPlayer, uint rx, uint ry);
    private:
        static void fillItemInfo(XPacket &, Item *);
};


#endif // PROJECT_MESSAGES_H
