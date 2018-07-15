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

#include "QuestManager.h"
#include "ObjectMgr.h"
#include "GameContent.h"

bool QuestManager::DoEachActiveQuest(const std::function<void(Quest *)> &fn)
{
    for(auto& q : m_vActiveQuest) {
        fn(q);
    }
    return true;
}

void QuestManager::SetMaxQuestID(int id)
{
    m_QuestIndex = id;
}

bool QuestManager::AddQuest(Quest *quest)
{
    int nQuestCode{0};
    if(quest->m_QuestBase == nullptr)
        return false;

    nQuestCode = quest->m_Instance.Code;

    if(quest->m_Instance.Code == 0) {
        NG_LOG_ERROR("quest", "QuestManager::AddQuest: Quest code is zero!");
        return false;
    }
    for(auto& q : m_vActiveQuest) {
        if(quest->m_Instance.Code == q->m_Instance.Code) {
            return false;
        }
    }

    if (quest->m_Instance.nProgress == QuestProgress::QUEST_IS_FINISHABLE)
    {
        if(m_hsFinishedQuest.count(nQuestCode) > 0 && !quest->m_QuestBase->bIsRepeatable) {
            return false;
        }
        m_hsFinishedQuest.emplace(nQuestCode, true);
        delete quest;
        return true;
    }
    if(Quest::IsRandomQuest(quest->m_Instance.Code)) {
//                 v27 = (int)&v3->m_vRandomQuestInfo;
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                 &thisa,
//                 *(unsigned int **)(v27 + 4),
//                 (std::_Container_base *)v27);
//                 v39 = (signed int)thisa.baseclass_0.baseclass_0._Mycont;
//                 v35 = (std::_Container_base *)v27;
//                 v33 = *(unsigned int **)(v27 + 8);
//                 nQuestCode = (unsigned int)thisa._Myptr;
//                 while ( 1 )
//                 {
//                     std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     &thisa,
//                     v33,
//                     v35);
//                     if ( std::_Vector_const_iterator<X2D::Point<float>_std::allocator<X2D::Point<float>>>::operator__(
//                         (std::_Vector_const_iterator<ArObject *,std::allocator<ArObject *> > *)&v39,
//                         (std::_Vector_const_iterator<ArObject *,std::allocator<ArObject *> > *)&thisa) )
//                         break;
//                     if ( LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((std::_Vector_const_iterator<ArMoveVector::MOVE_INFO,std::allocator<ArMoveVector::MOVE_INFO> > *)&v39)->end.x) == v2 )
//                     {
//                         v28 = 0;
//                         do
//                         {
//                             v29 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((std::_Vector_const_iterator<ArMoveVector::MOVE_INFO,std::allocator<ArMoveVector::MOVE_INFO> > *)&v39);
//                             StructQuest::SetRandomKey(pQuest, v28, *((_DWORD *)&v29->end.y + v28));
//                             v30 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((std::_Vector_const_iterator<ArMoveVector::MOVE_INFO,std::allocator<ArMoveVector::MOVE_INFO> > *)&v39);
//                             StructQuest::SetRandomValue(pQuest, v28, *(&v30->end_time + v28));
//                             ++v28;
//                         }
//                         while ( v28 < 3 );
//                         v7 = (std::vector<unsigned int,std::allocator<unsigned int> > *)v38;
//                         break;
//                     }
//                     std::_Vector_const_iterator<LuaVM::LuaScriptInfo_std::allocator<LuaVM::LuaScriptInfo>>::operator__((std::_Vector_const_iterator<StructCreature::DamageReflectInfo,std::allocator<StructCreature::DamageReflectInfo> > *)&v39);
//                     v35 = (std::_Container_base *)v27;
//                     v33 = *(unsigned int **)(v27 + 8);
//                 }
    }
    m_vActiveQuest.emplace_back(quest);
    return true;
}

