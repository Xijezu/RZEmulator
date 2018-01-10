#ifndef PROJECT_QUESTBASE_H
#define PROJECT_QUESTBASE_H

#include "Common.h"

#define MAX_VALUE_NUMBER 12
#define MAX_OPTIONAL_REWARD 3
#define MAX_FOREQUEST 3
#define MAX_KEY_VALUE 3
#define MAX_QUEST_STATUS 3
#define MAX_RANDOM_QUEST_VALUE 3
#define QUEST_PARAMETER_CHAOS 99
#define FAVOR_GROUP_ID_CONTACT 999

enum QB_LimitFlags : int {
    QB_LimitDeva     = 1,
    QB_LimitAsura    = 2,
    QB_LimitGaia     = 3,
    QB_LimitFighter  = 4,
    QB_LimitHunter   = 5,
    QB_LimitMagician = 6,
    QB_LimitSummoner = 7
};

enum QuestTypeFlag : int {
    QTF_Misc                   = 0x1,
    QTF_KillTotal              = 0x2,
    QTF_KillIndividual         = 0x4,
    QTF_FlagCollect            = 0x8,
    QTF_HuntItem               = 0x10,
    QTF_LearnSkill             = 0x20,
    QTF_UpgradeItem            = 0x40,
    QTF_Contact                = 0x80,
    QTF_JobLevel               = 0x100,
    QTF_Paramater              = 0x200,
    QTF_RandomKillIndividual   = 0x400,
    QTF_RandomCollect          = 0x800,
    QTF_HuntItemFromAnyMonster = 0x1000,
    QTF_Kill                   = 0x406,
    QTF_All                    = 0xff,
};

enum QuestType : int {
    QT_Misc                    = 100,
    QT_KillTotal               = 101,
    QT_KillIndividual          = 102,
    QT_Collect                 = 103,
    QT_HuntItem                = 106,
    QT_HuntItemFromAnyMonsters = 107,
    QT_LearnSkill              = 201,
    QT_UpgradeItem             = 301,
    QT_Contact                 = 401,
    QT_JobLevel                = 501,
    QT_Parameter               = 601,
    QT_EndViaScript            = 701,
    QT_RandomKillIndividual    = 901,
    QT_RandomCollect           = 902,
};

struct Reward {
    int nItemCode;
    int nLevel;
    int nQuantity;
};

struct QuestBase {
    int       nCode;
    int       nQuestTextID;
    int       nSummaryTextID;
    int       nStatusTextID;
    int       nLimitLevel;
    int       nLimitJobLevel;
    int       nLimitMaxLevel;
    uint8     nLimitIndication;
    uint      LimitFlag;
    int       nLimitJob;
    int       nLimitFavor;
    bool      bIsRepeatable;
    int       nInvokeCondition;
    int       nInvokeValue;
    int       nTimeLimit;
    QuestType nType;
    int       nValue[MAX_VALUE_NUMBER];
    int       nDropGroupID;
    int       nQuestDifficulty;
    int       nFavor;
    uint64    nEXP;
    int       nJP;
    uint64    nGold;
    Reward    DefaultReward;
    Reward    OptionalReward[MAX_OPTIONAL_REWARD];
    int       nForeQuest[MAX_FOREQUEST];
    bool      bForceCheckType;
    int       nIsMagicPointQuest;
    int       nEndType;
};

struct QuestBaseServer : public QuestBase {
    int         nLimitFavorGroupID;
    int         nFavorGroupID;
    int         nHateGroupID;
    std::string strAcceptScript;
    std::string strClearScript;
    std::string strScript;
};

struct QuestLink {
    int  nNPCID;
    int  code;
    bool bLF_Start;
    bool bLF_Progress;
    bool bLF_End;
    int  nStartTextID;
    int  nInProgressTextID;
    int  nEndTextID;
};

enum QuestProgress : int {
    QP_NotStarted = 0,
    QP_InProgress = 1,
    QP_Finished   = 255
};

class NPC;
struct QuestInstance {
    int nID;
    int Code;
    uint nTime;
    int nStatus[MAX_QUEST_STATUS];
    QuestProgress nProgress;
    int nStartID;
    int nRandomKey[MAX_RANDOM_QUEST_VALUE];
    int nRandomValue[MAX_RANDOM_QUEST_VALUE];
    NPC* start_npc{nullptr};
};

struct RandomQuestInfo {
    int code;
    int key[MAX_RANDOM_QUEST_VALUE];
    int value[MAX_RANDOM_QUEST_VALUE];
    bool is_dropped;
};

#endif // PROJECT_QUESTBASE_H
