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

#include "SkillProp.h"

#include "Log.h"
#include "MemPool.h"
#include "Skill.h"
#include "World.h"
#include "XPacket.h"

SkillProp *SkillProp::Create(uint32_t caster, Skill *pSkill, int32_t nMagicPoint, float fHateRatio)
{
    return new SkillProp(caster, *pSkill, nMagicPoint, fHateRatio);
}

void SkillProp::EnterPacket(TS_SC_ENTER &pEnterPct, SkillProp *pSkillProp, Player * /*pPlayer*/)
{
    TS_SC_ENTER__SKILL_INFO skillInfo{};
    skillInfo.casterHandle = pSkillProp->m_hCaster;
    skillInfo.startTime = pSkillProp->start_time;
    skillInfo.skillId = pSkillProp->m_pSkill.GetSkillId();
    pEnterPct.skillInfo = skillInfo;
}

SkillProp::SkillProp(uint32_t caster, Skill pSkill, int32_t nMagicPoint, float fHateRatio)
    : WorldObject(false)
    , m_hCaster(caster)
    , m_pSkill(pSkill)
    , m_nOwnerMagicPoint(nMagicPoint)
    , m_fHateRatio(fHateRatio)
{
    _mainType = MT_StaticObject;
    _subType = ST_SkillProp;
    _objType = OBJ_STATIC;
    _valuesCount = 1;
    _InitValues();

    sMemoryPool.AllocMiscHandle(this);

    _objType = OBJ_STATIC;
    m_bFired = false;
    m_bProcessEnded = false;
    m_bIsRemovePended = false;

    switch (m_pSkill.GetSkillBase()->GetSkillEffectType()) {
    case EF_AREA_EFFECT_MAGIC_DAMAGE_OLD: {
        INIT_AREA_EFFECT_MAGIC_DAMAGE();
        break;
    }
    case EF_AREA_EFFECT_HEAL: {
        INIT_AREA_EFFECT_HEAL();
        break;
    }
    case EF_AREA_EFFECT_HEAL_BY_FIELD_PROP: {
        INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP();
        break;
    }
    case EF_AREA_EFFECT_MAGIC_DAMAGE:
    case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL: {
        INIT_SKILL_PROP_PARAMETER((m_pSkill.GetVar(6) + m_pSkill.GetVar(7) * m_pSkill.GetRequestedSkillLevel()) * 100, m_pSkill.GetVar(8) * 100);
        break;
    }
    case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2: {
        INIT_SKILL_PROP_PARAMETER((m_pSkill.GetVar(12) + m_pSkill.GetVar(13) * m_pSkill.GetRequestedSkillLevel()) * 100, m_pSkill.GetVar(14) * 100);
        break;
    }
        // Ʈ��
    case EF_TRAP_PHYSICAL_DAMAGE:
    case EF_TRAP_MAGICAL_DAMAGE:
    case EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE:
    case EF_TRAP_MULTIPLE_MAGICAL_DAMAGE: {
        INIT_SKILL_PROP_PARAMETER((m_pSkill.GetVar(3) + m_pSkill.GetVar(4) * m_pSkill.GetRequestedSkillLevel()) * 100, 30);
        break;
    }
    default:
        NG_LOG_ERROR("entities.skill", "SkillProp::SkillProp - Unknown Effect Type: %d", m_pSkill.m_SkillBase->effect_type);
        break;
    }
}