bool QuestManager::StartQuest(int code, int nStartID)
{
    if(code == 0 || FindQuest(code) != nullptr)
        return false;
    int  qid = allocQuestID();
    int  tmp[MAX_QUEST_STATUS]{0};
    auto q   = Quest::AllocQuest(m_pHandler, qid, code, tmp, QuestProgress::QUEST_IS_IN_PROGRESS, nStartID);
    if(Quest::IsRandomQuest(q->m_Instance.Code)) {
//                 v12 = v4->m_vRandomQuestInfo._Myfirst;
//                 v13 = &v4->m_vRandomQuestInfo;
//                 *(v3 + 11) = 1;
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     (v3 - 32),
//                     v12,
//                     v13);
//                 *(v3 - 40) = *(v3 - 32);
//                 *(v3 - 36) = *(v3 - 28);
//                 while ( 1 )
//                 {
//                     std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                         (v3 - 32),
//                         *(v13 + 8),
//                         v13);
//                     if ( std::_Vector_const_iterator<X2D::Point<float>_std::allocator<X2D::Point<float>>>::operator__(
//                              (v3 - 40),
//                              (v3 - 32)) )
//                     {
//                         StructQuestManager::RandomQuestInfo::RandomQuestInfo((v3 - 140), v5);
//                         std::vector<StructCreature::DamageReflectInfo_std::allocator<StructCreature::DamageReflectInfo>>::push_back(
//                             v13,
//                             v16);
//                         v17 = std::vector<StructQuestManager::RandomQuestInfo_std::allocator<StructQuestManager::RandomQuestInfo>>::back(v13);
//                         goto LABEL_10;
//                     }
//                     v14 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v3 - 40));
//                     v15 = (v3 - 40);
//                     if ( LODWORD(v14->end.x) == v5 )
//                         break;
//                     std::_Vector_const_iterator<LuaVM::LuaScriptInfo_std::allocator<LuaVM::LuaScriptInfo>>::operator__(v15);
//                 }
//                 v17 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(v15);
//                 *(v3 + 11) = *(v17 + 28) == 0;
//         LABEL_10:
//                 if ( *(v3 + 11) )
//                 {
//                     *(v3 - 28) = 0;
//                     *(v3 - 80) = 0;
//                     *(v3 - 76) = 0;
//                     *(v3 - 72) = 0;
//                     *(v3 - 4) = 0;
//                     v18 = (v17 + 4);
//                     v19 = 3;
//                     do
//                     {
//                         *(v18 + 12) = 0;
//                         *v18 = 0;
//                         v18 += 4;
//                         --v19;
//                     }
//                     while ( v19 );
//                     *(v3 - 16) = -14;
//                     *(v3 - 16) -= v17;
//                     *(v3 - 20) = -15;
//                     *(v3 - 20) -= v17;
//                     *(v3 + 8) = -16;
//                     *(v3 + 8) -= v17;
//                     *(v3 - 24) = 0;
//                     v20 = (v17 + 16);
//                     while ( 1 )
//                     {
//                         v21 = StructQuest::GetValue(*(v3 + 12), v20 + *(v3 + 8));
//                         *(v3 - 36) = v21;
//                         if ( v21 == v6 )
//                         {
//         LABEL_20:
//                             *(v3 - 4) = -1;
//                             std::vector<GameContent::REGEN_INFO_std::allocator<GameContent::REGEN_INFO>>::_Tidy((v3 - 84));
//                             goto LABEL_21;
//                         }
//                         if ( v21 != *(v3 - 28) )
//                             GameContent::GetRandomPoolInfo(v21, (v3 - 84), *(*(*(v3 + 12) + 4) + 16));
//                         if ( *(v3 - 80) == v6 )
//                             break;
//                         v22 = (*(v3 - 76) - *(v3 - 80)) >> 3;
//                         if ( !v22 )
//                             break;
//                         *(v3 - 28) = *(v3 - 36);
//                         v23 = XFastRandom() % v22;
//                         v24 = std::vector<XPerformanceGuard::_TAG_std::allocator<XPerformanceGuard::_TAG>>::operator__(
//                                   (v3 - 84),
//                                   v23);
//                         v25 = *(v3 + 12);
//                         *(v20 - 12) = v24->pObj;
//                         v26 = StructQuest::GetValue(v25, v20 + *(v3 - 16));
//                         v27 = StructQuest::GetValue(*(v3 + 12), v20 + *(v3 - 20));
//                         *v20 = XFastRandom(v27, v26);
//                         std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                             (v3 - 52),
//                             *(v3 - 80),
//                             (v3 - 84));
//                         v28 = std::_Vector_iterator<std::pair<int_int>_std::allocator<std::pair<int_int>>>::operator_(
//                                   (v3 - 52),
//                                   (v3 - 60),
//                                   v23);
//                         std::vector<GameContent::RANDOM_POOL_INFO_std::allocator<GameContent::RANDOM_POOL_INFO>>::erase(
//                             (v3 - 84),
//                             (v3 - 68),
//                             *v28);
//                         ++*(v3 - 24);
//                         v20 += 4;
//                         v6 = 0;
//                         if ( *(v3 - 24) >= 3 )
//                             goto LABEL_20;
//                     }
//                     StructQuest::FreeQuest(*(v3 + 12));
//                     std::vector<GameContent::REGEN_INFO_std::allocator<GameContent::REGEN_INFO>>::_Tidy((v3 - 84));
//                     return 0;
//                 }
//         LABEL_21:
//                 *(v17 + 28) = 0;
//                 v29 = (v17 + 16);
//                 do
//                 {
//                     StructQuest::SetRandomKey(*(v3 + 12), v6, *(v29 - 3));
//                     StructQuest::SetRandomValue(*(v3 + 12), v6++, *v29);
//                     ++v29;
//                 }
//                 while ( v6 < 3 );
//                 v4 = *(v3 - 44);
    }
    m_vActiveQuest.emplace_back(q);
    return true;
}

