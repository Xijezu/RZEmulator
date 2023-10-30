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
#include "QuestBase.h"

class Quest;
class Player;
struct QuestEventHandler {
    virtual void onStatusChanged(Quest *quest, int32_t nOldStatus, int32_t nNewStatus) = 0;
    virtual void onProgressChanged(Quest *quest, QuestProgress oldProgress, QuestProgress newProgress) = 0;
};

class Quest {
public:
    static Quest *AllocQuest(QuestEventHandler *handler, int32_t nID, int32_t code, const int32_t status[], QuestProgress progress, int32_t nStartID);
    static bool IsRandomQuest(int32_t code);
    static void DB_Insert(Player *pPlayer, Quest *pQuest);

    Quest() = default;
    ~Quest() = default;

    // Deleting the copy & assignment operators
    // Better safe than sorry
    Quest(const Quest &) = delete;
    Quest &operator=(const Quest &) = delete;

    void FreeQuest();
    int32_t GetQuestCode() const;
    QuestInstance *GetQuestInstance();
    int32_t GetQuestID() const;
    QuestType GetQuestType() const;

    void SetProgress(QuestProgress progress);
    int32_t GetValue(int32_t idx) const;
    int32_t GetStatus(int32_t idx) const;
    void UpdateStatus(int32_t idx, int32_t value);
    void IncStatus(int32_t idx, int32_t value);
    int32_t GetRandomKey(int32_t idx) const;
    int32_t GetRandomValue(int32_t idx) const;
    bool IsFinishable() const;

    QuestBaseServer *m_QuestBase{nullptr};
    QuestEventHandler *m_Handler{nullptr};
    QuestInstance m_Instance{};
    bool m_bIsNeedUpdateToDB{false};
};