void SkillProp::Update(uint32_t /* diff*/)
{
    if (m_bProcessEnded)
        return;

    Unit *pCaster{nullptr};

    uint32_t ct = sWorld.GetArTime();
    if (ct > m_Info.m_nEndTime || m_bIsRemovePended) {
        m_bProcessEnded = true;

        pCaster = sMemoryPool.GetObjectInWorld<Unit>(m_hCaster);
        if (pCaster != nullptr && pCaster->GetTrapHandle() == GetHandle())
            pCaster->SetTrapHandle(0);

        sWorld.RemoveObjectFromWorld(this);
        DeleteThis();
        return;
    }

    if (ct < m_Info.m_nLastFireTime + m_Info.m_nInterval)
        return;

    m_Info.m_nLastFireTime = ct;

    pCaster = sMemoryPool.GetObjectInWorld<Unit>(m_hCaster);
    if (pCaster == nullptr) {
        m_bProcessEnded = true;
        sWorld.RemoveObjectFromWorld(this);
        DeleteThis();
        return;
    }

    m_pSkill.m_targetPosition.Relocate(this);
    m_pSkill.m_vResultList.clear();
    m_pSkill.m_nTargetCount = 0;

    switch (m_pSkill.GetSkillBase()->GetSkillEffectType()) {
    case EF_AREA_EFFECT_MAGIC_DAMAGE_OLD: {
        FIRE_AREA_EFFECT_MAGIC_DAMAGE_OLD(pCaster);
        break;
    }
    case EF_AREA_EFFECT_HEAL: {
        FIRE_AREA_EFFECT_HEAL(pCaster);
        break;
    }
    case EF_AREA_EFFECT_HEAL_BY_FIELD_PROP: {
        FIRE_AREA_EFFECT_HEAL_BY_FIELD_PROP(pCaster);
        break;
    }
    case EF_AREA_EFFECT_MAGIC_DAMAGE: {
        FIRE_AREA_EFFECT_MAGIC_DAMAGE(pCaster);
        break;
    }
    case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL: {
        FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL(pCaster);
        break;
    }
    case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2: {
        FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2(pCaster);
        break;
    }
    case EF_TRAP_PHYSICAL_DAMAGE:
    case EF_TRAP_MAGICAL_DAMAGE: {
        FIRE_TRAP_DAMAGE(pCaster);
        break;
    }
    case EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE:
    case EF_TRAP_MULTIPLE_MAGICAL_DAMAGE: {
        FIRE_TRAP_MULTIPLE_DAMAGE(pCaster);
        break;
    }
    default:
        NG_LOG_ERROR("entities.skill", "SkillProp::Update - Unknown Effect Type: %d", m_pSkill.m_SkillBase->effect_type);
        break;
    }

    if (m_pSkill.m_nTargetCount != 0)
        m_pSkill.broadcastSkillMessage(this, 0, 0, TS_SKILL__TYPE::ST_RegionFire);
}

bool SkillProp::IsSkillProp() const
{
    return true;
}

void SkillProp::PendRemove()
{
    if (!m_bProcessEnded && !m_bIsRemovePended) {
        m_bIsRemovePended = true;
    }
}

void SkillProp::INIT_AREA_EFFECT_MAGIC_DAMAGE()
{
    m_Info.m_nStartTime = sWorld.GetArTime();
    m_Info.m_nEndTime = m_Info.m_nStartTime + (m_pSkill.GetVar(3) + m_pSkill.GetVar(4) * m_pSkill.GetRequestedSkillLevel()) * 100;
    m_Info.m_nInterval = static_cast<uint32_t>(m_pSkill.GetVar(6) * 100);
    m_Info.m_nLastFireTime = 0;
}

void SkillProp::INIT_AREA_EFFECT_HEAL()
{
    m_Info.m_nStartTime = sWorld.GetArTime();
    m_Info.m_nEndTime = m_Info.m_nStartTime + (m_pSkill.GetVar(7) + m_pSkill.GetVar(8) * m_pSkill.GetRequestedSkillLevel()) * 100;
    m_Info.m_nInterval = static_cast<uint32_t>(m_pSkill.GetVar(10) * 100);
    m_Info.m_nLastFireTime = 0;
}

void SkillProp::INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP()
{
    m_Info.m_nStartTime = sWorld.GetArTime();
    m_Info.m_nEndTime = m_Info.m_nStartTime + (m_pSkill.GetVar(7) + m_pSkill.GetVar(8) * m_pSkill.GetRequestedSkillLevel()) * 100;
    m_Info.m_nInterval = static_cast<uint32_t>(m_pSkill.GetVar(5) * 100);
    m_Info.m_nLastFireTime = 0;
}

void SkillProp::INIT_SKILL_PROP_PARAMETER(uint32_t nDuration, uint32_t nInterval)
{
    m_Info.m_nStartTime = sWorld.GetArTime();
    m_Info.m_nEndTime = m_Info.m_nStartTime + nDuration;
    m_Info.m_nInterval = nInterval;
    m_Info.m_nLastFireTime = 0;
}