bool QuestManager::EndQuest(Quest *pQuest)
{
    pQuest->SetProgress(QuestProgress::QUEST_IS_FINISHABLE);
    if(!m_hsFinishedQuest.count(pQuest->m_Instance.Code))
        m_hsFinishedQuest.emplace(pQuest->m_Instance.Code, true);
    return true;
}

Quest *QuestManager::FindQuest(int code)
{
    for(auto& q : m_vActiveQuest) {
        if(q->m_QuestBase->nCode == code)
            return q;
    }
    return nullptr;
}

bool QuestManager::IsFinishedQuest(int code)
{
    if(m_hsFinishedQuest.count(code) == 1)
        return m_hsFinishedQuest[code];
    return false;
}

bool QuestManager::IsTakeableQuestItem(int code)
{
    for(auto& q : m_vActiveQuest) {
        if (q->m_Instance.nProgress == QuestProgress::QUEST_IS_FINISHABLE
            || (q->m_QuestBase->nType != QuestType::QUEST_COLLECT
               && q->m_QuestBase->nType != QuestType::QUEST_HUNT_ITEM
               && q->m_QuestBase->nType != QuestType::QUEST_HUNT_ITEM_FROM_ANY_MONSTERS
               && q->m_QuestBase->nType != QuestType::QUEST_RANDOM_COLLECT)
            || q->IsFinishable())
            continue;

        if(Quest::IsRandomQuest(q->m_QuestBase->nCode)) {
            if (q->GetRandomKey(0) == code
                || q->GetRandomKey(1) == code
                || q->GetRandomKey(2) == code)
                return true;
        }
        else if (q->m_QuestBase->nType == QuestType::QUEST_COLLECT)
        {
            if (q->GetValue(0) == code
                || q->GetValue(2) == code
                || q->GetValue(4) == code
                || q->GetValue(6) == code
                || q->GetValue(8) == code
                || q->GetValue(10) == code)
                return true;
        } else {
            if (q->GetValue(0) == code
                || q->GetValue(2) == code
                || q->GetValue(4) == code)
                return true;
        }
    }
    return false;
}

