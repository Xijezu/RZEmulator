#ifndef PROJECT_SKILLFUNCTOR_H
#define PROJECT_SKILLFUNCTOR_H

#include "Common.h"
#include "Skill.h"
#include "Player.h"
#include "GameRule.h"

struct SkillTargetFunctor {
    virtual void onCreature(Skill* pSkill, uint t, Unit* pCaster, Unit* pTarget) = 0;
};

struct FireSkillStateSkillFunctor : public SkillTargetFunctor {
    void onCreature(Skill* pSkill, uint t, Unit* pCaster, Unit* pTarget) override
    {
        int chanceRes = 1;
        bool bResult = true;
        if (pSkill->m_SkillBase->is_harmful != 0) {
            if (pSkill->m_SkillBase->id >= 6008 && pSkill->m_SkillBase->id <= 6010) {
                if(GameRule::GetChipLevelLimit(pSkill->m_nRequestedSkillLevel) >= pTarget->GetLevel()) {
                    chanceRes = 1;
                } else {
                    chanceRes = (10 * (GameRule::GetChipLevelLimit(pSkill->m_nRequestedSkillLevel) - pTarget->GetLevel() + 10 )) - ((uint)rand32() % 100);
                }
            } else if (pSkill->m_SkillBase->effect_type >= 301 && pSkill->m_SkillBase->effect_type <= 309) {
                //chanceRes = (int)((pCaster->m_Attribute.nMagicAccuracy - pCaster->m_Attribute.nMagicAvoid))
                chanceRes = 1;
            } else {
                chanceRes = (pSkill->m_SkillBase->probability_on_hit + pSkill->m_nRequestedSkillLevel * pSkill->m_SkillBase->probability_inc_by_slv);
            }
        }
        if(chanceRes <= 0) {
            bResult = false;
        } else {
            int  gss      = pSkill->m_SkillBase->GetStateSecond(pSkill->m_nRequestedSkillLevel, 0);
            uint end_time = 0;
            if (gss >= 0)
                end_time = t + gss * 100;
            else
                end_time = t + 6000;

            switch (pSkill->m_SkillBase->effect_type) {
                case 301:
                case 302:
                case 305:
                case 306: {
                    int count  = 5;
                    int nLevel = 0;
                    if (pSkill->m_SkillBase->effect_type == 305 || pSkill->m_SkillBase->effect_type == 306)
                        count = 0;
                    nLevel    = pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, 0);
                    StateCode stateCode{ };
                    if (count > 0) {
                        for (int i = -1; i < count; ++i) {
                            if (i >= 0)
                                stateCode = (StateCode) (int) pSkill->m_SkillBase->var[i];
                            else
                                stateCode = (StateCode) pSkill->m_SkillBase->state_id;
                            if (stateCode != StateCode::SC_NONE) {
                                bResult = pTarget->AddState((StateType) pSkill->m_SkillBase->state_type, stateCode, pCaster->GetHandle(),
                                                            nLevel, t, end_time, false, 0, "") == 0;
                            }
                        }
                    }
                }
                    break;
                case 314: {
                    int nLevel = pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, 0);
                    StateCode stateCode = (StateCode)pSkill->m_SkillBase->state_id;
                    if(stateCode != StateCode::SC_NONE)
                        bResult = pTarget->AddState((StateType) pSkill->m_SkillBase->state_type, stateCode, pCaster->GetHandle(),
                                                    nLevel, t, end_time, false, 0, "") == 0;
                }
                    break;
                default:
                    if(pSkill->m_SkillBase->effect_type != 701 && pSkill->m_SkillBase->effect_type != 702) {
                        if((!pCaster->GetHandle() != pTarget->GetHandle()
                        || pSkill->m_SkillBase->effect_type != 121
                        && pSkill->m_SkillBase->effect_type != 221
                        && pSkill->m_SkillBase->effect_type != 235
                        && pSkill->m_SkillBase->effect_type != 266
                        && pSkill->m_SkillBase->effect_type != 30002))
                            bResult = pTarget->AddState((StateType) pSkill->m_SkillBase->state_type, (StateCode) pSkill->m_SkillBase->state_id,
                                                        pCaster->GetHandle(), pSkill->m_SkillBase->GetStateLevel(pSkill->m_nRequestedSkillLevel, 0),
                                                        t, end_time, false, 0, "") == 0;
                    }
                    break;
            }
            if(pSkill->m_SkillBase->id >= 6008 && pSkill->m_SkillBase->id <= 6010) {
                if(pTarget->IsMonster()) {
                    dynamic_cast<Monster*>(pTarget)->AddHate(pCaster->GetHandle(), 1, true, true);
                }
            }
        }
    }
};

#endif // PROJECT_SKILLFUNCTOR_H
