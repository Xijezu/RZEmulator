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
#include "Skill.h"
#include "Player.h"
#include "GameRule.h"

struct SkillTargetFunctor
{
    virtual bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) = 0;
};

struct HealingSkillFunctor : public SkillTargetFunctor
{
    HealingSkillFunctor(std::vector<SkillResult> *pSRList, bool bIsByItem = false) : m_pSRList(pSRList), m_bIsByItem(bIsByItem)
    {
    }

    bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) override
    {
        auto slv = pSkill->GetRequestedSkillLevel();
        int32_t nResult = pCaster->GetMagicPoint((ElementalType)pSkill->GetSkillBase()->GetElementalType(), pSkill->GetSkillBase()->IsPhysicalSkill(), pSkill->GetSkillBase()->IsHarmful()) * (pSkill->GetVar(0) + pSkill->GetVar(1) * slv) + pSkill->GetVar(2) + pSkill->GetVar(3) * slv + pSkill->GetVar(6) * pSkill->GetSkillEnhance() + pTarget->GetMaxHealth() * (pSkill->GetVar(4) + pSkill->GetVar(5) * slv + pSkill->GetVar(7) * pSkill->GetSkillEnhance());

        if (pTarget->GetHealth() == 0)
            nResult = 0;
        else if (m_bIsByItem)
            nResult = pTarget->HealByItem(nResult);
        else
            nResult = pTarget->Heal(nResult);

        SkillResult skillResult{};
        skillResult.type = SHT_ADD_HP;
        skillResult.hTarget = pTarget->GetHandle();
        skillResult.hitAddStat.target_stat = pTarget->GetHealth();
        skillResult.hitAddStat.nIncStat = nResult;
        m_pSRList->emplace_back(skillResult);

        return true;
    }

private:
    std::vector<SkillResult> *m_pSRList{nullptr};
    bool m_bIsByItem;
};

struct StateSkillFunctor : public SkillTargetFunctor
{
    StateSkillFunctor(std::vector<SkillResult> *pResultList) : m_vResult(pResultList)
    {
    }

    bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) override
    {
        bool bResult{true};

        if (pSkill->GetSkillBase()->IsHarmful())
        {
            if (pSkill->GetSkillId() == SKILL_FORCE_CHIP || pSkill->GetSkillId() == SKILL_SOUL_CHIP || pSkill->GetSkillId() == SKILL_LUNA_CHIP)
            {
                if (GameRule::GetChipLevelLimit(pSkill->GetRequestedSkillLevel() - 1) < pTarget->GetLevel())
                {
                    int32_t nRatio = 100 - (pTarget->GetLevel() - GameRule::GetChipLevelLimit(pSkill->GetRequestedSkillLevel() - 1)) * 10;
                    if (nRatio < irand(0, 100))
                        bResult = false;
                }
            }
            else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_SELF_COST ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE_BY_SELF_COST ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_TARGET_TYPE ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATES_WITH_EACH_DIFF_LV ||
                     pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION)
            {
                int32_t nAccuracyBonus = pSkill->GetSkillBase()->GetHitBonus(pSkill->GetSkillEnhance(), pCaster->GetLevel() - pTarget->GetLevel());
                if (50 + pCaster->GetMagicAccuracy() - pTarget->GetMagicAvoid() + nAccuracyBonus < irand(0, 100))
                    bResult = false;
            }
            else
            {
                if (pSkill->GetSkillBase()->GetProbabilityOnHit(pSkill->GetRequestedSkillLevel()) < irand(0, 100))
                    bResult = false;
            }
        }

        if (bResult)
        {
            int32_t end_time{0};
            int32_t nEndTime = pSkill->GetSkillBase()->GetStateSecond(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance());

            if (nEndTime / 100 == -1)
                end_time = -1;
            if (nEndTime < 0)
                end_time = t + 6000;
            else
                end_time = t + nEndTime;

            if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE ||
                pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE ||
                pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_SELF_COST ||
                pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE_BY_SELF_COST)
            {
                int32_t count = 5;

                if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_SELF_COST ||
                    pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE_BY_SELF_COST)
                    count = 0;

                int32_t nLevel = pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance());

                for (int32_t i = -1; i < count; ++i)
                {
                    StateCode code{};
                    if (i < 0)
                        code = static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId());
                    else
                        code = static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(i)));

                    if (code == 0)
                        continue;

                    bResult = pTarget->AddState((StateType)pSkill->GetSkillBase()->GetStateType(), code, pCaster->GetHandle(), nLevel, t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                }
            }
            else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_TARGET_TYPE)
            {
                int32_t count = 5;

                bResult = false;
                for (int32_t i = -1; i < count; ++i)
                {
                    if (static_cast<int32_t>(pTarget->GetCreatureGroup()) == pSkill->GetVar(7 + i))
                    {
                        bResult = true;
                        break;
                    }
                }

                if (bResult)
                {
                    int32_t nLevel = pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance());

                    for (int32_t i = -1; i < count; ++i)
                    {
                        StateCode code{};
                        if (i < 0)
                            code = static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId());
                        else
                            code = static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(i)));

                        if (code == 0)
                            continue;

                        bResult = pTarget->AddState((StateType)pSkill->GetSkillBase()->GetStateType(), code, pCaster->GetHandle(), nLevel, t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                    }
                }
            }
            else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATES_WITH_EACH_DIFF_LV)
            {
                int32_t count = 3;

                for (int32_t i = -1; i < count; ++i)
                {
                    StateCode code{};
                    int32_t nLevel = 0;
                    if (i < 0)
                    {
                        code = static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId());
                        nLevel = pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance());
                    }
                    else
                    {
                        code = static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(i)));
                        nLevel = pSkill->GetVar(3 + (i * 3)) + (pSkill->GetVar(3 + (i * 3) + 1) * pSkill->GetRequestedSkillLevel()) + (pSkill->GetVar(3 + (i * 3) + 2) * pSkill->GetSkillEnhance());
                    }

                    if (code == 0 || nLevel == 0)
                        continue;

                    bResult = pTarget->AddState(static_cast<StateType>(pSkill->GetSkillBase()->GetStateType()), code, pCaster->GetHandle(), nLevel, t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                }
            }
            else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION)
            {
                for (int32_t i = 0; i < 2; ++i)
                {
                    StateCode code{};
                    int32_t nLevel = 0;
                    if (i < 0)
                    {
                        code = static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId());
                        nLevel = pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance());
                    }
                    else
                    {
                        code = static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(0)));
                        nLevel = pSkill->GetVar(1) + (pSkill->GetVar(2) * pSkill->GetRequestedSkillLevel()) + (pSkill->GetVar(3) * pSkill->GetSkillEnhance());
                        end_time = t + 100 * (pSkill->GetVar(4) + (pSkill->GetVar(5) * pSkill->GetRequestedSkillLevel()) + (pSkill->GetVar(6) * pSkill->GetSkillEnhance()));
                    }

                    if (code == 0 || nLevel == 0 || static_cast<uint32_t>(end_time) <= t)
                        continue;

                    bResult = pTarget->AddState(static_cast<StateType>(pSkill->GetSkillBase()->GetStateType()), code, pCaster->GetHandle(), nLevel, t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                }
            }
            else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE || pSkill->GetSkillBase()->GetSkillEffectType() == EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE)
            {
                std::vector<int32_t> vStateID{};
                vStateID.reserve(7);
                vStateID.push_back(pSkill->GetSkillBase()->GetStateId());
                for (int32_t i = 0; i < 6; ++i)
                {
                    if (pSkill->GetVar(6 + i))
                        vStateID.push_back(pSkill->GetVar(6 + i));
                }

                if (!vStateID.empty())
                {
                    StateCode code = static_cast<StateCode>(vStateID[irand(0, static_cast<int32_t>(vStateID.size()) - 1)]);

                    bResult = pTarget->AddState(static_cast<StateType>(pSkill->GetSkillBase()->GetStateType()), code, pCaster->GetHandle(), pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance()), t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                }
            }
            else
            {
                if (pSkill->GetSkillBase()->GetSkillEffectType() != EF_TOGGLE_AURA &&
                    pSkill->GetSkillBase()->GetSkillEffectType() != EF_TOGGLE_DIFFERENTIAL_AURA)
                {
                    if (pTarget != pCaster || (pSkill->GetSkillBase()->GetSkillEffectType() != EF_PHYSICAL_ABSORB_DAMAGE &&
                                               pSkill->GetSkillBase()->GetSkillEffectType() != EF_MAGIC_ABSORB_DAMAGE_OLD &&
                                               pSkill->GetSkillBase()->GetSkillEffectType() != EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP &&
                                               pSkill->GetSkillBase()->GetSkillEffectType() != EF_ADD_HP_MP_BY_ABSORB_HP_MP &&
                                               pSkill->GetSkillBase()->GetSkillEffectType() != EF_PHYSICAL_SINGLE_DAMAGE_ABSORB))
                    {
                        bResult = pTarget->AddState(static_cast<StateType>(pSkill->GetSkillBase()->GetStateType()), static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId()),
                                                    pCaster->GetHandle(), pSkill->GetSkillBase()->GetStateLevel(pSkill->GetRequestedSkillLevel(), pSkill->GetSkillEnhance()),
                                                    t, end_time, false, 0, "") == TS_RESULT_SUCCESS;
                    }
                }
            }

            if (pSkill->GetSkillId() != SKILL_FORCE_CHIP && pSkill->GetSkillId() != SKILL_SOUL_CHIP && pSkill->GetSkillId() != SKILL_LUNA_CHIP)
            {
                if (pTarget->IsMonster() && pTarget->IsEnemy(pCaster, true))
                {
                    auto pMonster = pTarget->As<Monster>();
                    pMonster->AddHate(pCaster->GetHandle(), 1, true, true);
                }
                else if (pTarget->IsNPC() && pTarget->IsEnemy(pCaster, true))
                {
                    //auto pNPC = pTarget->As<NPC>();

                    //pNPC->SetAttacker(pCaster);
                }
            }
        }

        Skill::AddSkillResult(*m_vResult, bResult, SRST_AddState, pTarget->GetHandle());

        return true;
    }

