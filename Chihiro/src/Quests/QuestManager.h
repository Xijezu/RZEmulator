#ifndef PROJECT_QUESTMANAGER_H
#define PROJECT_QUESTMANAGER_H

#include "Common.h"
#include "Quest.h"

class QuestManager {
public:
    QuestManager() = default;
    ~QuestManager() = default;

    bool DoEachActiveQuest(const std::function<void (Quest*)>& fn);
    void SetMaxQuestID(int id);
    bool AddQuest(Quest* quest);
    void AddRandomQuestInfo(int code, int key[], int value[], bool is_dropped) { };
    bool StartQuest(int code, int nStartID);
    bool EndQuest(Quest* pQuest);
    Quest* FindQuest(int code);
    bool IsFinishedQuest(int code);
    bool IsTakeableQuestItem(int code);
    void GetRelatedQuest(std::vector<Quest*>& vQuestList, int flag);
    void GetRelatedQuestByItem(int code, std::vector<Quest*>& vQuest, int flag);
    void GetRelatedQuestByMonster(int nMonsterID, std::vector<Quest*>& vQuest, int flag);
    void UpdateQuestStatusByItemCount(int code, uint64 count);
    void UpdateQuestStatusByMonsterKill(int nMonsterID);
    void UpdateQuestStatusBySkillLevel(int nSkillID, int nSkillLevel);
    void UpdateQuestStatusByJobLevel(int nJobDepth, int nJobLevel);
    void UpdateQuestStatusByParameter(int parameter_id, int value);
    void PopFromActiveQuest(Quest* pQuest);
    bool IsStartableQuest(int code);
    void SetDropFlagToRandomQuestInfo(int code);
    bool HasRandomQuestInfo(int code);

    QuestEventHandler* m_pHandler{nullptr};

private:
    std::map<int, bool> m_hsFinishedQuest{};
    std::vector<Quest*> m_vActiveQuest{};
    int m_QuestIndex{0};
    std::vector<RandomQuestInfo> m_vRandomQuestInfo{};

protected:
    int allocQuestID();
};


#endif // PROJECT_QUESTMANAGER_H
