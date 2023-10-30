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
#include <functional>
#include <map>

#include "Common.h"
#include "Quest.h"

class QuestManager {
public:
    friend class Player;
    QuestManager() = default;
    ~QuestManager() = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    QuestManager(const QuestManager &) = delete;
    QuestManager &operator=(const QuestManager &) = delete;

    bool DoEachActiveQuest(const std::function<void(Quest *)> &fn);
    void SetMaxQuestID(int32_t id);
    bool AddQuest(Quest *quest);

    void AddRandomQuestInfo(int32_t code, int32_t key[], int32_t value[], bool is_dropped){};
    bool StartQuest(int32_t code, int32_t nStartID);
    bool EndQuest(Quest *pQuest);
    Quest *FindQuest(int32_t code);
    bool IsFinishedQuest(int32_t code);
    bool IsTakeableQuestItem(int32_t code);
    void GetRelatedQuest(std::vector<Quest *> &vQuestList, int32_t flag);
    void GetRelatedQuestByItem(int32_t code, std::vector<Quest *> &vQuest, int32_t flag);
    void GetRelatedQuestByMonster(int32_t nMonsterID, std::vector<Quest *> &vQuest, int32_t flag);
    void UpdateQuestStatusByItemCount(int32_t code, int64_t count);
    void UpdateQuestStatusByMonsterKill(int32_t nMonsterID);
    void UpdateQuestStatusBySkillLevel(int32_t nSkillID, int32_t nSkillLevel);
    void UpdateQuestStatusByJobLevel(int32_t nJobDepth, int32_t nJobLevel);
    void UpdateQuestStatusByParameter(int32_t parameter_id, int32_t value);
    void PopFromActiveQuest(Quest *pQuest);
    bool IsStartableQuest(int32_t code);
    void SetDropFlagToRandomQuestInfo(int32_t code);
    bool HasRandomQuestInfo(int32_t code);

    QuestEventHandler *m_pHandler{nullptr};

private:
    std::map<int, bool> m_hsFinishedQuest{};
    std::vector<Quest *> m_vActiveQuest{};
    int32_t m_QuestIndex{0};
    std::vector<RandomQuestInfo> m_vRandomQuestInfo{};

protected:
    int32_t allocQuestID();
};