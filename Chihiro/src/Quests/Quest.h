#ifndef PROJECT_QUEST_H
#define PROJECT_QUEST_H

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


#endif // PROJECT_QUEST_H
