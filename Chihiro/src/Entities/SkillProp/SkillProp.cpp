/*
  *  Copyright (C) 2018 Xijezu <http://xijezu.com/>
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

#include <Skills/Skill.h>
#include "SkillProp.h"
#include "World.h"
#include "MemPool.h"

SkillProp *SkillProp::Create(uint caster, Skill *pSkill, int nMagicPoint, float fHateRatio)
{
    return new SkillProp(caster, pSkill, nMagicPoint, fHateRatio);
}

void SkillProp::Update(uint/* diff*/)
{
    if(m_bProcessEnded)
        return;

    Unit* pCaster{nullptr};

    uint ct = sWorld->GetArTime();
    if(ct > m_Info.m_nEndTime  || m_bIsRemovePended)
    {
        m_bProcessEnded = true;

        sWorld->RemoveObjectFromWorld(this);
        DeleteThis();
    }
    else
    {
        if(ct < m_Info.m_nLastFireTime + m_Info.m_nInterval)
            return;

        m_Info.m_nLastFireTime = ct;
        pCaster = sMemoryPool->GetObjectInWorld<Unit>(m_hCaster);
        if(pCaster != nullptr)
        {
            m_pSkill->m_targetPosition.Relocate(this);
            m_pSkill->m_vResultList.clear();

            m_pSkill->m_nTargetCount = 0;

            switch (m_pSkill->m_SkillBase->effect_type)
            {
                case 271:
                    FIRE_AREA_EFFECT_MAGIC_DAMAGE(pCaster);
                    break;

//                             case 272:
//                                 this.FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL(pCaster);
//                                 break;
//
//                             case 273:
//                                 this.FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2(pCaster);
//                                 break;
//
//                             case 352:
//                                 this.FIRE_AREA_EFFECT_MAGIC_DAMAGE_OLD(pCaster);
//                                 break;
//
//                             case 353:
//                                 this.FIRE_AREA_EFFECT_HEAL(pCaster);
//                                 break;
//
//                             case 382:
//                                 this.FIRE_TRAP_DAMAGE(pCaster);
//                                 break;
//
//                             case 383:
//                             case 384:
//                                 this.FIRE_TRAP_MULTIPLE_DAMAGE(pCaster);
//                                 break;
//
//                             case 9503:
//                                 this.FIRE_AREA_EFFECT_HEAL_BY_FIELD_PROP(pCaster);
//                                 break;
                default:
                    MX_LOG_ERROR("skill", "SkillProp::Update - Unknown Effect Type: %d", m_pSkill->m_SkillBase->effect_type);
                    break;
            }

            if(m_pSkill->m_nTargetCount != 0)
            {
                m_pSkill->broadcastSkillMessage(this, 0, 0, 4);
            }
        }
        else
        {
            m_bProcessEnded = true;
            sWorld->RemoveObjectFromWorld(this);
            DeleteThis();
        }
    }
}

bool SkillProp::IsSkillProp() const
{
    return true;
}

void SkillProp::PendRemove()
{
    if (!m_bProcessEnded && !m_bIsRemovePended)
    {
        m_bIsRemovePended = true;
    }
}

void SkillProp::INIT_AREA_EFFECT_MAGIC_DAMAGE()
{
    uint ct = sWorld->GetArTime();
    m_Info.m_nStartTime = ct;
    m_Info.m_nEndTime   = (uint)(((m_pSkill->m_SkillBase->var[3] + (m_pSkill->m_SkillBase->var[4] * m_pSkill->m_nRequestedSkillLevel)) * 100) + ct);
    m_Info.m_nInterval  = (uint)(m_pSkill->m_SkillBase->var[6] * 100);
}

void SkillProp::INIT_AREA_EFFECT_HEAL()
{
    uint ct = sWorld->GetArTime();
    m_Info.m_nStartTime = ct;
    m_Info.m_nEndTime = (uint)(((m_pSkill->m_SkillBase->var[7] + (m_pSkill->m_SkillBase->var[8] * m_pSkill->m_nRequestedSkillLevel)) * 100) + ct);
    m_Info.m_nInterval = (uint)(m_pSkill->m_SkillBase->var[10] * 100);
}

void SkillProp::INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP()
{
    uint ct = sWorld->GetArTime();
    m_Info.m_nStartTime = ct;
    m_Info.m_nEndTime = (uint)(((m_pSkill->m_SkillBase->var[7] + (m_pSkill->m_SkillBase->var[8] * m_pSkill->m_nRequestedSkillLevel)) * 100) + ct);
    m_Info.m_nInterval = (uint)(m_pSkill->m_SkillBase->var[5] * 100);
}

void SkillProp::INIT_SKILL_PROP_PARAMETER(uint nDuration, uint nInterval)
{
    uint ct = sWorld->GetArTime();
    m_Info.m_nLastFireTime = 0;
    m_Info.m_nStartTime = ct;
    m_Info.m_nEndTime = nDuration + ct;
    m_Info.m_nInterval = nInterval;
}

