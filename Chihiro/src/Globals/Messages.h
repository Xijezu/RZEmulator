#ifndef PROJECT_MESSAGES_H
#define PROJECT_MESSAGES_H

#include "Common.h"

class Unit;
class Player;
class Summon;
class Item;
class XPacket;
struct MarketInfo;
class WorldObject;

class Messages {
public:
    static void SendEXPMessage(Player *, Unit *);
    static void SendHPMPMessage(Player *, Unit *, int, float, bool);
    static void SendLevelMessage(Player *, Unit *);
    static void SendStatInfo(Player *, Unit *);
    static void SendAddSummonMessage(Player *, Summon *);
    static void SendCreatureEquipMessage(Player *, bool);
    static void SendPropertyMessage(Player *, Unit *, std::string, int64_t);
    static void SendDialogMessage(Player *, uint32_t, int, std::string, std::string, std::string);
    static void SendSkillList(Player *, Unit *, int);
    static void SendChatMessage(int, std::string, Player*, std::string);
    static void SendMarketInfo(Player *, uint32_t, std::vector<MarketInfo>);
    static void SendItemList(Player *, bool);
    static void SendItemMessage(Player *, Item *);
    static void SendGameTime(Player *);
    static void SendResult(Player *, uint16_t, uint16_t, uint16_t);
    static void sendEnterMessage(Player *, WorldObject *, bool);
    static void SendMoveMessage(Player *, Unit *);
    static void SendTimeSynch(Player*);
    static void SendWearInfo(Player*,Unit*);
    static void BroadcastHPMPMessage(Unit*,int,float,bool);
    static void BroadcastLevelMsg(Unit*);
private:
    static void fillItemInfo(XPacket&,Item *);
};


#endif // PROJECT_MESSAGES_H
