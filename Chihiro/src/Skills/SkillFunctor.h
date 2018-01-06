#ifndef PROJECT_SKILLFUNCTOR_H
#define PROJECT_SKILLFUNCTOR_H

#include "Common.h"
#include "Skill.h"
#include "Player.h"

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
                chanceRes = 1;
            } else {
                if (pSkill->m_SkillBase->effect_type >= 301 && pSkill->m_SkillBase->effect_type <= 309) {
                    //chanceRes = (int)((pCaster->m_Attribute.nMagicAccuracy - pCaster->m_Attribute.nMagicAvoid))
                } else {

                }
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
            }
        }
    }
};

#endif // PROJECT_SKILLFUNCTOR_H
