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
#include "Quest.h"
#include <functional>
#include <map>

class QuestManager
{
    public:
        friend class Player;
        QuestManager() = default;
        ~QuestManager() = default;
        // Deleting the copy & assignment operators
        // Better safe than sorry
        QuestManager(const QuestManager &) = delete;
        QuestManager &operator=(const QuestManager &) = delete;

        bool DoEachActiveQuest(const std::function<void(Quest *)> &fn);
        void SetMaxQuestID(int id);
        bool AddQuest(Quest *quest);

        void AddRandomQuestInfo(int code, int key[], int value[], bool is_dropped) {};
        bool StartQuest(int code, int nStartID);
        bool EndQuest(Quest *pQuest);
        Quest *FindQuest(int code);
        bool IsFinishedQuest(int code);
        bool IsTakeableQuestItem(int code);
        void GetRelatedQuest(std::vector<Quest *> &vQuestList, int flag);
        void GetRelatedQuestByItem(int code, std::vector<Quest *> &vQuest, int flag);
        void GetRelatedQuestByMonster(int nMonsterID, std::vector<Quest *> &vQuest, int flag);
        void UpdateQuestStatusByItemCount(int code, int64 count);
        void UpdateQuestStatusByMonsterKill(int nMonsterID);
        void UpdateQuestStatusBySkillLevel(int nSkillID, int nSkillLevel);
        void UpdateQuestStatusByJobLevel(int nJobDepth, int nJobLevel);
        void UpdateQuestStatusByParameter(int parameter_id, int value);
        void PopFromActiveQuest(Quest *pQuest);
        bool IsStartableQuest(int code);
        void SetDropFlagToRandomQuestInfo(int code);
        bool HasRandomQuestInfo(int code);

        QuestEventHandler *m_pHandler{nullptr};

    private:
        std::map<int, bool>          m_hsFinishedQuest{ };
        std::vector<Quest *>         m_vActiveQuest{ };
        int                          m_QuestIndex{0};
        std::vector<RandomQuestInfo> m_vRandomQuestInfo{ };

    protected:
        int allocQuestID();
};