void SkillProp::FIRE_AREA_EFFECT_MAGIC_DAMAGE(Unit *pCaster)
{
    std::vector<uint> vResult{ };

    float fRange  = m_pSkill->m_SkillBase->var[9] * 12.0f;
    auto  nDamage = (int)((m_pSkill->m_SkillBase->var[10] + (m_pSkill->m_SkillBase->var[11] * m_pSkill->m_nEnhance))
                          * ((m_pSkill->m_SkillBase->var[5] * m_pSkill->m_nEnhance) + m_nOwnerMagicPoint
                                                                                      * ((m_pSkill->m_SkillBase->var[0] + (m_pSkill->m_SkillBase->var[1] * m_pSkill->m_nRequestedSkillLevel))
                                                                                         + (m_pSkill->m_SkillBase->var[2] * m_pSkill->m_nEnhance))
                             + (m_pSkill->m_SkillBase->var[3] + (m_pSkill->m_SkillBase->var[4] * m_pSkill->m_nRequestedSkillLevel))));

    // sWorld->EnumMovableObject()

    auto t = sWorld->GetArTime();
    Unit *pUnit{nullptr};

    for (const auto &uid : vResult)
    {
        pUnit = sMemoryPool->GetObjectInWorld<Unit>(uid);
        if (pUnit == nullptr || !pCaster->IsEnemy(pUnit, true) || pUnit->GetHealth() == 0)
        {
            continue;
        }

        auto elemental_type = m_pSkill->m_SkillBase->elemental;
        int  flag           = 0;

        auto dmg = pUnit->DealMagicalDamage(m_pSkill->m_pOwner, nDamage, (ElementalType)elemental_type,
                                            m_pSkill->m_SkillBase->GetHitBonus(m_pSkill->m_nEnhance, pCaster->GetLevel() - pUnit->GetLevel()),
                                            m_pSkill->m_SkillBase->critical_bonus + m_pSkill->m_nRequestedSkillLevel * m_pSkill->m_SkillBase->critical_bonus_per_skl,
                                            0, nullptr, nullptr);

        if (dmg.bCritical)
            flag = 1;
        if (dmg.bMiss)
            flag |= 2;
        if (dmg.bBlock)
            flag |= 4;
        if (dmg.bPerfectBlock)
            flag |= 8;

        SkillResult skill_result{ };
        skill_result.type               = 1;
        skill_result.damage.type        = 1;
        skill_result.hTarget            = pUnit->GetHandle();
        skill_result.damage.hTarget     = pUnit->GetHandle();
        skill_result.damage.target_hp   = pUnit->GetHealth();
        skill_result.damage.damage_type = elemental_type;
        skill_result.damage.damage      = dmg.nDamage;
        skill_result.damage.flag        = flag;
        m_pSkill->m_vResultList.emplace_back(skill_result);
        ++m_pSkill->m_nTargetCount;

        if (!dmg.bMiss)
        {
            if (pUnit->IsMonster())
            {
                auto HateMod = pCaster->GetHateMod((m_pSkill->m_SkillBase->is_physical_act == 0 ? 2 : 1), m_pSkill->m_SkillBase->is_harmful != 0);
                auto nHate   = dmg.nDamage + HateMod.second;
                pUnit->As<Monster>()->AddHate(m_pSkill->m_pOwner->GetHandle(), (int)(m_fHateRatio * (nHate * HateMod.first)), true, true);
            }

            if (m_pSkill->m_SkillBase->state_id != 0)
            {
                auto nLevel = m_pSkill->m_SkillBase->GetStateLevel(m_pSkill->m_nRequestedSkillLevel, m_pSkill->m_nEnhance);
                pUnit->AddState((StateType)m_pSkill->m_SkillBase->state_type, (StateCode)m_pSkill->m_SkillBase->state_id,
                                pCaster->GetHandle(), nLevel, t, (uint)(t + m_pSkill->m_SkillBase->GetStateSecond(m_pSkill->m_nRequestedSkillLevel, m_pSkill->m_nEnhance)),
                                false, 0, "");
            }
        }
    }
}

SkillProp::SkillProp(uint caster, Skill *pSkill, int nMagicPoint, float fHateRatio) : WorldObject(false)
{
    m_hCaster          = caster;
    m_pSkill           = pSkill;
    m_fHateRatio       = fHateRatio;
    m_nOwnerMagicPoint = nMagicPoint;
    sMemoryPool->AllocMiscHandle(this);

    _valuesCount = 1;
    _InitValues();

    _objType          = OBJ_STATIC;
    m_bFired          = false;
    m_bProcessEnded   = false;
    m_bIsRemovePended = false;

    uint nDuration{ };
    uint nInterval{ };

    switch (m_pSkill->m_SkillBase->effect_type)
    {
        case 271:
        case 272:
            nDuration = (uint)((m_pSkill->m_SkillBase->var[6] + (m_pSkill->m_SkillBase->var[7] * m_pSkill->m_nRequestedSkillLevel)) * 100);
            nInterval = (uint)(m_pSkill->m_SkillBase->var[8] * 100);
            INIT_SKILL_PROP_PARAMETER(nDuration, nInterval);
            break;

        case 273:
            nDuration = (uint)((m_pSkill->m_SkillBase->var[12] + (m_pSkill->m_SkillBase->var[13] * m_pSkill->m_nRequestedSkillLevel)) * 100);
            nInterval = (uint)(m_pSkill->m_SkillBase->var[14] * 100);
            INIT_SKILL_PROP_PARAMETER(nDuration, nInterval);
            break;

        case 352:
            INIT_AREA_EFFECT_MAGIC_DAMAGE();
            break;

        case 353:
            INIT_AREA_EFFECT_HEAL();
            break;

        case 382:
        case 383:
        case 384:
            nDuration = (uint)((m_pSkill->m_SkillBase->var[3] + (m_pSkill->m_SkillBase->var[4] * m_pSkill->m_nRequestedSkillLevel)) * 100);
            nInterval = 30;
            INIT_SKILL_PROP_PARAMETER(nDuration, nInterval);
            break;

        case 9503:
            INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP();
            break;

        default:
            MX_LOG_ERROR("skill", "SkillProp::SkillProp - Unknown Effect Type: %d", m_pSkill->m_SkillBase->effect_type);
            break;

    }
}