void QuestManager::GetRelatedQuest(std::vector<Quest*> &vQuestList, int flag)
{
    for(auto& q : m_vActiveQuest) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE)
        {
            if(sObjectMgr.checkQuestTypeFlag(q->m_QuestBase->nType, flag)) {
                vQuestList.emplace_back(q);
            }
        }
    }
}

void QuestManager::GetRelatedQuestByItem(int code, std::vector<Quest*> &vQuest, int flag)
{
    for(auto& q : m_vActiveQuest) {
        if (q->m_Instance.nProgress == QuestProgress::QUEST_IS_FINISHABLE || !sObjectMgr.checkQuestTypeFlag(q->m_QuestBase->nType, flag))
            continue;

        switch(q->m_QuestBase->nType) {
            case QuestType::QUEST_HUNT_ITEM:
            case QuestType::QUEST_HUNT_ITEM_FROM_ANY_MONSTERS:
                if (q->GetValue(0) == code
                    || q->GetValue(2) == code
                    || q->GetValue(4) == code)
                    vQuest.emplace_back(q);
                break;

            case QuestType::QUEST_COLLECT:
                if (q->GetValue(0) == code
                    || q->GetValue(2) == code
                    || q->GetValue(4) == code
                    || q->GetValue(6) == code
                    || q->GetValue(8) == code
                    || q->GetValue(10) == code)
                    vQuest.emplace_back(q);
                break;

            case QuestType::QUEST_RANDOM_COLLECT:
                if (q->GetRandomKey(0) == code
                    || q->GetRandomKey(1) == code
                    || q->GetRandomKey(2) == code)
                    vQuest.emplace_back(q);
                break;

            default:
                break;
        }
    }
}

void QuestManager::GetRelatedQuestByMonster(int nMonsterID, std::vector<Quest*> &vQuest, int flag)
{
    for (auto &q :m_vActiveQuest) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE && sObjectMgr.checkQuestTypeFlag(q->m_QuestBase->nType, flag))
        {
            if (q->m_QuestBase->nType == QuestType::QUEST_KILL_TOTAL || q->m_QuestBase->nType == QuestType::QUEST_KILL_INDIVIDUAL)
            {
                if (GameContent::IsInRandomPoolMonster(q->GetValue(0), nMonsterID)
                    || GameContent::IsInRandomPoolMonster(q->GetValue(2), nMonsterID)
                    || GameContent::IsInRandomPoolMonster(q->GetValue(4), nMonsterID))
                    vQuest.emplace_back(q);
            } else {
                if (q->m_QuestBase->nType != QuestType::QUEST_HUNT_ITEM)
                {
                    if (q->m_QuestBase->nType == QuestType::QUEST_HUNT_ITEM_FROM_ANY_MONSTERS)
                        vQuest.emplace_back(q);
                } else {
                    if (GameContent::IsInRandomPoolMonster(q->GetValue(6), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(7), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(8), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(9), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(10), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(11), nMonsterID))
                        vQuest.emplace_back(q);
                }
            }
        }
    }
}

void QuestManager::UpdateQuestStatusByItemCount(int code, int64 count)
{
    std::vector<Quest *> vQuestList{ };
    int                  i{0};

    GetRelatedQuestByItem(code, vQuestList, 6168);

    for (auto &q : vQuestList)
    {
        if (Quest::IsRandomQuest(q->m_QuestBase->nCode))
        {
            for (i = 0; i < MAX_RANDOM_QUEST_VALUE; ++i)
            {
                if (q->GetRandomKey(6) == code)
                {
                    auto qv = q->GetRandomValue(i);
                    if (count > qv)
                        count = (int64)qv;
                    q->UpdateStatus(i, (int)count);
                    return;
                }
            }
        } else
        {
            for (i = 0; i < MAX_RANDOM_QUEST_VALUE; ++i)
            {
                if (q->GetValue(2 * i) == code)
                {
                    auto qv = q->GetValue((2 * i) + 1);
                    if (count > qv)
                        count = (int64)qv;
                    q->UpdateStatus(i, (int)count);
                    return;
                }
            }
        }
    }
}

