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

#ifndef NGEMITY_QUEST_H
#define NGEMITY_QUEST_H

#include "Common.h"
#include "QuestBase.h"

class Quest;
class Player;
struct QuestEventHandler {
    virtual void onStatusChanged(Quest* quest, int nOldStatus, int nNewStatus) = 0;
    virtual void onProgressChanged(Quest* quest, QuestProgress oldProgress, QuestProgress newProgress) = 0;
};

class Quest {
public:
    static Quest* AllocQuest(QuestEventHandler* handler, int nID, int code, const int status[], QuestProgress progress, int nStartID);
    static bool IsRandomQuest(int code);
    static void DB_Insert(Player* pPlayer, Quest* pQuest);

    Quest() = default;
    ~Quest() = default;

    void FreeQuest();
    int GetQuestCode() const;
    QuestInstance* GetQuestInstance();
    int GetQuestID() const;
    QuestType GetQuestType() const;

    void SetProgress(QuestProgress progress);
    int GetValue(int idx) const;
    int GetStatus(int idx) const;
    void UpdateStatus(int idx, int value);
    void IncStatus(int idx, int value);
    int GetRandomKey(int idx) const;
    int GetRandomValue(int idx) const;
    bool IsFinishable() const;

    QuestBaseServer *m_QuestBase{nullptr};
    QuestEventHandler *m_Handler{nullptr};
    QuestInstance m_Instance{};
    bool m_bIsNeedUpdateToDB{false};
};


#endif // NGEMITY_QUEST_H
