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

#include "Quest.h"
#include "QuestBase.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "Player.h"

Quest *Quest::AllocQuest(QuestEventHandler *handler, int nID, int code, const int *status, QuestProgress progress, int nStartID)
{
    auto result = new Quest{ };
    if (result != nullptr)
    {
        result->m_Handler           = handler;
        result->m_bIsNeedUpdateToDB = false;
        result->m_QuestBase         = sObjectMgr.GetQuestBase(code);
        if (result->m_QuestBase == nullptr)
        {
            NG_LOG_ERROR("quest", "Quest::AllocQuest: Invalid Quest Code: %u", code);
            delete result;
            return nullptr;
        }
        result->m_Instance.nID      = nID;
        result->m_Instance.nStartID = nStartID;
        result->m_Instance.Code     = code;
        for (int i = 0; i < MAX_QUEST_STATUS; ++i)
        {
            result->m_Instance.nStatus[i] = status[i];
        }
        result->m_Instance.nProgress = progress;
    }
    return result;
}

bool Quest::IsRandomQuest(int code)
{
    auto      base = sObjectMgr.GetQuestBase(code);
    if (base == nullptr)
    {
        NG_LOG_ERROR("quest", "Quest::IsRandomQuest: Invalid Quest Code: %u", code);
        return false;
    }
    QuestType qt   = base->nType;
    return qt == QuestType::QUEST_RANDOM_COLLECT || qt == QuestType::QUEST_RANDOM_KILL_INDIVIDUAL;
}

void Quest::FreeQuest()
{

}

int Quest::GetQuestCode() const
{
    return m_QuestBase->nCode;
}

QuestInstance *Quest::GetQuestInstance()
{
    return &m_Instance;
}

int Quest::GetQuestID() const
{
    return m_Instance.nID;
}

QuestType Quest::GetQuestType() const
{
    return m_QuestBase->nType;
}

void Quest::SetProgress(QuestProgress progress)
{
    QuestProgress old = m_Instance.nProgress;
    m_Instance.nProgress = progress;
    if (m_Handler != nullptr)
        m_Handler->onProgressChanged(this, old, progress);
}

int Quest::GetValue(int idx) const
{
    int result{0};
    if (idx > MAX_VALUE_NUMBER - 1)
    {
        NG_LOG_ERROR("quest", "Quest::GetValue - Invald Index %u", idx);
        result = 0;
    }
    else
    {
        result = m_QuestBase->nValue[idx];
    }
    return result;
}

int Quest::GetStatus(int idx) const
{
    int result{0};
    if (idx > 5)
    {
        NG_LOG_ERROR("quest", "Quest::GetStatus - Invald Index %u", idx);
        result = 0;
    }
    else
    {
        result = m_Instance.nStatus[idx];
    }
    return result;
}

void Quest::UpdateStatus(int idx, int value)
{
    if (idx > 5)
    {
        NG_LOG_ERROR("quest", "Quest::UpdateStatus - Invald Index %u", idx);
    }
    else
    {
        int old = m_Instance.nStatus[idx];
        m_Instance.nStatus[idx] = value;
        if (m_Handler != nullptr)
            m_Handler->onStatusChanged(this, old, value);
        m_bIsNeedUpdateToDB = true;
    }
}

void Quest::IncStatus(int idx, int value)
{
    if (idx > 5)
    {
        NG_LOG_ERROR("quest", "Quest::IncStatus - Invald Index %u", idx);
    }
    else
    {
        int old = m_Instance.nStatus[idx];
        m_Instance.nStatus[idx] += value;
        if (m_Handler != nullptr)
            m_Handler->onStatusChanged(this, old, old + value);
        m_bIsNeedUpdateToDB = true;
    }
}

int Quest::GetRandomKey(int idx) const
{
    return m_Instance.nRandomKey[idx];
}

int Quest::GetRandomValue(int idx) const
{
    return m_Instance.nRandomValue[idx];
}