void SkillProp::FIRE_AREA_EFFECT_MAGIC_DAMAGE(Unit *pCaster)
{
    std::vector<Unit *> vResult{};
    vResult.reserve(30);

    float fRange = m_pSkill.GetVar(9) * 12.0f;

    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();
    int32_t nDamage = m_nOwnerMagicPoint * (m_pSkill.GetVar(0) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(1) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(2)) + m_pSkill.GetVar(3) +
        m_pSkill.GetVar(4) * m_pSkill.GetRequestedSkillLevel() + m_pSkill.GetVar(5) * m_pSkill.GetSkillEnhance();
    nDamage *= m_pSkill.GetVar(10) + m_pSkill.GetVar(11) * m_pSkill.GetSkillEnhance();

    nDamage = Skill::EnumSkillTargetsAndCalcDamage(GetPosition(), GetLayer(), GetPosition(), false, fRange, -1, 0, nDamage, true, pCaster, m_pSkill.GetVar(16), m_pSkill.GetVar(17), vResult, false);

    SkillResult skill_result{};
    auto t = sWorld.GetArTime();

    for (auto &pUnit : vResult) {
        if (pUnit == nullptr /* IsPet() */)
            continue;

        if (!pCaster->IsEnemy(pUnit, true))
            continue;

        if (pUnit->GetHealth() == 0)
            continue;

        int32_t flag = 0;
        auto Damage = pUnit->DealMagicalDamage(m_pSkill.m_pOwner, nDamage, (ElementalType)elemental_type,
            m_pSkill.GetSkillBase()->GetHitBonus(m_pSkill.GetSkillEnhance(), pCaster->GetLevel() - pUnit->GetLevel()), m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0,
            nullptr, nullptr);

        if (Damage.bCritical)
            flag |= AIF_Critical;
        if (Damage.bBlock)
            flag |= AIF_Block;
        if (Damage.bMiss)
            flag |= AIF_Miss;
        if (Damage.bPerfectBlock)
            flag |= AIF_PerfectBlock;

        skill_result.type = SHT_MAGIC_DAMAGE;
        skill_result.hTarget = pUnit->GetHandle();
        skill_result.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(elemental_type);
        skill_result.hitDamage.damage.damage = Damage.nDamage;
        skill_result.hitDamage.damage.flag = flag;
        skill_result.hitDamage.damage.target_hp = pUnit->GetHealth();
        m_pSkill.m_vResultList.emplace_back(skill_result);

        ++m_pSkill.m_nTargetCount;

        if (!Damage.bMiss) {
            if (pUnit->IsMonster()) {
                int32_t nHate = Damage.nDamage;

                auto HateMod = pCaster->GetHateMod((m_pSkill.GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, m_pSkill.GetSkillBase()->IsHarmful());

                nHate += HateMod.second;
                nHate *= static_cast<int32_t>(HateMod.first);

                pUnit->As<Monster>()->AddHate(m_pSkill.m_pOwner->GetHandle(), static_cast<int32_t>(m_fHateRatio * nHate), false, false);
            } /*
            else if (pUnit->IsNPC())
                pUnit->As<NPC>()->SetAttacker(m_pSkill.m_pOwner); */

            if (!Damage.bMiss && !pUnit->IsDead()) {
                StateCode nStateCode = static_cast<StateCode>(m_pSkill.GetSkillBase()->GetStateId());

                if (nStateCode != 0) {
                    int32_t nLevel = m_pSkill.GetSkillBase()->GetStateLevel(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());

                    uint32_t nDuration = m_pSkill.GetSkillBase()->GetStateSecond(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());

                    StateType nStateType = static_cast<StateType>(m_pSkill.GetSkillBase()->GetStateType());

                    pUnit->AddState(nStateType, nStateCode, pCaster->GetHandle(), nLevel, t, t + nDuration, false, 0, "");
                }
            }
        }
    }
}

void SkillProp::FIRE_AREA_EFFECT_MAGIC_DAMAGE_OLD(Unit *pCaster)
{
    std::vector<uint32_t> vResult{};
    float fRange = m_pSkill.GetVar(5) * 12.0f;
    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();

    int32_t nDamage = m_nOwnerMagicPoint + m_pSkill.GetVar(0) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(1) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(7);
    nDamage *= m_pSkill.GetVar(2) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(8);

    sWorld.EnumMovableObject(GetPosition(), GetLayer(), fRange, vResult, false, false);

    SkillResult skill_result{};
    Unit *pUnit{nullptr};

    for (auto &handle : vResult) {
        pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr /* IsPet */)
            continue;

        if (!pCaster->IsEnemy(pUnit, true))
            continue;

        if (pUnit->GetHealth() == 0)
            continue;

        int32_t flag = 0;
        auto Damage =
            pUnit->DealMagicalDamage(m_pSkill.m_pOwner, nDamage, (ElementalType)elemental_type, 0, m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0, nullptr, nullptr);

        if (Damage.bCritical)
            flag |= AIF_Critical;
        if (Damage.bBlock)
            flag |= AIF_Block;
        if (Damage.bMiss)
            flag |= AIF_Miss;
        if (Damage.bPerfectBlock)
            flag |= AIF_PerfectBlock;

        skill_result.type = SHT_DAMAGE;
        skill_result.hTarget = pUnit->GetHandle();
        skill_result.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(elemental_type);
        skill_result.hitDamage.damage.damage = Damage.nDamage;
        skill_result.hitDamage.damage.flag = flag;
        skill_result.hitDamage.damage.target_hp = pUnit->GetHealth();
        m_pSkill.m_vResultList.emplace_back(skill_result);

        ++m_pSkill.m_nTargetCount;

        if (!Damage.bMiss && m_pSkill.m_pOwner->GetPosition().GetExactDist2d(this) <= 525.0f) {
            if (pUnit->IsMonster()) {
                int32_t nHate = Damage.nDamage;
                auto HateMod = pCaster->GetHateMod((m_pSkill.GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, m_pSkill.GetSkillBase()->IsHarmful());

                nHate += HateMod.second;
                nHate *= static_cast<int32_t>(HateMod.first);
                pUnit->As<Monster>()->AddHate(m_pSkill.m_pOwner->GetHandle(), static_cast<int32_t>(m_fHateRatio * nHate), true, true);
            }
            /*
            else if (pUnit->IsNPC())
                pUnit->As<NPC>()->SetAttacker(m_pSkill.m_pOwner); */
        }
    }
}

void SkillProp::FIRE_AREA_EFFECT_HEAL(Unit *pCaster)
{
    std::vector<uint32_t> vResult{};

    float fRange = m_pSkill.GetVar(9) * 12.0f;
    m_pSkill.m_fRange = fRange;

    int32_t nHPHeal = m_pSkill.GetVar(0) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(1);
    int32_t nMPHeal = m_pSkill.GetVar(2) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(3);
    int32_t nSPHeal = m_pSkill.GetVar(4) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(5);
    nHPHeal *= m_pSkill.GetVar(11) * m_pSkill.GetSkillEnhance() + 1;
    nMPHeal *= m_pSkill.GetVar(11) * m_pSkill.GetSkillEnhance() + 1;
    nSPHeal *= m_pSkill.GetVar(11) * m_pSkill.GetSkillEnhance() + 1;

    sWorld.EnumMovableObject(GetPosition(), GetLayer(), fRange, vResult, false, false);

    Unit *pUnit{nullptr};
    int32_t nTargetLimit = m_pSkill.GetVar(6);
    SkillResult skill_result{};

    for (const auto &handle : vResult) {
        pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr /* IsPet() */)
            continue;

        if (nTargetLimit == SKILL_EFFECT_TARGET_LIMIT_NOT_ENEMY && pCaster->IsEnemy(pUnit, true))
            continue;

        if (nTargetLimit == SKILL_EFFECT_TARGET_LIMIT_ONLY_ALLY && !pCaster->IsAlly(pUnit))
            continue;

        if (pUnit->GetHealth() == 0)
            continue;

        nHPHeal = pUnit->Heal(nHPHeal);
        nMPHeal = pUnit->MPHeal(nMPHeal);

        if(pUnit->IsSummon()) {
            auto pSummon = pUnit->As<Summon>();
            pSummon->SetSP(pSummon->GetSP() + nSPHeal); 
        }

        skill_result.type = TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP;
        skill_result.hTarget = pUnit->GetHandle();
        skill_result.hitAddHPMPSP.target_hp = pUnit->GetHealth();
        skill_result.hitAddHPMPSP.target_mp = pUnit->GetMana();
        skill_result.hitAddHPMPSP.nIncHP = nHPHeal;
        skill_result.hitAddHPMPSP.nIncMP = nMPHeal;
        skill_result.hitAddHPMPSP.nIncSP = nSPHeal;
        m_pSkill.m_vResultList.emplace_back(skill_result);

        ++m_pSkill.m_nTargetCount;
    }
}

