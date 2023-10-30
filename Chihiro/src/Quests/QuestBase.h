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
#include "Common.h"

constexpr int32_t MAX_VALUE_NUMBER = 12;
constexpr int32_t MAX_OPTIONAL_REWARD = 3;
constexpr int32_t MAX_FOREQUEST = 3;
constexpr int32_t MAX_KEY_VALUE = 3;
constexpr int32_t MAX_QUEST_STATUS = 3;
constexpr int32_t MAX_RANDOM_QUEST_VALUE = 3;
constexpr int32_t QUEST_PARAMETER_CHAOS = 99;
constexpr int32_t FAVOR_GROUP_ID_CONTACT = 999;

enum class QuestType : int {
    QUEST_MISC = 100,
    QUEST_KILL_TOTAL = 101,
    QUEST_KILL_INDIVIDUAL = 102,
    QUEST_COLLECT = 103,
    QUEST_HUNT_ITEM = 106,
    QUEST_HUNT_ITEM_FROM_ANY_MONSTERS = 107,
    QUEST_LEARN_SKILL = 201,
    QUEST_UPGRADE_ITEM = 301,
    QUEST_CONTACT = 401,
    QUEST_JOB_LEVEL = 501,
    QUEST_PARAMETER = 601,
    QUEST_END_VIA_SCRIPT = 701,
    QUEST_RANDOM_KILL_INDIVIDUAL = 901,
    QUEST_RANDOM_COLLECT = 902,
};

enum class QuestProgress : int { QUEST_IS_STARTABLE = 0, QUEST_IS_IN_PROGRESS = 1, QUEST_IS_FINISHABLE = 255 };

struct Reward {
    int32_t nItemCode;
    int32_t nLevel;
    int32_t nQuantity;
};

struct QuestBase {
    int32_t nCode;
    int32_t nQuestTextID;
    int32_t nSummaryTextID;
    int32_t nStatusTextID;
    int32_t nLimitLevel;
    int32_t nLimitJobLevel;
    int32_t nLimitMaxLevel;
    uint8_t nLimitIndication;
    uint32_t LimitFlag;
    int32_t nLimitJob;
    int32_t nLimitFavor;
    bool bIsRepeatable;
    int32_t nInvokeCondition;
    int32_t nInvokeValue;
    int32_t nTimeLimit;
    QuestType nType;
    int32_t nValue[MAX_VALUE_NUMBER];
    int32_t nDropGroupID;
    int32_t nQuestDifficulty;
    int32_t nFavor;
    int64_t nEXP;
    int32_t nJP;
    int64_t nGold;
    Reward DefaultReward;
    Reward OptionalReward[MAX_OPTIONAL_REWARD];
    int32_t nForeQuest[MAX_FOREQUEST];
    bool bForceCheckType;
    int32_t nIsMagicPointQuest;
    int32_t nEndType;
};

struct QuestBaseServer : public QuestBase {
    int32_t nLimitFavorGroupID;
    int32_t nFavorGroupID;
    int32_t nHateGroupID;
    std::string strAcceptScript;
    std::string strClearScript;
    std::string strScript;
};

struct QuestLink {
    int32_t nNPCID;
    int32_t code;
    bool bLF_Start;
    bool bLF_Progress;
    bool bLF_End;
    int32_t nStartTextID;
    int32_t nInProgressTextID;
    int32_t nEndTextID;
};

class NPC;
struct QuestInstance {
    int32_t nID;
    int32_t Code;
    uint32_t nTime;
    int32_t nStatus[MAX_QUEST_STATUS];
    QuestProgress nProgress;
    int32_t nStartID;
    int32_t nRandomKey[MAX_RANDOM_QUEST_VALUE];
    int32_t nRandomValue[MAX_RANDOM_QUEST_VALUE];
    NPC *start_npc{nullptr};
};

struct RandomQuestInfo {
    int32_t code;
    int32_t key[MAX_RANDOM_QUEST_VALUE];
    int32_t value[MAX_RANDOM_QUEST_VALUE];
    bool is_dropped;
};