private:
    std::vector<SkillResult> *m_vResult{};
};

struct RemoveBadStateSkillFunctor : public SkillTargetFunctor
{
    RemoveBadStateSkillFunctor(std::vector<SkillResult> *pvList) : m_vList(pvList)
    {
    }

    bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) override
    {
        int32_t nStateLevel = pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetRequestedSkillLevel() + pSkill->GetVar(3) * pSkill->GetSkillEnhance();
        pTarget->RemoveState(static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(0))), nStateLevel);
        Skill::AddSkillResult(*m_vList, true, SRST_RemoveState, pTarget->GetHandle());

        for (int32_t nIndex = 4; nIndex < 9 && pSkill->GetVar(nIndex); nIndex++)
        {
            pTarget->RemoveState(static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(nIndex))), nStateLevel);
            Skill::AddSkillResult(*m_vList, true, SRST_RemoveState, pTarget->GetHandle());
        }

        return true;
    }

private:
    std::vector<SkillResult> *m_vList{nullptr};
};

struct RemoveGoodStateSkillFunctor : public SkillTargetFunctor
{
    RemoveGoodStateSkillFunctor(std::vector<SkillResult> *pvList) : m_vList(pvList)
    {
    }

    bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) override
    {
        int32_t nStateLevel = pSkill->GetVar(1) * pSkill->GetRequestedSkillLevel() + pSkill->GetVar(2) * pSkill->GetSkillEnhance();

        bool bResult = true;
        int32_t accuracy_bonus = pSkill->GetVar(9) + pSkill->GetVar(10) * pSkill->GetRequestedSkillLevel();
        if (accuracy_bonus <= irand(0, 100))
            bResult = false;

        if (pSkill->GetVar(8))
        {
            if (bResult)
                pTarget->RemoveGoodState(nStateLevel);
            Skill::AddSkillResult(*m_vList, bResult, SRST_RemoveState, pTarget->GetHandle());
        }
        else
        {
            if (bResult)
                pTarget->RemoveState(static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(0))), nStateLevel);
            Skill::AddSkillResult(*m_vList, bResult, SRST_RemoveState, pTarget->GetHandle());

            for (int32_t nIndex = 3; nIndex < 8 && pSkill->GetVar(nIndex); nIndex++)
            {
                if (bResult)
                    pTarget->RemoveState(static_cast<StateCode>(static_cast<int32_t>(pSkill->GetVar(nIndex))), nStateLevel);
                Skill::AddSkillResult(*m_vList, bResult, SRST_RemoveState, pTarget->GetHandle());
            }
        }
        return true;
    }

private:
    std::vector<SkillResult> *m_vList{nullptr};
};

struct RecoveryMPSkillFunctor : public SkillTargetFunctor
{
    RecoveryMPSkillFunctor(std::vector<SkillResult> *pvList, bool bIsByItem = false) : m_vList(pvList), m_bIsByItem(bIsByItem)
    {
    }

    bool onCreature(Skill *pSkill, uint32_t t, Unit *pCaster, Unit *pTarget) override
    {
        int32_t slv = pSkill->GetRequestedSkillLevel();
        int32_t enhance = pSkill->GetSkillEnhance();
        nResult =
            pCaster->GetMagicPoint((ElementalType)pSkill->GetSkillBase()->GetElementalType(), pSkill->GetSkillBase()->IsPhysicalSkill(), pSkill->GetSkillBase()->IsHarmful()) * (pSkill->GetVar(0) + slv * pSkill->GetVar(1)) + pSkill->GetVar(2) + slv * pSkill->GetVar(3) + enhance * pSkill->GetVar(6) +
            +pTarget->GetMaxMana() * (pSkill->GetVar(4) + slv * pSkill->GetVar(5) + enhance * pSkill->GetVar(7));

        if (pTarget->GetHealth() == 0)
            nResult = 0;
        else if (m_bIsByItem)
            nResult = pTarget->MPHealByItem(nResult);
        else
            nResult = pTarget->MPHeal(nResult);

        SkillResult skill_result{};
        skill_result.type = TS_SKILL__HIT_TYPE::SHT_ADD_MP;
        skill_result.hTarget = pTarget->GetHandle();
        skill_result.hitAddStat.nIncStat = nResult;
        skill_result.hitAddStat.target_stat = pTarget->GetMana();

        m_vList->push_back(skill_result);

        return true;
    }

private:
    std::vector<SkillResult> *m_vList{nullptr};
    int32_t nResult{0};
    bool m_bIsByItem{false};
};