void SkillProp::FIRE_AREA_EFFECT_HEAL_BY_FIELD_PROP(Unit *pCaster)
{
    std::vector<uint32_t> vResult{};

    float fRange = m_pSkill.GetVar(4) * 12.0f;
    m_pSkill.m_fRange = fRange;

    float fHPHeal = m_pSkill.GetVar(0) + m_pSkill.GetVar(1) * m_pSkill.GetRequestedSkillLevel();
    float fMPHeal = m_pSkill.GetVar(2) + m_pSkill.GetVar(3) * m_pSkill.GetRequestedSkillLevel();

    sWorld.EnumMovableObject(GetPosition(), GetLayer(), fRange, vResult, false, false);

    Unit *pUnit{nullptr};
    int32_t nTargetLimit = m_pSkill.GetVar(6);
    SkillResult skill_result{};

    for (auto &handle : vResult) {
        pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr /* IsPet() */)
            continue;

        switch (nTargetLimit) {
        case SKILL_EFFECT_TARGET_LIMIT_NOT_ENEMY:
            if (pCaster->IsEnemy(pUnit, true))
                continue;
            break;
        case SKILL_EFFECT_TARGET_LIMIT_ONLY_ALLY:
            if (!pCaster->IsAlly(pUnit))
                continue;
            break;
        case SKILL_EFFECT_TARGET_LIMIT_ONLY_ENEMY:
            if (!pCaster->IsEnemy(pUnit, false))
                continue;
            break;
        default:
            break;
        }

        if (pUnit->GetHealth() == 0)
            continue;

        int32_t nHPHeal = pUnit->Heal(static_cast<int32_t>(fHPHeal * pUnit->GetMaxHealth()));
        int32_t nMPHeal = pUnit->MPHeal(static_cast<int32_t>(fMPHeal * pUnit->GetMaxMana()));

        skill_result.type = TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP;
        skill_result.hTarget = pUnit->GetHandle();
        skill_result.hitAddHPMPSP.target_hp = pUnit->GetHealth();
        skill_result.hitAddHPMPSP.target_mp = pUnit->GetMana();
        skill_result.hitAddHPMPSP.nIncHP = nHPHeal;
        skill_result.hitAddHPMPSP.nIncMP = nMPHeal;
        skill_result.hitAddHPMPSP.nIncSP = 0;
        m_pSkill.m_vResultList.emplace_back(skill_result);
        ++m_pSkill.m_nTargetCount;
    }
}

