#pragma once
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
#include "Common.h"
#include "Skill.h"
#include "Player.h"
#include "GameRule.h"

struct SkillTargetFunctor
{
    virtual bool onCreature(Skill *pSkill, uint t, Unit *pCaster, Unit *pTarget) = 0;
};

struct HealingSkillFunctor : public SkillTargetFunctor
{
        HealingSkillFunctor(std::vector<SkillResult> *pSRList, bool bIsByItem = false) : m_pSRList(pSRList), m_bIsByItem(bIsByItem)
        {
        }

        bool onCreature(Skill *pSkill, uint t, Unit *pCaster, Unit *pTarget) override
        {
            auto slv     = pSkill->GetRequestedSkillLevel();
            int  nResult = pCaster->GetMagicPoint((ElementalType)pSkill->GetSkillBase()->GetElementalType(), pSkill->GetSkillBase()->IsPhysicalSkill(), pSkill->GetSkillBase()->IsHarmful())
                           * (pSkill->GetVar(0) + pSkill->GetVar(1) * slv)
                           + pSkill->GetVar(2) + pSkill->GetVar(3) * slv
                           + pSkill->GetVar(6) * pSkill->GetSkillEnhance()
                           + pTarget->GetMaxHealth() * (pSkill->GetVar(4) + pSkill->GetVar(5) * slv + pSkill->GetVar(7) * pSkill->GetSkillEnhance());

            if (pTarget->GetHealth() == 0)
                nResult = 0;
            else if (m_bIsByItem)
                nResult = pTarget->HealByItem(nResult);
            else
                nResult = pTarget->Heal(nResult);

            SkillResult skillResult{ };
            skillResult.type                   = SRT_ADD_HP;
            skillResult.hTarget                = pTarget->GetHandle();
            skillResult.hitAddStat.target_stat = pTarget->GetHealth();
            skillResult.hitAddStat.nIncStat    = nResult;
            m_pSRList->emplace_back(skillResult);

            return true;
        }

    private:
        std::vector<SkillResult> *m_pSRList{nullptr};
        bool m_bIsByItem;
};

struct FireSkillStateSkillFunctor : public SkillTargetFunctor
{
    std::vector<SkillResult> pvList;