void QuestManager::UpdateQuestStatusByMonsterKill(int nMonsterID)
{
    std::vector<Quest*> vQuestList{};
    GetRelatedQuest(vQuestList, 1030);

    for(auto& q : vQuestList) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE && !q->IsFinishable())
        {
            switch(q->m_QuestBase->nType) {
                case QuestType::QUEST_KILL_TOTAL:
                    if (GameContent::IsInRandomPoolMonster(q->GetValue(0), nMonsterID)
                        || GameContent::IsInRandomPoolMonster(q->GetValue(2), nMonsterID)
                        || (GameContent::IsInRandomPoolMonster(q->GetValue(4), nMonsterID)
                           && q->GetStatus(0) < q->GetValue(1))) {
                        q->IncStatus(0, 1);
                    }
                    break;

                case QuestType::QUEST_KILL_INDIVIDUAL:
                    if(GameContent::IsInRandomPoolMonster(q->GetValue(0), nMonsterID) && q->GetStatus(0) < q->GetValue(1))
                        q->IncStatus(0, 1);
                    else if(GameContent::IsInRandomPoolMonster(q->GetValue(2), nMonsterID) && q->GetStatus(1) < q->GetValue(3))
                        q->IncStatus(1, 1);
                    else if(GameContent::IsInRandomPoolMonster(q->GetValue(4), nMonsterID) && q->GetStatus(2) < q->GetValue(5))
                        q->IncStatus(2, 1);
                    break;

                case QuestType::QUEST_RANDOM_KILL_INDIVIDUAL:
                    if(q->GetRandomKey(0) == nMonsterID && q->GetStatus(0) < q->GetRandomValue(0))
                        q->IncStatus(0, 1);
                    else if(q->GetRandomKey(1) == nMonsterID && q->GetStatus(1) < q->GetRandomValue(1))
                        q->IncStatus(0, 1);
                    else if(q->GetRandomKey(2) == nMonsterID && q->GetStatus(1) < q->GetRandomValue(2))
                        q->IncStatus(0, 1);
                    break;
                default:
                    break;
            }
        }
    }
}

void QuestManager::UpdateQuestStatusBySkillLevel(int nSkillID, int nSkillLevel)
{
    int v{0};
    for(auto& q : m_vActiveQuest) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE && q->m_QuestBase->nType == QuestType::QUEST_LEARN_SKILL)
        {
            if(q->GetValue(0) == nSkillID) {
                v = q->GetValue(1);
                if(nSkillID > v)
                    nSkillID = v;
                q->UpdateStatus(0, nSkillLevel);
            } else if (q->GetValue(2) == nSkillID) {
                v = q->GetValue(3);
                if(nSkillID > v)
                    nSkillID = v;
                q->UpdateStatus(1, nSkillLevel);
            } else if(q->GetValue(4) == nSkillID) {
                v = q->GetValue(5);
                if(nSkillID > v)
                    nSkillID = v;
                q->UpdateStatus(2, nSkillLevel);
            }
        }
    }
}

void QuestManager::UpdateQuestStatusByJobLevel(int nJobDepth, int nJobLevel)
{
    int v{0};
    std::vector<Quest*> vQuestList{ };
    GetRelatedQuest(vQuestList, 256);

    for(auto& q : vQuestList) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE && q->m_QuestBase->nType == QuestType::QUEST_JOB_LEVEL)
        {
            v = q->GetValue(0);
            if(nJobDepth > v)
                nJobDepth = v;
            q->UpdateStatus(0, nJobDepth);

            v = q->GetValue(1);
            if(nJobLevel > v)
                nJobLevel = v;
            q->UpdateStatus(1, nJobLevel);
        }
    }
}

