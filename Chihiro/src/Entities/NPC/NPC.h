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
#include "Unit.h"
#include "NPCBase.h"
#include "Quest.h"
#include <functional>

class NPC : public Unit
{
    public:
        explicit NPC(NPCTemplate *base);
        // Deleting the copy & assignment operators
        // Better safe than sorry
        NPC(const NPC &) = delete;
        NPC &operator=(const NPC &) = delete;

        static void EnterPacket(XPacket &pEnterPct, NPC *pNPC, Player *pPlayer);

        void LinkQuest(QuestLink *quest_link_info);
        NPC_STATUS GetStatus() const;
        void SetStatus(NPC_STATUS status);
        int GetNPCID() const;

        bool IsNPC() const override { return true; }

        bool HasStartableQuest(Player *player);
        bool HasFinishableQuest(Player *player);
        bool HasInProgressQuest(Player *player);

        void DoEachStartableQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn);
        void DoEachInProgressQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn);
        void DoEachFinishableQuest(Player *pPlayer, const std::function<void(Player *, QuestLink *)> &fn);

        int GetQuestTextID(int code, int progress) const;
        int GetProgressFromTextID(int code, int textId) const;

        NPCTemplate *m_pBase;
    private:
        int                      m_nStatus;
        std::vector<QuestLink *> m_vQuestLink_Start{ };
        std::vector<QuestLink *> m_vQuestLink_Progress{ };
        std::vector<QuestLink *> m_vQuestLink_End{ };
        std::vector<int>         m_vQuest{ };
};