void SkillProp::FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL(Unit *pCaster)
{
    std::vector<uint32_t> vResult{};
    float fRange = m_pSkill.GetVar(9) * 12.0f;

    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();
    int32_t nDamage = m_nOwnerMagicPoint * (m_pSkill.GetVar(0) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(1) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(2));
    nDamage *= m_pSkill.GetVar(10) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(11);

    int32_t nHPHeal = m_pSkill.GetVar(3) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(4) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(5);

    sWorld.EnumMovableObject(GetPosition(), GetLayer(), fRange, vResult, false, false);

    Unit *pUnit{nullptr};
    SkillResult skill_result{};

    for (auto &handle : vResult) {
        pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr /* IsPet() */)
            continue;

        if (pUnit->GetHealth() == 0)
            continue;

        if (pCaster->IsEnemy(pUnit, true)) {
            int32_t flag = 0;
            auto Damage = pUnit->DealMagicalDamage(
                m_pSkill.m_pOwner, nDamage, (ElementalType)elemental_type, 0, m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0, nullptr, nullptr);

            if (Damage.bCritical)
                flag |= AIF_Critical;
            if (Damage.bBlock)
                flag |= AIF_Block;
            if (Damage.bMiss)
                flag |= AIF_Miss;
            if (Damage.bPerfectBlock)
                flag |= AIF_PerfectBlock;

            skill_result.type = TS_SKILL__HIT_TYPE::SHT_DAMAGE;
            skill_result.hTarget = pUnit->GetHandle();
            skill_result.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(elemental_type);
            skill_result.hitDamage.damage.damage = Damage.nDamage;
            skill_result.hitDamage.damage.flag = flag;
            skill_result.hitDamage.damage.target_hp = pUnit->GetHealth();
            m_pSkill.m_vResultList.emplace_back(skill_result);
            ++m_pSkill.m_nTargetCount;

            if (!Damage.bMiss) {
                if (pUnit->IsMonster()) {
                    int32_t nHate = Damage.nDamage;

                    auto HateMod = pCaster->GetHateMod((m_pSkill.GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, m_pSkill.GetSkillBase()->IsHarmful());

                    nHate += HateMod.second;
                    nHate *= static_cast<int32_t>(HateMod.first);

                    pUnit->As<Monster>()->AddHate(m_pSkill.m_pOwner->GetHandle(), static_cast<int32_t>(m_fHateRatio * nHate), false, false);
                } /*
                else if (pUnit->IsNPC())
                    pUnit->As<NPC>()->SetAttacker(m_pSkill.m_pOwner);*/
            }
        }
        else if (pCaster->IsAlly(pUnit)) {
            nHPHeal = pUnit->Heal(nHPHeal);

            skill_result.type = TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP;
            skill_result.hTarget = pUnit->GetHandle();
            skill_result.hitAddHPMPSP.target_hp = pUnit->GetHealth();
            skill_result.hitAddHPMPSP.target_mp = pUnit->GetMana();
            skill_result.hitAddHPMPSP.nIncHP = nHPHeal;
            skill_result.hitAddHPMPSP.nIncMP = 0;
            skill_result.hitAddHPMPSP.nIncSP = 0;
            m_pSkill.m_vResultList.emplace_back(skill_result);
            ++m_pSkill.m_nTargetCount;
        }
    }
}