void QuestManager::UpdateQuestStatusByParameter(int parameter_id, int value)
{
    int v{0};
    std::vector<Quest*> vQuestList{};
    GetRelatedQuest(vQuestList, 512);

    for(auto& q : vQuestList) {
        if (q->m_Instance.nProgress != QuestProgress::QUEST_IS_FINISHABLE && q->m_QuestBase->nType == QuestType::QUEST_PARAMETER)
        {
            if(q->GetValue(0) == parameter_id)
                q->UpdateStatus(0, value);
            if(q->GetValue(3) == parameter_id)
                q->UpdateStatus(1, value);
            if(q->GetValue(6) == parameter_id)
                q->UpdateStatus(2, value);
        }
    }
}

void QuestManager::PopFromActiveQuest(Quest *pQuest)
{
    if(std::find(m_vActiveQuest.begin(), m_vActiveQuest.end(), pQuest) != m_vActiveQuest.end()) {
        m_vActiveQuest.erase(std::remove(m_vActiveQuest.begin(), m_vActiveQuest.end(), pQuest), m_vActiveQuest.end());
        pQuest->FreeQuest();
    }
}

bool QuestManager::IsStartableQuest(int code)
{
    bool res{false};
    if (FindQuest(code) != nullptr)
        return false;
    auto qbs = sObjectMgr.GetQuestBase(code);
    if (qbs == nullptr)
        return false;
    if (!qbs->bIsRepeatable && IsFinishedQuest(code))
        return false;

/*    if (qbs->bForceCheckType)
        res = false;*/
    if(qbs->nForeQuest[0] == 0) {
        return true;
    }
    else
    {
        for (int &id : qbs->nForeQuest)
        {
            if (id != 0 && IsFinishedQuest(id))
            {
                return true;
            }
        }
        return false;
    }
}

void QuestManager::SetDropFlagToRandomQuestInfo(int code)
{
    for (auto &rqi : m_vRandomQuestInfo) {
        if (rqi.code == code) {
            rqi.is_dropped = true;
            return;
        }
    }
}

bool QuestManager::HasRandomQuestInfo(int code)
{
    return false;
    //             char *v2; // esi@1
//             unsigned int *v4; // [sp-8h] [bp-1Ch]@1
//             std::_Container_base *v5; // [sp-4h] [bp-18h]@1
//             std::_Vector_const_iterator<unsigned int,std::allocator<unsigned int> > thisa; // [sp+4h] [bp-10h]@1
//             std::_Vector_const_iterator<StructQuestManager::RandomQuestInfo,std::allocator<StructQuestManager::RandomQuestInfo> > it; // [sp+Ch] [bp-8h]@1
//
//             v2 = &this->m_vRandomQuestInfo;
//             std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                 &thisa,
//                 this->m_vRandomQuestInfo._Myfirst,
//                 &this->m_vRandomQuestInfo.baseclass_0.___u0.baseclass_0);
//             it.baseclass_0.baseclass_0._Mycont = thisa.baseclass_0.baseclass_0._Mycont;
//             v5 = v2;
//             v4 = *(v2 + 2);
//             it._Myptr = thisa._Myptr;
//             while ( 1 )
//             {
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     &thisa,
//                     v4,
//                     v5);
//                 if ( std::_Vector_const_iterator<X2D::Point<float>_std::allocator<X2D::Point<float>>>::operator__(&it, &thisa) )
//                     return 0;
//                 if ( LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&it)->end.x) == code )
//                     break;
//                 std::_Vector_const_iterator<LuaVM::LuaScriptInfo_std::allocator<LuaVM::LuaScriptInfo>>::operator__(&it);
//                 v5 = v2;
//                 v4 = *(v2 + 2);
//             }
//             return 1;
}

int QuestManager::allocQuestID()
{
    return ++m_QuestIndex;
}