    bool onCreature(Skill *pSkill, uint t, Unit *pCaster, Unit *pTarget) override
    {
        int  chanceRes = 1;
        bool bResult   = true;
        if (pSkill->m_SkillBase->is_harmful != 0)
        {
            if (pSkill->m_SkillBase->id >= 6008 && pSkill->m_SkillBase->id <= 6010)
            {
                if (GameRule::GetChipLevelLimit(pSkill->m_nRequestedSkillLevel) >= pTarget->GetLevel())
                {
                    chanceRes = 1;
                }
                else
                {
                    chanceRes = (10 * (GameRule::GetChipLevelLimit(pSkill->m_nRequestedSkillLevel) - pTarget->GetLevel() + 10)) - ((uint)rand32() % 100);
                }
            }
            else if (pSkill->m_SkillBase->effect_type >= 301 && pSkill->m_SkillBase->effect_type <= 309)
            {
                //chanceRes = (int)((pCaster->m_Attribute.nMagicAccuracy - pCaster->m_Attribute.nMagicAvoid))
                chanceRes = 1;
            }
            else
            {
                chanceRes = (pSkill->m_SkillBase->probability_on_hit + pSkill->m_nRequestedSkillLevel * pSkill->m_SkillBase->probability_inc_by_slv);
            }
        }
        if (chanceRes <= 0)
        {
            bResult = false;
        }
        else
        {
            int  gss      = pSkill->m_SkillBase->GetStateSecond(pSkill->m_nRequestedSkillLevel, pSkill->GetSkillEnhance());
            uint end_time = 0;
            if (gss >= 0)
                end_time = t + gss * 100;
            else
                end_time = t + 6000;

            switch (pSkill->m_SkillBase->effect_type)
            {
                case 301:
                case 302:
                case 305:
                case 306:
                {
                    int count  = 5;
                    int nLevel = 0;
                    if (pSkill->m_SkillBase->effect_type == 305 || pSkill->m_SkillBase->effect_type == 306)
                        count = 0;
                    nLevel    = pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, pSkill->GetSkillEnhance());
                    StateCode stateCode{ };
                    if (count > 0)
                    {
                        for (int i = -1; i < count; ++i)
                        {
                            if (i >= 0)
                                stateCode = (StateCode)(int)pSkill->m_SkillBase->var[i];
                            else
                                stateCode = (StateCode)pSkill->m_SkillBase->state_id;
                            if (stateCode != StateCode::SC_NONE)
                            {
                                bResult = pTarget->AddState((StateType)pSkill->m_SkillBase->state_type, stateCode, pCaster->GetHandle(),
                                                            nLevel, t, end_time, false, 0, "") == 0;
                            }
                        }
                    }
                }
                    break;
                case 314:
                {
                    int  nLevel    = pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, pSkill->GetSkillEnhance());
                    auto stateCode = (StateCode)pSkill->m_SkillBase->state_id;
                    if (stateCode != StateCode::SC_NONE)
                        bResult = pTarget->AddState((StateType)pSkill->m_SkillBase->state_type, stateCode, pCaster->GetHandle(),
                                                    nLevel, t, end_time, false, 0, "") == 0;
                }
                    break;
                default:
                    if (pSkill->m_SkillBase->effect_type != 701 && pSkill->m_SkillBase->effect_type != 702)
                    {
                        if ((pCaster->GetHandle() != pTarget->GetHandle()
                             || (pSkill->m_SkillBase->effect_type != 121
                                 && pSkill->m_SkillBase->effect_type != 221
                                 && pSkill->m_SkillBase->effect_type != 235
                                 && pSkill->m_SkillBase->effect_type != 266
                                 && pSkill->m_SkillBase->effect_type != 30002)))
                            bResult = pTarget->AddState((StateType)pSkill->m_SkillBase->state_type, (StateCode)pSkill->m_SkillBase->state_id,
                                                        pCaster->GetHandle(), pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, 0),
                                                        t, end_time, false, 0, "") == 0;
                    }
                    break;
            }
            if (pSkill->m_SkillBase->id >= 6008 && pSkill->m_SkillBase->id <= 6010)
            {
                if (pTarget->IsMonster())
                {
                    dynamic_cast<Monster *>(pTarget)->AddHate(pCaster->GetHandle(), 1, true, true);
                }
            }
        }
        sWorld.AddSkillResult(pvList, bResult, 10, pTarget->GetHandle());
        return bResult;
    }
};

struct RemoveGoodStateSkillFunctor : public SkillTargetFunctor
{
    std::vector<SkillResult> &pvList;

    bool onCreature(Skill *pSkill, uint t, Unit *pCaster, Unit *pTarget) override
    {
        auto nEnhance    = pSkill->GetSkillEnhance();
        auto v13         = pSkill->m_SkillBase->var[9] + (pSkill->m_SkillBase->var[10] * pSkill->m_nRequestedSkillLevel);
        int  nSkillLevel = ((int)pSkill->m_SkillBase->var[1] * pSkill->m_nRequestedSkillLevel) + ((int)pSkill->m_SkillBase->var[2] * nEnhance);

        bool bResult = (v13 <= (uint)rand32() % 100);

        auto targetHandle = pTarget->GetHandle();
        if (pSkill->m_SkillBase->var[8] == 0.0f)
        {
            if (bResult)
                pTarget->RemoveState((StateCode)static_cast<int>(pSkill->m_SkillBase->var[0]), nSkillLevel);

            sWorld.AddSkillResult(pvList, bResult, 11, targetHandle);
            auto counter = 304;

            for (int i = 0; i < 5; ++i)
            {
                auto val = pSkill->m_SkillBase->var[9 + i];
                if (val == 0.0f)
                    break;

                if (bResult)
                    pTarget->RemoveState((StateCode)static_cast<int>(val), nSkillLevel);
                sWorld.AddSkillResult(pvList, bResult, 11, targetHandle);
            }
        }
        else
        {
            if (bResult)
                pTarget->RemoveGoodState(nSkillLevel);
            sWorld.AddSkillResult(pvList, bResult, 11, targetHandle);
        }
        return bResult;
    }
};