bool Quest::IsFinishable() const
{
    switch (m_QuestBase->nType)
    {
        case QuestType::QUEST_MISC:
            break;

        case QuestType::QUEST_KILL_TOTAL:
            if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1])
                return true;
            return false;

        case QuestType::QUEST_KILL_INDIVIDUAL:
            if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1]
                && m_Instance.nStatus[1] >= m_QuestBase->nValue[3]
                && m_Instance.nStatus[2] >= m_QuestBase->nValue[5])
            {
                return true;
            }
            return false;

        case QuestType::QUEST_COLLECT:
            if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1]
                && m_Instance.nStatus[1] >= m_QuestBase->nValue[3]
                && m_Instance.nStatus[2] >= m_QuestBase->nValue[5])
                /*&& m_Instance.nStatus[3] >= m_QuestBase->nValue[7]
                && m_Instance.nStatus[4] >= m_QuestBase->nValue[9])
                 * For later usage
                */
                return true;
            return false;

        case QuestType::QUEST_HUNT_ITEM:
        case QuestType::QUEST_HUNT_ITEM_FROM_ANY_MONSTERS:
            if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1]
                && m_Instance.nStatus[1] >= m_QuestBase->nValue[3]
                && m_Instance.nStatus[2] >= m_QuestBase->nValue[5])
                return true;
            return false;

        case QuestType::QUEST_LEARN_SKILL:
            if ((m_QuestBase->nValue[0] == 0 || m_Instance.nStatus[0] >= m_QuestBase->nValue[1])
                && (m_QuestBase->nValue[2] == 0 || m_Instance.nStatus[1] >= m_QuestBase->nValue[3])
                && (m_QuestBase->nValue[4] == 0 || m_Instance.nStatus[2] >= m_QuestBase->nValue[5]))
                return true;
            return false;

        case QuestType::QUEST_UPGRADE_ITEM:
            if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1]
                && m_Instance.nStatus[1] >= m_QuestBase->nValue[3]
                && m_Instance.nStatus[2] >= m_QuestBase->nValue[5])
                return true;
            return false;

        case QuestType::QUEST_CONTACT:
            return true;

        case QuestType::QUEST_JOB_LEVEL:
            if (m_Instance.nStatus[0] < m_QuestBase->nValue[0])
                return false;
            return m_Instance.nStatus[1] >= m_QuestBase->nValue[1];

        case QuestType::QUEST_PARAMETER:
            if (m_QuestBase->nValue[0] == 0
                || (m_QuestBase->nValue[1] == 2 && m_Instance.nStatus[0] > m_QuestBase->nValue[2])
                || (m_QuestBase->nValue[1] == 1 && m_Instance.nStatus[0] >= m_QuestBase->nValue[2])
                || (m_QuestBase->nValue[1] == 0 && m_Instance.nStatus[0] == m_QuestBase->nValue[2])
                || (m_QuestBase->nValue[1] == -1 && m_Instance.nStatus[0] <= m_QuestBase->nValue[2])
                || (m_QuestBase->nValue[1] == -1 && m_Instance.nStatus[0] < m_QuestBase->nValue[2]))
            {
                if (m_QuestBase->nValue[3] == 0
                    || (m_QuestBase->nValue[4] == 2 && m_Instance.nStatus[1] > m_QuestBase->nValue[5])
                    || (m_QuestBase->nValue[4] == 1 && m_Instance.nStatus[1] >= m_QuestBase->nValue[5])
                    || (m_QuestBase->nValue[4] == 0 && m_Instance.nStatus[1] == m_QuestBase->nValue[5])
                    || (m_QuestBase->nValue[4] == -1 && m_Instance.nStatus[1] <= m_QuestBase->nValue[5])
                    || (m_QuestBase->nValue[4] == -1 && m_Instance.nStatus[1] < m_QuestBase->nValue[5]))
                {
                    if (m_QuestBase->nValue[6] == 0
                        || (m_QuestBase->nValue[7] == 2 && m_Instance.nStatus[2] > m_QuestBase->nValue[8])
                        || (m_QuestBase->nValue[7] == 1 && m_Instance.nStatus[2] >= m_QuestBase->nValue[8])
                        || (m_QuestBase->nValue[7] == 0 && m_Instance.nStatus[2] == m_QuestBase->nValue[8])
                        || (m_QuestBase->nValue[7] == -1 && m_Instance.nStatus[2] <= m_QuestBase->nValue[8])
                        || (m_QuestBase->nValue[7] == -1 && m_Instance.nStatus[2] < m_QuestBase->nValue[8]))
                    {
                        return true;
                    }
                }
            }
            return false;

        case QuestType::QUEST_END_VIA_SCRIPT:
            if (m_QuestBase->nEndType == 3)
            {
                if (m_Instance.nStatus[0] >= m_QuestBase->nValue[1]
                    && m_Instance.nStatus[1] >= m_QuestBase->nValue[3]
                    && m_Instance.nStatus[2] >= m_QuestBase->nValue[5])
                    return true;
            }
            return m_QuestBase->nEndType < 2;

        case QuestType::QUEST_RANDOM_KILL_INDIVIDUAL:
        case QuestType::QUEST_RANDOM_COLLECT:
            break;
    }
    return false;
}

void Quest::DB_Insert(Player *pPlayer, Quest *pQuest)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_QUEST);
    stmt->setInt32(0, pPlayer->GetUInt32Value(UNIT_FIELD_UID));
    stmt->setInt32(1, pQuest->m_Instance.nID);
    stmt->setInt32(2, pQuest->m_Instance.Code);
    stmt->setInt32(3, pQuest->m_Instance.nStartID);
    stmt->setInt32(4, pQuest->m_Instance.nStatus[0]);
    stmt->setInt32(5, pQuest->m_Instance.nStatus[1]);
    stmt->setInt32(6, pQuest->m_Instance.nStatus[2]);
    stmt->setInt32(7, (int)pQuest->m_Instance.nProgress);
    CharacterDatabase.Execute(stmt);
}