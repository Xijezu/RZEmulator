#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "Common.h"

constexpr int MAX_VALUE_NUMBER       = 12;
constexpr int MAX_OPTIONAL_REWARD    = 3;
constexpr int MAX_FOREQUEST          = 3;
constexpr int MAX_KEY_VALUE          = 3;
constexpr int MAX_QUEST_STATUS       = 3;
constexpr int MAX_RANDOM_QUEST_VALUE = 3;
constexpr int QUEST_PARAMETER_CHAOS  = 99;
constexpr int FAVOR_GROUP_ID_CONTACT = 999;

enum class QuestType : int
{
        QUEST_MISC                        = 100,
        QUEST_KILL_TOTAL                  = 101,
        QUEST_KILL_INDIVIDUAL             = 102,
        QUEST_COLLECT                     = 103,
        QUEST_HUNT_ITEM                   = 106,
        QUEST_HUNT_ITEM_FROM_ANY_MONSTERS = 107,
        QUEST_LEARN_SKILL                 = 201,
        QUEST_UPGRADE_ITEM                = 301,
        QUEST_CONTACT                     = 401,
        QUEST_JOB_LEVEL                   = 501,
        QUEST_PARAMETER                   = 601,
        QUEST_END_VIA_SCRIPT              = 701,
        QUEST_RANDOM_KILL_INDIVIDUAL      = 901,
        QUEST_RANDOM_COLLECT              = 902,
};

enum class QuestProgress : int
{
        QUEST_IS_STARTABLE   = 0,
        QUEST_IS_IN_PROGRESS = 1,
        QUEST_IS_FINISHABLE  = 255
};

struct Reward
{
    int nItemCode;
    int nLevel;
    int nQuantity;
};

struct QuestBase
{
    int       nCode;
    int       nQuestTextID;
    int       nSummaryTextID;
    int       nStatusTextID;
    int       nLimitLevel;
    int       nLimitJobLevel;
    int       nLimitMaxLevel;
    uint8_t     nLimitIndication;
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
    int64_t     nEXP;
    int       nJP;
    int64_t     nGold;
    Reward    DefaultReward;
    Reward    OptionalReward[MAX_OPTIONAL_REWARD];
    int       nForeQuest[MAX_FOREQUEST];
    bool      bForceCheckType;
    int       nIsMagicPointQuest;
    int       nEndType;
};

struct QuestBaseServer : public QuestBase
{
    int         nLimitFavorGroupID;
    int         nFavorGroupID;
    int         nHateGroupID;
    std::string strAcceptScript;
    std::string strClearScript;
    std::string strScript;
};

struct QuestLink
{
    int  nNPCID;
    int  code;
    bool bLF_Start;
    bool bLF_Progress;
    bool bLF_End;
    int  nStartTextID;
    int  nInProgressTextID;
    int  nEndTextID;
};

class NPC;
struct QuestInstance
{
    int           nID;
    int           Code;
    uint          nTime;
    int           nStatus[MAX_QUEST_STATUS];
    QuestProgress nProgress;
    int           nStartID;
    int           nRandomKey[MAX_RANDOM_QUEST_VALUE];
    int           nRandomValue[MAX_RANDOM_QUEST_VALUE];
    NPC           *start_npc{nullptr};
};

struct RandomQuestInfo
{
    int  code;
    int  key[MAX_RANDOM_QUEST_VALUE];
    int  value[MAX_RANDOM_QUEST_VALUE];
    bool is_dropped;
};