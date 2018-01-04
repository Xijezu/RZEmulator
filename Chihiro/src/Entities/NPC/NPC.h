#ifndef _NPC_H_
#define _NPC_H_

#include "Unit.h"
#include "NPCBase.h"
#include "Quest.h"

class NPC : public Unit {
public:
    explicit NPC(NPCTemplate* base);
    static void EnterPacket(XPacket& pEnterPct, NPC* pNPC);

    void LinkQuest(QuestLink* quest_link_info);
    NPCStatus GetStatus() const;
    void SetStatus(NPCStatus status);
    int GetNPCID() const;


    NPCTemplate* m_pBase;
private:
    int m_nStatus;
    std::vector<QuestLink*> m_vQuestLink_Start{};
    std::vector<QuestLink*> m_vQuestLink_Progress{};
    std::vector<QuestLink*> m_vQuestLink_End{};
    std::vector<int> m_vQuest{};
};

#endif // _NPC_H_