void SkillProp::FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2(Unit *pCaster)
{
    std::vector<Unit *> vResult{};
    vResult.reserve(30);

    auto t = sWorld.GetArTime();
    float fRange = m_pSkill.GetVar(15) * 12.0f;

    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();

    int32_t nHPHeal = m_nOwnerMagicPoint * (m_pSkill.GetVar(0) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(1) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(2)) + m_pSkill.GetVar(3) +
        m_pSkill.GetVar(4) * m_pSkill.GetRequestedSkillLevel() + m_pSkill.GetVar(5) * m_pSkill.GetSkillEnhance();
    int32_t nDamage = m_nOwnerMagicPoint * (m_pSkill.GetVar(6) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(7) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(8)) + m_pSkill.GetVar(9) +
        m_pSkill.GetVar(10) * m_pSkill.GetRequestedSkillLevel() + m_pSkill.GetVar(11) * m_pSkill.GetSkillEnhance();

    nDamage *= m_pSkill.GetVar(18) + m_pSkill.GetSkillEnhance() * m_pSkill.GetVar(19);

    nDamage = Skill::EnumSkillTargetsAndCalcDamage(GetPosition(), GetLayer(), GetPosition(), false, fRange, -1, 0, nDamage, true, pCaster, m_pSkill.GetVar(16), m_pSkill.GetVar(17), vResult, false);

    SkillResult skill_result{};

    for (auto &pUnit : vResult) {
        if (pUnit->GetHealth() == 0)
            continue;

        if (pCaster->IsEnemy(pUnit, true)) {
            int32_t flag = 0;
            auto Damage = pUnit->DealMagicalDamage(
                m_pSkill.m_pOwner, nDamage, (ElementalType)elemental_type, 0, m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0, nullptr, nullptr);

            if (Damage.bCritical)
                flag |= AIF_Critical;
            if (Damage.bBlock)
                flag |= AIF_Block;
            if (Damage.bMiss)
                flag |= AIF_Miss;
            if (Damage.bPerfectBlock)
                flag |= AIF_PerfectBlock;

            skill_result.type = TS_SKILL__HIT_TYPE::SHT_MAGIC_DAMAGE;
            skill_result.hTarget = pUnit->GetHandle();
            skill_result.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(elemental_type);
            skill_result.hitDamage.damage.damage = Damage.nDamage;
            skill_result.hitDamage.damage.flag = flag;
            skill_result.hitDamage.damage.target_hp = pUnit->GetHealth();

            m_pSkill.m_vResultList.emplace_back(skill_result);

            ++m_pSkill.m_nTargetCount;

            if (!Damage.bMiss && pUnit->GetHealth() != 0) {
                StateCode nStateCode = static_cast<StateCode>(m_pSkill.GetSkillBase()->GetStateId());

                if (nStateCode != 0) {
                    int32_t nLevel = m_pSkill.GetSkillBase()->GetStateLevel(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
                    uint32_t nDuration = m_pSkill.GetSkillBase()->GetStateSecond(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
                    StateType nStateType = static_cast<StateType>(m_pSkill.GetSkillBase()->GetStateType());
                    pUnit->AddState(nStateType, nStateCode, pCaster->GetHandle(), nLevel, t, t + nDuration, false, 0, "");
                }

                if (pUnit->IsMonster()) {
                    int32_t nHate = Damage.nDamage;
                    auto HateMod = pCaster->GetHateMod((m_pSkill.GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, m_pSkill.GetSkillBase()->IsHarmful());

                    nHate += HateMod.second;
                    nHate *= static_cast<int32_t>(HateMod.first);

                    pUnit->As<Monster>()->AddHate(m_pSkill.m_pOwner->GetHandle(), m_fHateRatio * nHate, true, true);
                } /*
                else if (pUnit->IsNPC())
                    pUnit->As<NPC>()->SetAttacker(m_pSkill.m_pOwner);*/
            }
        }
        else if (pCaster->IsAlly(pUnit)) {
            nHPHeal = pUnit->Heal(nHPHeal);

            skill_result.type = TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP;
            skill_result.hTarget = pUnit->GetHandle();
            skill_result.hitAddHPMPSP.target_hp = pUnit->GetHealth();
            skill_result.hitAddHPMPSP.target_mp = pUnit->GetMana();
            skill_result.hitAddHPMPSP.nIncHP = nHPHeal;
            skill_result.hitAddHPMPSP.nIncMP = 0;
            skill_result.hitAddHPMPSP.nIncSP = 0;
            m_pSkill.m_vResultList.emplace_back(skill_result);
            ++m_pSkill.m_nTargetCount;
        }
    }
}

void SkillProp::FIRE_TRAP_DAMAGE(Unit *pCaster)
{
    float fFireRange = m_pSkill.GetVar(0) * 12.0f;
    float fDamageRange = m_pSkill.GetVar(5) * 12.0f;

    std::vector<Unit *> vTarget{};

    Skill::EnumSkillTargetsAndCalcDamage(GetPosition(), GetLayer(), GetPosition(), true, fFireRange, -1, 0, 0, true, pCaster, DISTRIBUTION_TYPE_NO_LIMIT, 0, vTarget, true);

    auto it = vTarget.begin();
    for (; it != vTarget.end(); ++it) {
        if (*it != nullptr && pCaster->IsEnemy(*it, true))
            break;
    }
    if (vTarget.empty() || it == vTarget.end())
        return;

    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();
    int32_t nDamage = m_pSkill.GetVar(1) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(2);

    auto t = sWorld.GetArTime();
    std::vector<Unit *> vTargetList{};

    nDamage = Skill::EnumSkillTargetsAndCalcDamage(
        GetPosition(), GetLayer(), GetPosition(), true, fDamageRange, -1, 0, nDamage, true, pCaster, DISTRIBUTION_TYPE_NO_LIMIT, m_pSkill.GetVar(6), vTargetList, true);

    for (auto &pDealTarget : vTargetList) {
        switch (m_pSkill.GetSkillBase()->GetSkillEffectType()) {
        case EF_TRAP_PHYSICAL_DAMAGE: {
            auto Damage = pDealTarget->DealPhysicalSkillDamage(pCaster, nDamage, (ElementalType)elemental_type,
                m_pSkill.GetSkillBase()->GetHitBonus(m_pSkill.GetSkillEnhance(), pCaster->GetLevel() - pDealTarget->GetLevel()),
                m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0);
            Skill::AddSkillDamageResult(m_pSkill.m_vResultList, SHT_DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
        } break;
        case EF_TRAP_MAGICAL_DAMAGE: {
            auto Damage = pDealTarget->DealMagicalSkillDamage(pCaster, nDamage, (ElementalType)elemental_type,
                m_pSkill.GetSkillBase()->GetHitBonus(m_pSkill.GetSkillEnhance(), pCaster->GetLevel() - pDealTarget->GetLevel()),
                m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0);
            Skill::AddSkillDamageResult(m_pSkill.m_vResultList, SHT_MAGIC_DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
        } break;
        default:
            break;
        }

        ++m_pSkill.m_nTargetCount;

        if (irand(0, 99) < m_pSkill.GetSkillBase()->GetProbabilityOnHit(m_pSkill.GetRequestedSkillLevel())) {
            int32_t nLevel = m_pSkill.GetSkillBase()->GetStateLevel(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
            uint32_t nDuration = m_pSkill.GetSkillBase()->GetStateSecond(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
            StateType nStateType = static_cast<StateType>(m_pSkill.GetSkillBase()->GetStateType());
            StateCode nStateCode = static_cast<StateCode>(m_pSkill.GetSkillBase()->GetStateId());
            pDealTarget->AddState(nStateType, nStateCode, pCaster->GetHandle(), nLevel, t, t + nDuration, false, 0, "");

            Skill::AddSkillResult(m_pSkill.m_vResultList, true, TS_SKILL_RESULT_SUCESS_TYPE::SRST_AddState, pDealTarget->GetHandle());
            ++m_pSkill.m_nTargetCount;
        }
    }
    m_Info.m_nEndTime = t;
}

void SkillProp::FIRE_TRAP_MULTIPLE_DAMAGE(Unit *pCaster)
{
    float fFireRange = m_pSkill.GetVar(0) * 12.0f;
    float fDamageRange = m_pSkill.GetVar(5) * 12.0f;

    if (!m_bFired) {
        std::vector<Unit *> vTarget{};

        Skill::EnumSkillTargetsAndCalcDamage(GetPosition(), GetLayer(), GetPosition(), true, fFireRange, -1, 0, 0, true, pCaster, DISTRIBUTION_TYPE_NO_LIMIT, 0, vTarget, true);

        std::vector<Unit *>::iterator it{};
        for (it = vTarget.begin(); it != vTarget.end(); ++it) {
            if ((*it) && pCaster->IsEnemy(*it, true))
                break;
        }
        if (vTarget.empty() || it == vTarget.end())
            return;

        auto t = sWorld.GetArTime();
        auto nFireEndTime = t + (m_pSkill.GetVar(7) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(8)) * 100;

        m_bFired = true;
        m_Info.m_nInterval = m_pSkill.GetVar(9) * 100;
        m_Info.m_nEndTime = nFireEndTime;
    }

    int32_t elemental_type = m_pSkill.GetSkillBase()->GetElementalType();
    int32_t nDamage = m_pSkill.GetVar(1) + m_pSkill.GetRequestedSkillLevel() * m_pSkill.GetVar(2);
    auto t = sWorld.GetArTime();

    std::vector<Unit *> vTargetList{};

    nDamage = Skill::EnumSkillTargetsAndCalcDamage(
        GetPosition(), GetLayer(), GetPosition(), true, fDamageRange, -1, 0, nDamage, true, pCaster, DISTRIBUTION_TYPE_DISTRIBUTE, m_pSkill.GetVar(6), vTargetList, true);

    for (auto &pDealTarget : vTargetList) {
        switch (m_pSkill.GetSkillBase()->GetSkillEffectType()) {
        case EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE: {
            auto Damage = pDealTarget->DealPhysicalSkillDamage(pCaster, nDamage, (ElementalType)elemental_type,
                m_pSkill.GetSkillBase()->GetHitBonus(m_pSkill.GetSkillEnhance(), pCaster->GetLevel() - pDealTarget->GetLevel()),
                m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0);
            Skill::AddSkillDamageResult(m_pSkill.m_vResultList, SHT_DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
        } break;
        case EF_TRAP_MULTIPLE_MAGICAL_DAMAGE: {
            auto Damage = pDealTarget->DealMagicalSkillDamage(pCaster, nDamage, (ElementalType)elemental_type,
                m_pSkill.GetSkillBase()->GetHitBonus(m_pSkill.GetSkillEnhance(), pCaster->GetLevel() - pDealTarget->GetLevel()),
                m_pSkill.GetSkillBase()->GetCriticalBonus(m_pSkill.GetRequestedSkillLevel()), 0);
            Skill::AddSkillDamageResult(m_pSkill.m_vResultList, SHT_MAGIC_DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
        } break;
        default:
            break;
        }

        ++m_pSkill.m_nTargetCount;

        if (irand(0, 99) < m_pSkill.GetSkillBase()->GetProbabilityOnHit(m_pSkill.GetRequestedSkillLevel())) {
            int32_t nLevel = m_pSkill.GetSkillBase()->GetStateLevel(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
            uint32_t nDuration = m_pSkill.GetSkillBase()->GetStateSecond(m_pSkill.GetRequestedSkillLevel(), m_pSkill.GetSkillEnhance());
            StateType nStateType = static_cast<StateType>(m_pSkill.GetSkillBase()->GetStateType());
            StateCode nStateCode = static_cast<StateCode>(m_pSkill.GetSkillBase()->GetStateId());
            pDealTarget->AddState(nStateType, nStateCode, pCaster->GetHandle(), nLevel, t, t + nDuration, false, 0, "");

            Skill::AddSkillResult(m_pSkill.m_vResultList, true, SRST_AddState, pDealTarget->GetHandle());
            ++m_pSkill.m_nTargetCount;
        }
    }
}