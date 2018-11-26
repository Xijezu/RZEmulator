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

#include "Skill.h"
#include "MemPool.h"
#include "ClientPackets.h"
#include "SkillFunctor.h"
#include "Messages.h"
#include "FieldPropManager.h"
#include "RegionContainer.h"
#include "GameContent.h"
#include "RegionTester.h"
#include "SkillProp/SkillProp.h"

constexpr int PREDICTION_AIMING_TIME = 200;

Skill::Skill(Unit *pOwner, int64 _uid, int _id) : m_nErrorCode(0), m_nAuraRefreshTime(0)
{
    m_nSkillUID   = _uid;
    m_nSkillID    = _id;
    m_pOwner      = pOwner;
    cool_time     = 0;
    m_nSummonID   = 0;
    m_fRange      = 0;
    m_nSkillLevel = 0;
    m_SkillBase   = sObjectMgr.GetSkillBase(m_nSkillID);

    int nEffectType = GetSkillBase() != nullptr ? GetSkillBase()->GetSkillEffectType() : -1;

    if (nEffectType == EF_PHYSICAL_MULTIPLE_DAMAGE_T3 ||
        nEffectType == EF_MAGIC_MULTIPLE_DAMAGE_T1_OLD ||
        nEffectType == EF_MAGIC_MULTIPLE_DAMAGE_T2_OLD ||
        nEffectType == EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD ||
        nEffectType == EF_MAGIC_MULTIPLE_REGION_DAMAGE_OLD ||
        nEffectType == EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK ||
        nEffectType == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD ||
        nEffectType == EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK_OLD ||
        nEffectType == EF_MAGIC_MULTIPLE_DAMAGE ||
        nEffectType == EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP ||
        nEffectType == EF_MAGIC_MULTIPLE_REGION_DAMAGE ||
        nEffectType == EF_PHYSICAL_SINGLE_DAMAGE_RUSH ||
        nEffectType == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK ||
        nEffectType == EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE ||
        nEffectType == EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK ||
        nEffectType == EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE)
    {
        m_bMultiple = true;
    }

    Init();
}

void Skill::Init()
{
    m_nErrorCode           = 0;
    m_Status               = SkillStatus::SS_IDLE;
    m_nCastTime            = 0;
    m_nCastingDelay        = 0;
    m_nFireTime            = 0;
    m_nRequestedSkillLevel = 0;
    m_hTarget              = 0;
    m_nCurrentFire         = 0;
    m_nTotalFire           = 0;
    m_nTargetCount         = 1;
    m_nFireCount           = 1;
    m_targetPosition.Relocate(0, 0, 0, 0);
    m_targetPosition.SetLayer(0);
}

int Skill::InitError(uint16_t nErrorCode)
{
    Init();
    return nErrorCode;
}

void Skill::SetRequestedSkillLevel(int nLevel)
{
    int tl = m_nSkillLevel + m_nSkillLevelAdd;
    if (nLevel <= tl)
        tl                 = nLevel;
    m_nRequestedSkillLevel = (uint8)tl;
}

void Skill::DB_InsertSkill(Unit *pUnit, int64 skillUID, int skill_id, int skill_level, int cool_time)
{
    auto              owner_uid = pUnit->GetUInt32Value(UNIT_FIELD_UID);
    PreparedStatement *stmt     = CharacterDatabase.GetPreparedStatement(CHARACTER_REP_SKILL);
    stmt->setInt64(0, skillUID);
    stmt->setInt32(1, pUnit->GetSubType() == ST_Player ? owner_uid : 0);
    stmt->setInt32(2, pUnit->GetSubType() == ST_Summon ? owner_uid : 0);
    stmt->setInt32(3, skill_id);
    stmt->setInt32(4, skill_level);
    stmt->setInt32(5, cool_time);
    CharacterDatabase.Execute(stmt);
}

void Skill::AddSkillDamageResult(std::vector<SkillResult> &pvList, uint8 type, uint8 damageType, DamageInfo damageInfo, uint handle)
{
    SkillResult skillResult{ };
    skillResult.type    = type;
    skillResult.hTarget = handle;

    skillResult.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(damageType);
    skillResult.hitDamage.damage.flag        = ATTACK_INFO__FLAG::AIF_None;

    if (damageInfo.bCritical)
        skillResult.hitDamage.damage.flag |= ATTACK_INFO__FLAG::AIF_Critical;
    if (damageInfo.bBlock)
        skillResult.hitDamage.damage.flag |= ATTACK_INFO__FLAG::AIF_Block;
    if (damageInfo.bMiss)
        skillResult.hitDamage.damage.flag |= ATTACK_INFO__FLAG::AIF_Miss;
    if (damageInfo.bPerfectBlock)
        skillResult.hitDamage.damage.flag |= ATTACK_INFO__FLAG::AIF_PerfectBlock;

    skillResult.hitDamage.damage.damage    = damageInfo.nDamage;
    skillResult.hitDamage.damage.target_hp = damageInfo.target_hp;
    std::copy(std::begin(damageInfo.elemental_damage),
              std::end(damageInfo.elemental_damage),
              std::begin(skillResult.hitDamage.damage.elemental_damage));

    pvList.emplace_back(skillResult);
}

void Skill::AddSkillResult(std::vector<SkillResult> &pvList, bool bIsSuccess, int nSuccessType, uint handle)
{
    SkillResult skillResult{ };
    skillResult.type                   = TS_SKILL__HIT_TYPE::SHT_RESULT;
    skillResult.hTarget                = handle;
    skillResult.hitResult.bResult      = bIsSuccess;
    skillResult.hitResult.success_type = static_cast<TS_SKILL_RESULT_SUCESS_TYPE>(nSuccessType);
    pvList.emplace_back(skillResult);
}

void Skill::AddSkillDamageWithKnockBackResult(std::vector<SkillResult> &pvList, uint8_t type, uint8_t damage_type, const DamageInfo &damage_info, uint32_t handle, float x, float y, uint32_t knock_back_time)
{
    SkillResult skillResult{ };

    skillResult.type    = type;
    skillResult.hTarget = handle;

    skillResult.hitDamageWithKnockBack.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE>(damage_type);
    skillResult.hitDamageWithKnockBack.damage.flag        = AIF_None;
    if (damage_info.bCritical)
        skillResult.hitDamageWithKnockBack.damage.flag |= AIF_Critical;
    if (damage_info.bBlock)
        skillResult.hitDamageWithKnockBack.damage.flag |= AIF_Block;
    if (damage_info.bMiss)
        skillResult.hitDamageWithKnockBack.damage.flag |= AIF_Miss;
    if (damage_info.bPerfectBlock)
        skillResult.hitDamageWithKnockBack.damage.flag |= AIF_PerfectBlock;

    skillResult.hitDamageWithKnockBack.damage.damage    = damage_info.nDamage;
    skillResult.hitDamageWithKnockBack.damage.target_hp = damage_info.target_hp;

    std::copy(std::begin(damage_info.elemental_damage),
              std::end(damage_info.elemental_damage),
              std::begin(skillResult.hitDamageWithKnockBack.damage.elemental_damage));

    skillResult.hitDamageWithKnockBack.x               = x;
    skillResult.hitDamageWithKnockBack.y               = y;
    skillResult.hitDamageWithKnockBack.knock_back_time = knock_back_time;
    skillResult.hitDamageWithKnockBack.speed           = -116; // @todo: Make sure to double check, this seems odd

    pvList.emplace_back(skillResult);
}

struct lessByDistantFromTarget : public std::binary_function<Unit *, Unit *, bool>
{
    explicit lessByDistantFromTarget(const Position *_pTarget) : pTarget(_pTarget) {}

    result_type operator()(first_argument_type a, second_argument_type b)
    {
        return pTarget->GetExactDist2d(a) < pTarget->GetExactDist2d(b);
    }

    const Position *pTarget;
};

int Skill::EnumSkillTargetsAndCalcDamage(const Position &_OriginalPos, uint8_t layer, const Position &_TargetPos, bool bTargetOrigin, const float fEffectLength, const int nRegionType, const float fRegionProperty, const int nOriginalDamage, const bool bIncludeOriginalPos, Unit *pCaster, const int nDistributeType, const int nTargetMax, /*out*/ std::vector<Unit *> &vTargetList, bool bEnemyOnly)
{
    int nResult = nOriginalDamage;

    const Position &OriginalPos = bTargetOrigin ? _TargetPos : _OriginalPos;
    const Position &TargetPos   = bTargetOrigin ? _OriginalPos : _TargetPos;

    std::vector<uint32_t> vList{ };

    {
        sWorld.EnumMovableObject(OriginalPos, layer, fEffectLength, vList, false, false);
    }

    vTargetList.clear();

    RegionTester *pTester{nullptr};

    ArcCircleRegionTester RegionTester_ArcCircle{ };
    DirectionRegionTester RegionTester_Direction{ };
    CrossRegionTester     RegionTester_Cross{ };
    CircleRegionTester    RegionTester_Circle{ };

    if (nRegionType == REGION_TYPE_ARC_CIRCLE)
        pTester = &RegionTester_ArcCircle;
    else if (nRegionType == REGION_TYPE_DIRECTION)
        pTester = &RegionTester_Direction;
    else if (nRegionType == REGION_TYPE_CROSS)
        pTester = &RegionTester_Cross;
    else
        pTester = &RegionTester_Circle;

    pTester->Init(OriginalPos, TargetPos, fRegionProperty);

    int nTargetCount = 0;
    int nAllyCount   = 0;

    for (auto &handle : vList)
    {
        bool bIsAlly = false;
        auto *pObj   = sMemoryPool.GetObjectInWorld<WorldObject>(handle);

        if (pObj == nullptr)
            continue;
        if (!pObj->IsUnit() /* IsPet() */)
            continue;

        Unit *pTarget = pObj->As<Unit>();

        if (pTarget->GetHealth() == 0)
            continue;

        if (!pCaster->IsEnemy(pTarget, true))
        {
            if (bEnemyOnly)
                continue;
            bIsAlly = true;
        }

        Position current_pos = pObj->GetCurrentPosition(sWorld.GetArTime());
        if (!pTester->IsInRegion(current_pos))
            continue;

        if (!bIncludeOriginalPos && (Position)OriginalPos == current_pos)
            continue;

        if (bIsAlly)
        {
            vTargetList.insert(vTargetList.begin(), pTarget);
            ++nAllyCount;
        }
        else
        {
            vTargetList.push_back(pTarget);
            ++nTargetCount;
        }
    }

    if (nDistributeType == DISTRIBUTION_TYPE_SEQUENTIAL_TARGET)
    {
        std::sort(vTargetList.begin() + nAllyCount, vTargetList.end(), lessByDistantFromTarget(&_TargetPos));
    }
    else if (nDistributeType == DISTRIBUTION_TYPE_SEQUENTIAL_CASTER)
    {
        std::sort(vTargetList.begin() + nAllyCount, vTargetList.end(), lessByDistantFromTarget(&_OriginalPos));
    }

    if (nDistributeType == DISTRIBUTION_TYPE_RANDOM ||
        nDistributeType == DISTRIBUTION_TYPE_SEQUENTIAL_TARGET ||
        nDistributeType == DISTRIBUTION_TYPE_SEQUENTIAL_CASTER)
    {
        if (nTargetCount > nTargetMax)
            vTargetList.resize(nAllyCount + nTargetMax);
    }

    if (nDistributeType == DISTRIBUTION_TYPE_DISTRIBUTE)
    {
        if (nTargetCount > nTargetMax)
            nResult = nResult * nTargetMax / nTargetCount;
    }

    return nResult;
}

int Skill::Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    m_vResultList.clear();
    auto current_time = sWorld.GetArTime();
    int  delay        = -1;
    m_Status = SS_CAST;

    SetRequestedSkillLevel(std::min(nSkillLevel, GetCurrentSkillLevel()));

    if (!CheckCoolTime(current_time))
    {
        return InitError(TS_RESULT_COOL_TIME);
    }

    if (GetSkillBase()->IsNeedWeapon())
    {
        if (m_SkillBase->IsUseableWeapon(ItemClass::CLASS_SHIELD))
        {
            if (!m_pOwner->IsWearShield())
                return InitError(TS_RESULT_LIMIT_WEAPON);
        }
        else if (!m_SkillBase->IsUseableWeapon(m_pOwner->GetWeaponClass()))
        {
            return InitError(TS_RESULT_LIMIT_WEAPON);
        }
    }

    // ************* TAMING CHECK ************* //
    if (GetSkillId() == SKILL_CREATURE_TAMING)
    {
        if (auto result = PrepareTaming(handle); result != TS_RESULT_SUCCESS)
            return result;
    }
        // ************* SUMMON CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_SUMMON)
    {
        if (auto result = PrepareSummon(handle, pos); result != TS_RESULT_SUCCESS)
            return result;
    }
        // ************* UNSUMMON CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_UNSUMMON || GetSkillBase()->GetSkillEffectType() == EF_UNSUMMON_AND_ADD_STATE)
    {
        if (!m_pOwner->IsPlayer() || m_pOwner->As<Player>()->GetMainSummon() == nullptr)
            return InitError(TS_RESULT_NOT_ACTABLE);
    }
        // ************* EF_MAGIC_SINGLE_DAMAGE_OR_DEATH CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_SINGLE_DAMAGE_OR_DEATH)
    {
        auto pMonster = sMemoryPool.GetObjectInWorld<Monster>(handle);
        if (pMonster == nullptr || !pMonster->IsMonster())
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (GetVar(8) != pMonster->GetCreatureGroup())
            return InitError(TS_RESULT_LIMIT_RACE);

        /* @TODO: Implement GetMonsterType()
        if(GetVar(9) < pMonster->GetMonsterType())
            return TS_RESULT_NOT_ACTABLE; */
    }
        // ************* EF_ADD_STATE_BY_TARGET_TYPE CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_TARGET_TYPE)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr)
            return InitError(TS_RESULT_NOT_ACTABLE);

        auto     nTargetCreatureGroup = static_cast<int32_t>(pUnit->GetCreatureGroup());
        bool     bIsInvalidTarget{true};
        for (int i                    = -1; i < 5; ++i)
        {
            if (nTargetCreatureGroup == GetVar(i + 7) || true /* @Todo: CREATURE_ALL */)
            {
                bIsInvalidTarget = false;
                break;
            }
        }
        if (bIsInvalidTarget)
            return InitError(TS_RESULT_LIMIT_RACE);
    }
        // ************* EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD)
    {
        if (!m_pOwner->IsWearShield())
            return InitError(TS_RESULT_LIMIT_WEAPON);
    }
        // ************* EF_PHYSICAL_SINGLE_DAMAGE CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD ||
             GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK ||
             GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_RUSH ||
             GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr)
            return InitError(TS_RESULT_NOT_ACTABLE);

        auto myPos     = m_pOwner->GetCurrentPosition(current_time);
        auto targetPos = pUnit->GetCurrentPosition(current_time);

        int32_t nMinDistance{0};
        switch (GetSkillBase()->GetSkillEffectType())
        {
            case EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD:
            case EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK:
                nMinDistance = GetVar(3) * 12;
                break;
            case EF_PHYSICAL_SINGLE_DAMAGE_RUSH:
            case EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK:
                nMinDistance = GetVar(4) * 12;
                break;
            default:
                nMinDistance = 0;
                break;
        }

        if (myPos.GetExactDist2d(&targetPos) < nMinDistance)
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (GameContent::CollisionToLine(myPos.GetPositionX(), myPos.GetPositionY(), targetPos.GetPositionX(), targetPos.GetPositionY()))
            return InitError(TS_RESULT_NOT_ACTABLE);
    }
        // ************* FIELD PROP CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_ACTIVATE_FIELD_PROP ||
             GetSkillBase()->GetSkillEffectType() == EF_REGION_HEAL_BY_FIELD_PROP ||
             GetSkillBase()->GetSkillEffectType() == EF_AREA_EFFECT_HEAL_BY_FIELD_PROP)
    {
        if (!m_pOwner->IsPlayer())
            return InitError(TS_RESULT_NOT_ACTABLE);

        auto pProp = sMemoryPool.GetObjectInWorld<FieldProp>(handle);
        if (pProp == nullptr || !pProp->IsFieldProp())
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (pProp->m_pFieldPropBase->nActivateSkillID != GetSkillId())
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (pProp->m_nUseCount < 1)
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (!pProp->IsUsable(m_pOwner->As<Player>()))
            return InitError(TS_RESULT_NOT_ACTABLE);

        delay = pProp->GetCastingDelay();
    }
        // ************* DISPEL CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE ||
             GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit == nullptr)
            return InitError(TS_RESULT_NOT_ACTABLE);

        bool     bCastable{false};
        for (int i = 0; i < 4; ++i)
        {
            if (pUnit->GetState(static_cast<StateCode>(GetVar(i))) != nullptr)
            {
                bCastable = true;
                break;
            }
        }

        if (!bCastable)
            return InitError(TS_RESULT_NOT_ACTABLE);
    }
        // ************* EF_RESPAWN_MONSTER_NEAR CHECK ************* //
    else if (GetSkillBase()->GetSkillEffectType() == EF_RESPAWN_MONSTER_NEAR)
    {
        if (!m_pOwner->IsMonster())
            return InitError(TS_RESULT_NOT_ACTABLE);
    }

    // ************* SKILL COST CALCULATION ************* //
    int nHP                  = m_pOwner->GetHealth();
    int nMP                  = m_pOwner->GetMana();
    int nEnergy              = m_pOwner->GetInt32Value(UNIT_FIELD_ENERGY);
    int nCurrentHPPercentage = m_pOwner->GetMaxHealth() > 0 ? nHP * 100 / m_pOwner->GetMaxHealth() : 0;
    int nCurrentMPPercentage = m_pOwner->GetMaxMana() > 0 ? nMP * 100 / m_pOwner->GetMaxMana() : 0;
    int mana_cost{0};

    if ((GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_AURA ||
         GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_DIFFERENTIAL_AURA) && m_pOwner->IsActiveAura(this))
        mana_cost = 0;
    else
        mana_cost = static_cast<int>((GetSkillBase()->GetCostMP(GetRequestedSkillLevel(), GetSkillEnhance()) + (GetSkillBase()->GetCostMPPercent(GetRequestedSkillLevel()) * m_pOwner->GetMana() / 100)) * m_pOwner->GetManaCostRatio((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful()));

    int nNeedHP = GetSkillBase()->GetNeedHP();
    if (nNeedHP > 0)
    {
        if (nCurrentHPPercentage < nNeedHP)
            return InitError(TS_RESULT_NOT_ENOUGH_HP);
    }
    else if (nNeedHP < 0)
    {
        if (nCurrentHPPercentage > -nNeedHP)
            return InitError(TS_RESULT_NOT_ACTABLE);
    }

    if (nCurrentMPPercentage < GetSkillBase()->GetNeedMP())
        return InitError(TS_RESULT_NOT_ENOUGH_MP);

    // @todo: havoc

    if (nHP - 1 < static_cast< int >( GetSkillBase()->GetCostHP(GetRequestedSkillLevel()) + (GetSkillBase()->GetCostHPPercent(GetRequestedSkillLevel()) * m_pOwner->GetMaxHealth() / 100)))
        return InitError(TS_RESULT_NOT_ENOUGH_HP);
    if (nMP < mana_cost)
        return InitError(TS_RESULT_NOT_ENOUGH_MP);

    // @todo: havoc
    if (nEnergy < static_cast<int>(GetSkillBase()->GetCostEnergy(GetRequestedSkillLevel())))
        return InitError(TS_RESULT_NOT_ENOUGH_ENERGY);

    if (m_pOwner->GetLevel() < GetSkillBase()->GetNeedLevel())
        return InitError(TS_RESULT_NOT_ENOUGH_LEVEL);
    if (m_pOwner->GetJP() < GetSkillBase()->GetCostJP(GetRequestedSkillLevel(), GetSkillEnhance()))
        return InitError(TS_RESULT_NOT_ENOUGH_JP);;
    if (int nCostEXP = GetSkillBase()->GetCostEXP(GetRequestedSkillLevel(), GetSkillEnhance()); nCostEXP > 0)
    {
        if (m_pOwner->GetEXP() < nCostEXP)
            return InitError(TS_RESULT_NOT_ENOUGH_EXP);
        /// @Todo: Gamecontent::GetNeedEXP
    }

    auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(handle);
    if (GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_SELF_COST || GetSkillBase()->GetSkillEffectType() == EF_ADD_REGION_STATE_BY_SELF_COST)
    {
        float fCostHP{ };
        float fCostSP{ };
        float fCostEnergy{ };
        float fCostMP{ };

        fCostHP     = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(2) * GetSkillEnhance();
        fCostSP     = GetVar(3) + GetVar(4) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();
        fCostEnergy = GetVar(6) + GetVar(7) * GetRequestedSkillLevel() + GetVar(8) * GetSkillEnhance();
        if (GetSkillBase()->GetSkillEffectType() == EF_ADD_STATE_BY_SELF_COST)
            fCostMP = GetVar(9) + GetVar(10) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

        if (pTarget == nullptr || !pTarget->IsSummon())
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (fCostHP && pTarget->GetHealth() < fCostHP)
            return InitError(TS_RESULT_NOT_ENOUGH_HP);

        /// @Todo: SP

        if (fCostEnergy > 0 && pTarget->GetUInt32Value(UNIT_FIELD_ENERGY) < fCostEnergy)
            return InitError(TS_RESULT_NOT_ENOUGH_ENERGY);
        else if (fCostEnergy == -1 && pTarget->GetUInt32Value(UNIT_FIELD_ENERGY) == 0)
            return InitError(TS_RESULT_NOT_ENOUGH_ENERGY);

        if (fCostMP && pTarget->GetMana() < fCostMP)
            return InitError(TS_RESULT_NOT_ENOUGH_MP);
    }

    if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD ||
        GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_SUMMON_DAMAGE ||
        GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_SUMMON_DEAD ||
        GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP ||
        GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP)
    {
        if (!m_pOwner->IsPlayer() || m_pOwner->As<Player>()->GetMainSummon() == nullptr)
            return InitError(TS_RESULT_NOT_ENOUGH_HP); // No idea why it's HP doe

        int decHP{ };
        int decMP{ };

        if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD)
            decHP = GetVar(6) + GetVar(7) * GetRequestedSkillLevel() + GetVar(8) * GetSkillEnhance();

        if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP)
            decHP = GetVar(9) + GetVar(10) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

        if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_SUMMON_DAMAGE)
            decHP = m_pOwner->As<Player>()->GetMainSummon()->GetMaxHealth() * GetVar(10);

        if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP)
        {
            decHP = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(4) * GetSkillEnhance();
            decMP = GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();
        }

        if (m_pOwner->As<Player>()->GetMainSummon()->GetHealth() < decHP + 1)
            return InitError(TS_RESULT_NOT_ENOUGH_HP);

        if (m_pOwner->As<Player>()->GetMainSummon()->GetMana() < decMP)
            return InitError(TS_RESULT_NOT_ENOUGH_MP);
    }

    Player *pPlayer{nullptr};
    if (m_pOwner->IsPlayer())
        pPlayer = m_pOwner->As<Player>();
    else if (m_pOwner->IsSummon())
        pPlayer = m_pOwner->As<Summon>()->GetMaster();

    if (pPlayer != nullptr)
    {
        /// @TODO Cost Item
    }

    if (GetSkillBase()->GetNeedStateId())
    {
        auto pState = m_pOwner->GetState(static_cast<StateCode>(GetSkillBase()->GetNeedStateId()));
        if (pState == nullptr || pState->GetLevel() < GetSkillBase()->GetNeedStateLevel())
            return InitError(TS_RESULT_NOT_ACTABLE);

        if (GetSkillBase()->NeedStateExhaust())
            m_pOwner->RemoveState(pState->m_nUID);
    }

    if (GetSkillBase()->GetSkillEffectType() == EF_ACTIVATE_FIELD_PROP ||
        GetSkillBase()->GetSkillEffectType() == EF_REGION_HEAL_BY_FIELD_PROP ||
        GetSkillBase()->GetSkillEffectType() == EF_AREA_EFFECT_HEAL_BY_FIELD_PROP)
    {
        auto pProp = sMemoryPool.GetObjectInWorld<FieldProp>(handle);
        if (pProp == nullptr || !pProp->IsFieldProp() || !sRegion.IsVisibleRegion(m_pOwner, pProp))
            return InitError(TS_RESULT_NOT_ACTABLE);

        pProp->Cast();
    }

    m_pOwner->SetMana(nMP - mana_cost);

    auto nOriginalCastingDelay = delay;
    if (delay == -1)
    {
        /// @Todo: based on SkillUID
        nOriginalCastingDelay = delay = m_SkillBase->GetCastDelay(GetRequestedSkillLevel(), GetSkillEnhance());
        delay /= (m_pOwner->GetCastingSpeed() / 100.0f);
        delay *= m_pOwner->GetCastingMod(static_cast<ElementalType>(GetSkillBase()->GetElementalType()), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful(), nOriginalCastingDelay);
    }

    if (GetSkillBase()->GetSkillEffectType() != EF_SUMMON)
        m_targetPosition = pos;

    m_hTarget       = handle;
    m_nCastTime     = current_time;
    m_nCastingDelay = static_cast<uint32_t>(nOriginalCastingDelay);
    m_nFireTime     = current_time + delay;

    broadcastSkillMessage(m_pOwner, 0, mana_cost, TS_SKILL__TYPE::ST_Casting);
    if (GetSkillBase()->IsHarmful() && !GetSkillBase()->IsPhysicalSkill())
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pUnit != nullptr && pUnit->IsMonster() && pUnit->As<Monster>()->IsCastRevenger())
        {
            pUnit->As<Monster>()->AddHate(m_pOwner->GetHandle(), 1, true, true);
        }
    }
    return TS_RESULT_SUCCESS;
}

uint16 Skill::PrepareSummon(uint handle, Position pos)
{
    if (!m_pOwner->IsPlayer() /* @todo: subsummon */)
        return TS_RESULT_NOT_ACTABLE;

    // @Todo: IsSummonable

    auto pItem = sMemoryPool.GetObjectInWorld<Item>(handle);
    if (pItem == nullptr || pItem->m_pItemBase == nullptr || pItem->m_pItemBase->group != GROUP_SUMMONCARD || pItem->m_Instance.OwnerHandle != m_pOwner->GetHandle())
        return TS_RESULT_NOT_ACTABLE;

    auto pPlayer = m_pOwner->As<Player>();
    if (pPlayer == nullptr)
        return TS_RESULT_NOT_ACTABLE;

    bool     bIsSummoncardBound{false};
    for (int j   = 0; j < 6; j++)
    {
        if (pPlayer->m_aBindSummonCard[j] != nullptr && pItem == pPlayer->m_aBindSummonCard[j])
        {
            bIsSummoncardBound = true;
            break;
        }
    }

    if (!bIsSummoncardBound)
        return TS_RESULT_NOT_ACTABLE;

    auto pSummon = pItem->GetSummon();
    if (pSummon == nullptr)
        return TS_RESULT_NOT_EXIST;

    if (pSummon->IsInWorld())
        return TS_RESULT_NOT_ACTABLE;

    /// @todo: Limit Dungeon Enterable Level

    Position tmpPos = pPlayer->GetCurrentPosition(sWorld.GetArTime());
    pSummon->SetCurrentXY(tmpPos.GetPositionX(), tmpPos.GetPositionY());
    pSummon->SetLayer(pPlayer->GetLayer());
    pSummon->StopMove();
    do
    {
        pSummon->AddNoise(rand32(), rand32(), 70);
        m_targetPosition = pSummon->GetCurrentPosition(sWorld.GetArTime());
    } while ((pos.GetPositionX() == m_targetPosition.GetPositionX() && pos.GetPositionY() == m_targetPosition.GetPositionY()) || tmpPos.GetExactDist2d(&m_targetPosition) < 24.0f);
    return TS_RESULT_SUCCESS;
}

void Skill::assembleMessage(TS_SC_SKILL &pSkillPct, int nType, int cost_hp, int cost_mp)
{
    pSkillPct.skill_id    = static_cast<uint16_t >(m_SkillBase->GetID());
    pSkillPct.skill_level = m_nRequestedSkillLevel;
    pSkillPct.caster      = m_pOwner->GetHandle();
    pSkillPct.target      = m_hTarget;
    pSkillPct.x           = m_targetPosition.GetPositionX();
    pSkillPct.y           = m_targetPosition.GetPositionY();
    pSkillPct.z           = m_targetPosition.GetPositionZ();
    pSkillPct.layer       = m_targetPosition.GetLayer();
    pSkillPct.type        = static_cast<TS_SKILL__TYPE >(nType);
    pSkillPct.hp_cost     = static_cast<decltype(pSkillPct.hp_cost)>(cost_hp);
    pSkillPct.mp_cost     = static_cast<decltype(pSkillPct.mp_cost)>(cost_mp);
    pSkillPct.caster_hp   = static_cast<decltype(pSkillPct.caster_hp)>(m_pOwner->GetHealth());
    pSkillPct.caster_mp   = static_cast<decltype(pSkillPct.caster_mp)>(m_pOwner->GetMana());

    switch (pSkillPct.type)
    {
        case TS_SKILL__TYPE::ST_Casting:
        case TS_SKILL__TYPE::ST_CastingUpdate:
        case TS_SKILL__TYPE::ST_Complete:
        {
            pSkillPct.casting.tm         = static_cast<uint32_t>(m_nFireTime - m_nCastTime);
            pSkillPct.casting.nErrorCode = m_nErrorCode;
            return;
        }
        case TS_SKILL__TYPE::ST_Cancel:
            return;
        default:
            break;
    }

    pSkillPct.fire.bMultiple    = m_bMultiple;
    pSkillPct.fire.range        = m_fRange;
    pSkillPct.fire.target_count = static_cast<int8_t >(m_nTargetCount);
    pSkillPct.fire.fire_count   = static_cast<int8_t>(m_nFireCount);

    for (const auto &skill_result : m_vResultList)
    {
        TS_SC_SKILL__HIT_DETAILS details{ };
        details.type    = static_cast<TS_SKILL__HIT_TYPE >(skill_result.type);
        details.hTarget = skill_result.hTarget;
        switch (details.type)
        {
            case SHT_DAMAGE:
            case SHT_MAGIC_DAMAGE:
                details.hitDamage = skill_result.hitDamage;
                break;
            case SHT_DAMAGE_WITH_KNOCK_BACK:
                details.hitDamageWithKnockBack = skill_result.hitDamageWithKnockBack;
                break;
            case SHT_RESULT:
                details.hitResult = skill_result.hitResult;
                break;
            case SHT_ADD_HP:
            case SHT_ADD_MP:
                details.hitAddStat = skill_result.hitAddStat;
                break;
            case SHT_ADD_HP_MP_SP:
                details.hitAddHPMPSP = skill_result.hitAddHPMPSP;
                break;
            case SHT_REBIRTH:
                details.hitRebirth = skill_result.hitRebirth;
                break;
            case SHT_RUSH:
                details.hitRush = skill_result.hitRush;
                break;
            default:
                break;
        }
        pSkillPct.fire.hits.emplace_back(details);
    }
}

void Skill::broadcastSkillMessage(WorldObject *pUnit, int cost_hp, int cost_mp, int nType)
{
    if (pUnit == nullptr)
        return;

    auto        rx    = (uint)(pUnit->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE));
    auto        ry    = (uint)(pUnit->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE));
    uint8       layer = pUnit->GetLayer();
    TS_SC_SKILL skillPct{ };
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld.Broadcast((uint)rx, (uint)ry, layer, skillPct);
}

void Skill::broadcastSkillMessage(Unit *pUnit1, Unit *pUnit2, int cost_hp, int cost_mp, int nType)
{
    if (pUnit1 == nullptr || pUnit2 == nullptr)
        return;

    TS_SC_SKILL skillPct{ };
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld.Broadcast((uint)(pUnit1->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(pUnit1->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                     (uint)(pUnit2->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(pUnit2->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                     pUnit1->GetLayer(), skillPct);
}

void Skill::ProcSkill()
{
    if (m_Status == SS_IDLE)
        return;

    if (m_pOwner->GetHealth() == 0)
    {
        m_pOwner->CancelSkill();
        return;
    }

    auto t = sWorld.GetArTime();

    auto pRawTarget = sMemoryPool.GetObjectInWorld<WorldObject>(m_hTarget);
    Unit *pTarget{nullptr};
    if (m_Status != SS_COMPLETE && pRawTarget != nullptr && pRawTarget->IsUnit())
    {
        pTarget         = pRawTarget->As<Unit>();
        if (auto tmpPos = m_pOwner->GetPosition(); pRawTarget->GetPosition().GetExactDist2d(&tmpPos) > 525.0f)
        {
            m_pOwner->CancelSkill();
            return;
        }

        if (pTarget->GetHealth() == 0 && !GetSkillBase()->IsValidToCorpse())
        {
            m_pOwner->CancelSkill();
            return;
        }

        if (pTarget->GetHealth() != 0 && GetSkillBase()->IsValidToCorpse())
        {
            m_pOwner->CancelSkill();
            return;
        }
    }

    if (m_hTarget != 0 && pRawTarget == nullptr)
    {
        m_pOwner->CancelSkill();
        return;
    }

    if (t < m_nFireTime)
        return;

    if (m_Status == SS_CAST)
    {
        int nElementalType = GetSkillBase()->GetElementalType();
        /// @Todo:
        /// m_pOwner->PrepareRemoveExhaustiveSkillStateMod( GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful(), nElementalType, GetOriginalCastingDelay() );
        m_Status = SS_FIRE;
    }
    /// Retail locks here, please don't mind this here
    if (pRawTarget != nullptr && pRawTarget->IsInWorld())
    {
        if (!pRawTarget->IsInWorld())
        {
            m_pOwner->CancelSkill();
            m_hTarget = 0;
            broadcastSkillMessage(m_pOwner, 0, 0, TS_SKILL__TYPE::ST_Cancel);
            return;
        }
    }
    else if (GetSkillBase()->GetSkillEffectType() == EF_UNSUMMON || GetSkillBase()->GetSkillEffectType() == EF_UNSUMMON_AND_ADD_STATE)
    {
        if (!m_pOwner->IsPlayer())
        {
            m_pOwner->CancelSkill();
            return;
        }

        auto pMainSummon = m_pOwner->As<Player>()->GetMainSummon();
        if (pMainSummon == nullptr || !pMainSummon->IsInWorld())
        {
            m_pOwner->CancelSkill();
            return;
        }
        /// More locks here in retail
    }

    if (pTarget != nullptr && !pTarget->IsInWorld())
    {
        m_pOwner->CancelSkill();
        m_hTarget = 0;
        broadcastSkillMessage(m_pOwner, 0, 0, TS_SKILL__TYPE::ST_Cancel);
        return;
    }

    if ((GetSkillBase()->GetSkillEffectType() == EF_ACTIVATE_FIELD_PROP ||
         GetSkillBase()->GetSkillEffectType() == EF_REGION_HEAL_BY_FIELD_PROP ||
         GetSkillBase()->GetSkillEffectType() == EF_AREA_EFFECT_HEAL_BY_FIELD_PROP) &&
        (pRawTarget != nullptr || !pRawTarget->IsInWorld()))
    {
        m_pOwner->CancelSkill();
        m_hTarget = 0;
        broadcastSkillMessage(m_pOwner, 0, 0, TS_SKILL__TYPE::ST_Cancel);
        return;
    }

    bool bSuccess{true};
    if (m_Status == SS_FIRE)
    {
        int cost_hp{ };
        int cost_mp{ };
        int cost_exp{ };
        int cost_jp{ };

        if (!m_bMultiple || m_nCurrentFire == 0)
        {
            auto next_cool_time = GetSkillCoolTime();
            SetRemainCoolTime(next_cool_time);

            /// @todo: Set cooltime group

            Player *pClient{nullptr};
            if (m_pOwner->IsPlayer())
                pClient = m_pOwner->As<Player>();
            else if (m_pOwner->IsSummon())
                pClient = m_pOwner->As<Summon>()->GetMaster();

            if (pClient != nullptr)
                Messages::SendSkillList(pClient, m_pOwner, GetSkillId());

            if ((GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_AURA || GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_DIFFERENTIAL_AURA) && m_pOwner->IsActiveAura(this))
                cost_mp = 0;
            else
                cost_mp = static_cast< int >((GetSkillBase()->GetCostMP(GetRequestedSkillLevel(), GetSkillEnhance()) + (GetSkillBase()->GetCostMPPercent(GetRequestedSkillLevel()) * m_pOwner->GetMaxMana() / 100)) * m_pOwner->GetManaCostRatio((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful()));

            cost_hp  = static_cast< int >( GetSkillBase()->GetCostHP(GetRequestedSkillLevel()) + (GetSkillBase()->GetCostHPPercent(GetRequestedSkillLevel()) * m_pOwner->GetMaxHealth() / 100));
            cost_exp = GetSkillBase()->GetCostEXP(GetRequestedSkillLevel(), GetSkillEnhance());
            cost_jp  = GetSkillBase()->GetCostJP(GetRequestedSkillLevel(), GetSkillEnhance());

            if (cost_hp > 0)
                m_pOwner->SetHealth(m_pOwner->GetHealth() - cost_hp);
            if (cost_mp > 0)
                m_pOwner->SetMana(m_pOwner->GetMana() - cost_mp);
            if (cost_exp > 0)
                m_pOwner->SetEXP(m_pOwner->GetEXP() - cost_exp);
            if (cost_jp > 0)
                m_pOwner->SetJP(m_pOwner->GetJP() - cost_jp);

            m_pOwner->RemoveEnergy(GetSkillBase()->GetCostEnergy(GetRequestedSkillLevel()));

            if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD)
            {
                int decHP = GetVar(6) + GetVar(7) * GetRequestedSkillLevel() + GetVar(8) * GetSkillEnhance();

                if (m_pOwner->IsPlayer() ||
                    m_pOwner->As<Player>()->GetMainSummon() != nullptr)
                {
                    auto pSummon = m_pOwner->As<Player>()->GetMainSummon();

                    int nPrevHP = pSummon->GetHealth();
                    pSummon->AddHealth(0 - std::min(pSummon->GetHealth() - 1, decHP));

                    Messages::BroadcastHPMPMessage(pSummon, pSummon->GetHealth() - nPrevHP, 0, true);
                }
            }
            else if (GetSkillBase()->GetSkillEffectType() == EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP)
            {
                int decHP = GetVar(9) + GetVar(10) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

                if (m_pOwner->IsPlayer() ||
                    m_pOwner->As<Player>()->GetMainSummon() != nullptr)
                {
                    auto pSummon = m_pOwner->As<Player>()->GetMainSummon();

                    int nPrevHP = pSummon->GetHealth();
                    pSummon->AddHealth(0 - std::min(pSummon->GetHealth() - 1, decHP));

                    Messages::BroadcastHPMPMessage(pSummon, pSummon->GetHealth() - nPrevHP, 0, true);
                }
            }
            /// @Todo: Item Cost
        }

        FireSkill(pTarget, bSuccess);

        if (pTarget != nullptr)
            broadcastSkillMessage(m_pOwner, pTarget, cost_hp, cost_mp, TS_SKILL__TYPE::ST_Fire);
        else
            broadcastSkillMessage(m_pOwner, cost_hp, cost_mp, TS_SKILL__TYPE::ST_Fire);

        if (!m_bMultiple || m_nCurrentFire == m_nTotalFire)
        {
            m_nFireTime += GetSkillBase()->GetCommonDelay();
            m_Status = SS_COMPLETE;

            /// @todo: SetCurrentEndurance, gear
        }
    }

    if (m_Status == SS_COMPLETE)
    {
        if (sWorld.GetArTime() < m_nFireTime)
            return;

        if (pTarget != nullptr)
            broadcastSkillMessage(m_pOwner, pTarget, 0, 0, TS_SKILL__TYPE::ST_Complete);
        else
            broadcastSkillMessage(m_pOwner, 0, 0, TS_SKILL__TYPE::ST_Complete);

        m_pOwner->OnCompleteSkill();
        Init();
    }
}

void Skill::FireSkill(Unit *pTarget, bool &bIsSuccess)
{
    auto t = m_nFireTime;
    m_vResultList.clear();

    /// @TODO: IsHiding

    bool bHandled{true};

    switch (GetSkillBase()->GetSkillEffectType())
    {
        case EF_ADD_STATE:
        case EF_ADD_STATE_BY_TARGET_TYPE:
        {
            StateSkillFunctor mySkillFunctor{&m_vResultList};
            process_target(t, mySkillFunctor, pTarget);
            switch (GetSkillId())
            {
                case SKILL_ITEM_PIECE_OF_STRENGTH:
                case SKILL_ITEM_PIECE_OF_VITALITY:
                case SKILL_ITEM_PIECE_OF_DEXTERITY:
                case SKILL_ITEM_PIECE_OF_AGILITY:
                case SKILL_ITEM_PIECE_OF_INTELLIGENCE:
                case SKILL_ITEM_PIECE_OF_MENTALITY:
                    if (pTarget != nullptr && pTarget->IsPlayer())
                        pTarget->As<Player>()->Save(true);
                default:
                    break;
            }

            break;
        }

        case EF_ADD_REGION_STATE:
        {
            ADD_REGION_STATE(pTarget);
            break;
        }
        case EF_ADD_STATE_BY_SELF_COST:
        {
            ADD_STATE_BY_SELF_COST(pTarget);
            break;
        }
        case EF_ADD_REGION_STATE_BY_SELF_COST:
        {
            ADD_REGION_STATE_BY_SELF_COST(pTarget);
            break;
        }
        case EF_PHYSICAL_DIRECTIONAL_DAMAGE:
        {
            PHYSICAL_DIRECTIONAL_DAMAGE(pTarget);
            break;
        }

        case EF_PHYSICAL_SINGLE_DAMAGE_T1:
        {
            SINGLE_PHYSICAL_DAMAGE_T1(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_T2:
        {
            SINGLE_PHYSICAL_DAMAGE_T2(pTarget);
            break;
        }/*
        case EF_PHYSICAL_SINGLE_DAMAGE_T3:
        {
            SINGLE_PHYSICAL_DAMAGE_T3(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_DAMAGE_T1:
        {
            MULTIPLE_PHYSICAL_DAMAGE_T1(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_DAMAGE_T2:
        {
            MULTIPLE_PHYSICAL_DAMAGE_T2(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_DAMAGE_T3:
        {
            MULTIPLE_PHYSICAL_DAMAGE_T3(pTarget);
            break;
        }*/
        case EF_PHYSICAL_ABSORB_DAMAGE:
        {
            SINGLE_PHYSICAL_DAMAGE_ABSORB(pTarget);
            break;
        }/*
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_OLD:
        {
            PHYSICAL_SINGLE_REGION_DAMAGE_OLD(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD:
        {
            PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_OLD:
        {
            PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE_OLD:
        {
            PHYSICAL_SPECIAL_REGION_DAMAGE(pTarget);
            break;
        }*/
        case EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD:
        {
            SINGLE_PHYSICAL_DAMAGE_T1(pTarget);
            break;
        }/*
        case EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY_OLD:
        {
            SINGLE_PHYSICAL_DAMAGE_T2_ADD_ENERGY(pTarget);
        }
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_OLD:
        {
            PHYSICAL_SINGLE_REGION_DAMAGE_OLD(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_WITHOUT_WEAPON_RUSH_KNOCK_BACK:
        {
            SINGLE_PHYSICAL_DAMAGE_T3(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD:
        {
            SINGLE_PHYSICAL_DAMAGE_T1(pTarget);
            break;
        }
        case EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK_OLD:
        {
            MULTIPLE_PHYSICAL_DAMAGE_T4(pTarget);
            break;
        }*/
        case EF_REMOVE_BAD_STATE:
        {
            RemoveBadStateSkillFunctor mySkillFunctor{&m_vResultList};
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_REMOVE_GOOD_STATE:
        {
            RemoveGoodStateSkillFunctor mySkillFunctor{&m_vResultList};
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_AREA_EFFECT_MAGIC_DAMAGE_OLD:
        case EF_AREA_EFFECT_MAGIC_DAMAGE:
        case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL:
        case EF_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2:
        case EF_AREA_EFFECT_HEAL:
        {
            MAKE_AREA_EFFECT_PROP(pTarget, false);
            break;
        }
        case EF_TRAP_PHYSICAL_DAMAGE:
        case EF_TRAP_MAGICAL_DAMAGE:
        case EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE:
        case EF_TRAP_MULTIPLE_MAGICAL_DAMAGE:
        {
            MAKE_AREA_EFFECT_PROP(pTarget, true);
            break;
        }/*
        case EF_CREATE_ITEM:
        {
            CREATE_ITEM(pTarget, bIsSuccess);
            break;
        }*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case EF_ACTIVATE_FIELD_PROP:
        {
            ACTIVATE_FIELD_PROP();
            break;
        }
            /*
             case EF_REGION_HEAL_BY_FIELD_PROP:
             {
                 REGION_HEAL_BY_FIELD_PROP();
                 break;
             }*/
        case EF_AREA_EFFECT_HEAL_BY_FIELD_PROP:
        {
            MAKE_AREA_EFFECT_PROP_BY_FIELD_PROP(false);
            break;
        }
        case EF_ADD_HP:
        {
            HealingSkillFunctor mySkillFunctor(&m_vResultList);
            process_target(t, mySkillFunctor, pTarget);
            break;
        }

        case EF_ADD_MP:
        {
            RecoveryMPSkillFunctor mySkillFunctor{&m_vResultList};
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_ADD_HP_BY_ITEM:
        {
            HealingSkillFunctor mySkillFunctor(&m_vResultList, true);
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_ADD_MP_BY_ITEM:
        {
            RecoveryMPSkillFunctor mySkillFunctor{&m_vResultList, true};
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_RESURRECTION:
        {
            SKILL_RESURRECTION(pTarget);
            break;
        }
        case EF_ADD_HP_MP:
        case EF_ADD_HP_MP_BY_SUMMON_DAMAGE:
        case EF_ADD_HP_MP_BY_SUMMON_DEAD:
        case EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP:
        case EF_ADD_HP_MP_WITH_LIMIT_PERCENT:
        {
            SKILL_ADD_HP_MP(pTarget);
            break;
        }
            /*
            case EF_ADD_REGION_HP_MP:
            {
                SKILL_ADD_REGION_HP_MP( pTarget );
                break;
            }
            case EF_ADD_REGION_HP:
            {
                SKILL_ADD_REGION_HP( pTarget );
                break;
            }
            case EF_ADD_REGION_MP:
            {
                SKILL_ADD_REGION_MP( pTarget );
                break;
            }
            case EF_MAGIC_SINGLE_DAMAGE_T1_OLD:
            {
                if( !GetSkillBase()->GetSkillTargetType() == TARGET_TARGET )
                    break;

                SINGLE_MAGICAL_DAMAGE_T1( pTarget );
                break;
            }
            case EF_MAGIC_SINGLE_DAMAGE_T2_OLD:
            {
                if( !GetSkillBase()->GetSkillTargetType() == TARGET_TARGET )
                    break;

                SINGLE_MAGICAL_DAMAGE_T2( pTarget );
                break;
            }*/
        case EF_MAGIC_SINGLE_DAMAGE:
        case EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE:
        {
            if (!GetSkillBase()->GetSkillTargetType() == TARGET_TARGET)
                break;

            SINGLE_MAGICAL_DAMAGE(pTarget);
            break;
        }
            /*
            case EF_MAGIC_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE:
            {
                SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE( pTarget );
                break;
            }

            case EF_MAGIC_MULTIPLE_DAMAGE_T1_OLD:
            case EF_MAGIC_MULTIPLE_DAMAGE_T1_DEAL_SUMMON_HP_OLD:
            {
                if( !GetSkillBase()->GetSkillTargetType() == TARGET_TARGET )
                    break;

                MULTIPLE_MAGICAL_DAMAGE_T1( pTarget );
                break;
            }
            case EF_MAGIC_MULTIPLE_DAMAGE_AT_ONCE:
            {
                if( !GetSkillBase()->GetSkillTargetType() == TARGET_TARGET )
                    break;

                MULTIPLE_MAGICAL_DAMAGE_AT_ONCE( pTarget );
                break;
            }*/
        case EF_MAGIC_MULTIPLE_DAMAGE:
        case EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP:
        {
            if (!GetSkillBase()->GetSkillTargetType() == TARGET_TARGET)
                break;

            MULTIPLE_MAGICAL_DAMAGE(pTarget);
            break;
        }/*
        case EF_MAGIC_SINGLE_DAMAGE_OR_DEATH:
        {
            if( !GetSkillBase()->GetSkillTargetType() == TARGET_TARGET )
                break;

            SINGLE_MAGICAL_DAMAGE_OR_DEATH( pTarget );
            break;
        }*/
        case EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP:
        {
            SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(pTarget);
            break;
        }
            /*
        case EF_ADD_HP_MP_BY_ABSORB_HP_MP:
        {
            ADD_HP_MP_BY_ABSORB_HP_MP(pTarget);
            break;
        }

        case EF_MAGIC_SINGLE_PERCENT_DAMAGE:
        {
            SINGLE_MAGICAL_TARGET_HP_PERCENT_DAMAGE(pTarget);
            break;
        }

        case EF_MAGIC_SINGLE_PERCENT_MANABURN:
        case EF_MAGIC_SINGLE_PERCENT_OF_MAX_MP_MANABURN:
        {
            SINGLE_MAGICAL_MANABURN(pTarget);
            break;
        }

        case EF_MAGIC_MULTIPLE_DAMAGE_T2_OLD:
        {
            if (!GetSkillBase()->GetSkillTargetType() == TARGET_TARGET)
                break;

            MULTIPLE_MAGICAL_DAMAGE_T2(pTarget);
            break;
        }
        case EF_MAGIC_MULTIPLE_DAMAGE_T3_OLD:
        {
            MULTIPLE_MAGICAL_DAMAGE_T3(pTarget);
            break;
        }
        case EF_MAGIC_SINGLE_REGION_DAMAGE_OLD:
        {
            MAGIC_SINGLE_REGION_DAMAGE_OLD(pTarget);
            break;
        }*/
        case EF_MAGIC_SINGLE_REGION_DAMAGE:
        case EF_MAGIC_SINGLE_REGION_DAMAGE_USING_CORPSE:
        case EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE:
        {
            MAGIC_SINGLE_REGION_DAMAGE(pTarget);
            break;
        }/*
        case EF_MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD:
        {
            MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD(pTarget);
            break;
        }
        case EF_REGION_TAUNT:
        {
            REGION_TAUNT(pTarget);
            break;
        }*/
        case EF_TAUNT:
        {
            TAUNT(pTarget);
            break;
        }/*
        case EF_REMOVE_HATE:
        {
            REMOVE_HATE(pTarget);
            break;
        }
        case EF_REGION_REMOVE_HATE:
        {
            REGION_REMOVE_HATE(pTarget);
            break;
        }
        case EF_MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE:
        {
            MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE(pTarget);
            break;
        }
        case EF_MAGIC_MULTIPLE_REGION_DAMAGE_OLD:
        {
            MAGIC_MULTIPLE_REGION_DAMAGE_OLD(pTarget);
            break;
        }
        case EF_MAGIC_MULTIPLE_REGION_DAMAGE_T2_OLD:
        {
            MAGIC_MULTIPLE_REGION_DAMAGE_T2(pTarget);
            break;
        }
        case EF_MAGIC_SPECIAL_REGION_DAMAGE_OLD:
        {
            MAGIC_SPECIAL_REGION_DAMAGE_OLD(pTarget);
            break;
        }
        case EF_MAGIC_SPECIAL_REGION_DAMAGE:
        {
            MAGIC_SPECIAL_REGION_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD:
        {
            SINGLE_PHYSICAL_DAMAGE_WITH_SHIELD(pTarget);
            break;
        }*/
        case EF_MAGIC_MULTIPLE_REGION_DAMAGE:
        {
            MAGIC_MULTIPLE_REGION_DAMAGE(pTarget);
            break;
        }/*
        case EF_MAGIC_REGION_PERCENT_DAMAGE:
        {
            MAGIC_SINGLE_REGION_PERCENT_DAMAGE(pTarget);
            break;
        }
        case EF_MAGIC_ABSORB_DAMAGE_OLD:
        {
            MAGIC_ABSORB_DAMAGE(pTarget);
            break;
        }*/
        case EF_TOGGLE_AURA:
        case EF_TOGGLE_DIFFERENTIAL_AURA:
        {
            TOGGLE_AURA(pTarget);
            break;
        }
        case EF_SUMMON:
        {
            if (m_pOwner->IsPlayer())
                DO_SUMMON();
            break;
        }
        case EF_UNSUMMON:
        {
            if (m_pOwner->IsPlayer())
                DO_UNSUMMON();
            break;
        }/*
        case EF_UNSUMMON_AND_ADD_STATE:
        {
            if (m_pOwner->IsPlayer())
                UnSummonAndAddState();
            break;
        }
        case EF_CORPSE_ABSORB:
        {
            CORPSE_ABSORB(pTarget);
            break;
        }
        case EF_CORPSE_EXPLOSION:
        {
            CORPSE_EXPLOSION(pTarget);
            break;
        }*/
        case EF_PHYSICAL_SINGLE_DAMAGE:
        case EF_PHYSICAL_SINGLE_DAMAGE_RUSH:
        case EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK:
        case EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK:
        {
            PHYSICAL_SINGLE_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_ABSORB:
        {
            PHYSICAL_SINGLE_DAMAGE_ABSORB(pTarget);
            break;
        }/*
        case EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY:
        {
            PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY(pTarget);
            break;
        }*/
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE:
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK:
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF:
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL:
        {
            PHYSICAL_SINGLE_REGION_DAMAGE(pTarget);
            break;
        }/*
        case EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE:
        case EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK:
        {
            PHYSICAL_REALTIME_MULTIPLE_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE:
        {
            PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE:
        {
            SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE(pTarget);
            break;
        }*/
        case EF_PHYSICAL_MULTIPLE_DAMAGE:
        {
            PHYSICAL_MULTIPLE_DAMAGE(pTarget);
            break;
        }/*
        case EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK:
        {
            PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK(pTarget);
            break;
        }*/
        case EF_PHYSICAL_MULTIPLE_REGION_DAMAGE:
        case EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE:
        case EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF:
        {
            PHYSICAL_MULTIPLE_REGION_DAMAGE(pTarget);
            break;
        }
        case EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE:
        {
            PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE(pTarget);
            break;
        }/*
        case EF_RESURRECTION_WITH_RECOVER:
        {
            SKILL_RESURRECTION_WITH_RECOVER(pTarget);
            break;
        }
        case EF_REMOVE_STATE_GROUP:
        {
            REMOVE_STATE_GROUP_SKILL_FUNCTOR mySkillFunctor(&m_vResultList, m_pOwner);
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
        case EF_CASTING_CANCEL_WITH_ADD_STATE:
        {
            CASTING_CANCEL_WITH_ADD_STATE(pTarget);
            break;
        }
        case EF_RESPAWN_MONSTER_NEAR:
        {
            RESPAWN_NEAR_MONSTER();
            break;
        }*/
        default:
        {
            switch (GetSkillId())
            {
                case SKILL_TOWN_PORTAL:
                case SKILL_RETURN:
                    TOWN_PORTAL();
                    break;
                    /*case SKILL_RANKED_DEATHMATCH_ENTER:
                    case SKILL_FREED_DEATHMATCH_ENTER:
                        INSTANCE_GAME_ENTER();
                        break;
                    case SKILL_WARP_TO_HUNTAHOLIC_LOBBY:
                        WARP_TO_HUNTAHOLIC_LOBBY();
                        break;
                    case SKILL_INSTANCE_GAME_EXIT:
                        INSTANCE_GAME_EXIT();
                        break;
                    case SKILL_RETURN_FEATHER:
                        RETURN_FEATHER();
                        break;
                    case SKILL_RETURN_BACK_FEATHER:
                        RETURN_BACK_FEATHER();
                        break;*/
                case SKILL_CREATURE_TAMING:
                    CREATURE_TAMING();
                    break;
                    /*case SKILL_PET_TAMING:
                        PET_TAMING(pTarget);
                        break;
                    case SKILL_SHOVELING:
                        SHOVELING();
                        break;*/
                case SKILL_GAIA_FORCE_SAVING:
                    ADD_ENERGY();
                    break;
                default:
                    bHandled = false;
                    break;
            }
        }
            break;
    }

    if (!bHandled)
    {
        auto result = string_format("Unknown skill casted - ID %u, effect_type %u", m_SkillBase->id, m_SkillBase->effect_type);
        NG_LOG_INFO("skill", "%s", result.c_str());
        if (m_pOwner->IsPlayer())
            Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Player>(), result);
        else if (m_pOwner->IsSummon())
        {
            Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Summon>()->GetMaster(), result);
        }
    }
    else
        PostFireSkill(pTarget);
}

void Skill::PostFireSkill(Unit *pTarget)
{
    auto                t = m_nFireTime;
    std::vector<Unit *> vNeedStateList{ };
    int                 pt{0};
    for (const auto     &sr : m_vResultList)
    {
        if (sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_MAGIC_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE_WITH_KNOCK_BACK
            || sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_HP || sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_MP || sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP || sr.type == TS_SKILL__HIT_TYPE::SHT_RESULT)
        {
            auto pDealTarget = sMemoryPool.GetObjectInWorld<Unit>(sr.hTarget);
            if (pDealTarget != nullptr && pDealTarget->GetHealth() != 0)
            {
                if (GetSkillBase()->GetSkillEffectType() != EF_ADD_STATE &&
                    GetSkillBase()->GetSkillEffectType() != EF_ADD_REGION_STATE &&
                    GetSkillBase()->GetSkillEffectType() != EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD &&
                    GetSkillBase()->GetSkillEffectType() != EF_ADD_STATE_BY_SELF_COST &&
                    GetSkillBase()->GetSkillEffectType() != EF_ADD_REGION_STATE_BY_SELF_COST &&
                    GetSkillBase()->GetSkillEffectType() != EF_REMOVE_STATE_GROUP &&
                    GetSkillBase()->GetStateId() != 0)
                {
                    if (sr.hitDamage.damage.flag ^ AIF_Miss)
                    {
                        vNeedStateList.emplace_back(pDealTarget);
                    }
                }

                if (!m_pOwner->IsMonster() && GetSkillBase()->GetSkillEffectType() != EF_REMOVE_HATE && GetSkillBase()->GetSkillEffectType() != EF_REGION_REMOVE_HATE)
                {
                    pt     = 0;
                    if (sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_MAGIC_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE_WITH_KNOCK_BACK)
                        pt = sr.hitDamage.damage.damage;
                    else if (sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_HP || sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_MP)
                        pt = sr.hitAddStat.nIncStat;
                    else if (sr.type == TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP)
                    {
                        pt = sr.hitAddHPMPSP.nIncHP;
                        pt += sr.hitAddHPMPSP.nIncMP;
                        pt += sr.hitAddHPMPSP.nIncSP;
                    }
                    /// @Todo: HateRatio
                    int  nAddHate   = 1 * GetSkillBase()->GetHatePoint(GetRequestedSkillLevel(), pt, GetSkillEnhance()) * (int)m_pOwner->GetMagicalHateMod((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
                    auto HateMod    = m_pOwner->GetHateMod((GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, GetSkillBase()->IsHarmful());

                    nAddHate += HateMod.second;
                    nAddHate *= HateMod.first;

                    if (nAddHate == 0)
                        ++nAddHate;

                    if (pDealTarget->IsMonster())
                    {
                        auto pMonster = pDealTarget->As<Monster>();
                        pMonster->AddHate(m_pOwner->GetHandle(), nAddHate, true, true);
                    }
                    else if (pDealTarget->IsNPC())
                    {
                        /// @Todo: SetAttacker
                    }
                    else if (!m_pOwner->IsEnemy(pDealTarget, false))
                    {
                        /// @Todo: AddHateToEnemyList
                    }

                    float fHV{0.0f};
                    if (nAddHate > 0)
                    {
                        fHV = nAddHate * 0.02f;
                    }
                    else
                    {
                        if (GetSkillBase()->IsPhysicalSkill())
                            fHV = nAddHate * 0.65f;
                        else
                            fHV = nAddHate * 0.25f;
                    }

                    float fSC{0.0f};
                    auto  cool_time = GetSkillCoolTime();
                    if (cool_time <= 1000)
                        fSC = 1.0f;
                    else if (cool_time <= 3000)
                        fSC = 1.2f;
                    else if (cool_time <= 6000)
                        fSC = 1.4f;
                    else if (cool_time <= 15000)
                        fSC = 1.7f;
                    else
                        fSC = 2.0f;

                    /// @Todo: AddHavoc, like seriously, wtf is havoc?!
                }
            }
        }

        if (sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_MAGIC_DAMAGE || sr.type == TS_SKILL__HIT_TYPE::SHT_DAMAGE_WITH_KNOCK_BACK)
        {
            /* @TODO
            if(sr.damage.flag ^ AIF_Critical)
                m_pOwner->ProcessAddHPMPOnCritical();
            */
        }
    }

    StateSkillFunctor myStateSkillFunctor{&m_vResultList};
    for (auto         &target : vNeedStateList)
    {
        process_target(t, myStateSkillFunctor, target);
    }

    int nElementalType = GetSkillBase()->GetElementalType();
    /// @Todo
    //m_pOwner->RemoveExhaustiveSkillStateMod( GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful(), nElementalType, GetOriginalCastingDelay() );
}

void Skill::DO_SUMMON()
{
    auto player = m_pOwner->As<Player>();
    if (player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if (item == nullptr)
        return;

    if (item->m_pItemBase->group != GROUP_SUMMONCARD)
        return;

    auto summon = item->m_pSummon;
    if (summon == nullptr || summon->GetMaster()->GetHandle() != player->GetHandle())
        return;

    if (!summon->IsInWorld())
        player->DoSummon(summon, m_targetPosition);

}

void Skill::DO_UNSUMMON()
{
    auto player = m_pOwner->As<Player>();
    if (player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if (item == nullptr)
        return;

    if (item->m_pItemBase->group != GROUP_SUMMONCARD)
        return;

    auto summon = item->m_pSummon;
    if (summon == nullptr || summon->GetMaster()->GetHandle() != player->GetHandle())
        return;

    if (summon->IsInWorld())
        player->DoUnSummon(summon);
}

bool Skill::Cancel()
{
    Init();
    broadcastSkillMessage(m_pOwner, 0, 0, SKILL_STATUS::ST_CANCEL);
    return true;
}

bool Skill::CheckCoolTime(uint t) const
{
    return m_nNextCoolTime < t;
}

uint Skill::GetSkillCoolTime() const
{
    int l{0};
    if (m_SkillBase == nullptr)
        return 0;

    if (m_pOwner->IsSummon())
        l = m_nRequestedSkillLevel;
    else
        l = m_nEnhance;

    auto cts = m_pOwner->GetCoolTimeSpeed();
    auto ctm = m_pOwner->GetCoolTimeMod((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
    auto ct  = m_SkillBase->GetCoolTime(l);
    return static_cast<uint32_t >(cts * ctm) * ct;
}

void Skill::SetRemainCoolTime(uint time)
{
    m_nNextCoolTime = time + sWorld.GetArTime();
}

uint Skill::GetSkillEnhance() const
{
    return m_nEnhance;
}

uint16 Skill::PrepareTaming(uint handle)
{
    if (!m_pOwner->IsPlayer())
        return TS_RESULT_NOT_ACTABLE;

    auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);
    if (pUnit == nullptr || !pUnit->IsMonster())
        return TS_RESULT_NOT_ACTABLE;

    auto pMonster      = pUnit->As<Monster>();
    auto nTameItemCode = pMonster->GetTameItemCode();
    if (nTameItemCode == 0)
        return TS_RESULT_NOT_ACTABLE;

    if (pMonster->GetTamer() != 0)
        return TS_RESULT_NOT_ACTABLE;

    if (pMonster->GetHealth() != pMonster->GetMaxHealth())
        return TS_RESULT_NOT_ENOUGH_HP;

    auto pPlayer = m_pOwner->As<Player>();
    if (pPlayer->FindItem((uint)nTameItemCode, ITEM_FLAG_SUMMON, false) == nullptr)
        return TS_RESULT_NOT_ACTABLE;

    if (pPlayer->GetTamingTarget() != 0)
        return TS_RESULT_ALREADY_TAMING;
    return TS_RESULT_SUCCESS;
}

void Skill::CREATURE_TAMING()
{
    auto pTarget = sMemoryPool.GetObjectInWorld<Monster>(m_hTarget);
    if (pTarget == nullptr || !pTarget->IsMonster() || !m_pOwner->IsPlayer())
        return;

    bool bResult = sWorld.SetTamer(pTarget, m_pOwner->As<Player>(), m_nRequestedSkillLevel);
    if (bResult)
    {
        pTarget->AddHate(m_pOwner->GetHandle(), 1, true, true);
    }
}

void Skill::PHYSICAL_SINGLE_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    if (m_SkillBase->effect_type == EF_PHYSICAL_SINGLE_DAMAGE_RUSH || m_SkillBase->effect_type == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK)
    {

    }
    bool v10          = m_SkillBase->is_physical_act != 0;
    auto attack_point = m_pOwner->GetAttackPointRight((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto nDamage      = (int)(attack_point
                              * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                              + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel) + (m_SkillBase->var[5] * m_nEnhance));

    auto damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental,
                                                   m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                   m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl), 0);

    AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
}

void Skill::SINGLE_MAGICAL_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    bool v10         = m_SkillBase->is_physical_act != 0;
    //auto attack_point = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto nMagicPoint = m_pOwner->m_Attribute.nMagicPoint;
    //int nDamage = nMagicPoint * ()
    auto nDamage     = (int)(nMagicPoint
                             * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                             + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel) + (m_SkillBase->var[5] * m_nEnhance));

    auto damage = pTarget->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental,
                                                  m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                  m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl), 0);

    AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
}

void Skill::SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    auto elemental_type = m_SkillBase->elemental;
    //auto nMagicPoint = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->elemental, this->m_SkillBase->is_physical_act == 0, m_SkillBase->is_harmful != 0);
    auto nMagicPoint    = m_pOwner->m_Attribute.nMagicPoint;
    auto nDamage        = (int)(nMagicPoint * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                                + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel)
                                + (m_SkillBase->var[5] * m_nEnhance));

    auto damage = pTarget->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental,
                                                  m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                  (m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl)), 0);

    AddSkillDamageResult(m_vResultList, 1, (uint8)elemental_type, damage, pTarget->GetHandle());

    auto nAddHP = (int)(((m_SkillBase->var[8] * m_nEnhance) + (m_SkillBase->var[6] + (m_SkillBase->var[7] * m_nRequestedSkillLevel))) * damage.nDamage);
    auto nAddMP = (int)(((m_SkillBase->var[11] * m_nEnhance) + (m_SkillBase->var[9] + (m_SkillBase->var[10] * m_nRequestedSkillLevel))) * damage.nDamage);
    m_pOwner->AddHealth(nAddHP);
    m_pOwner->AddMana(nAddMP);

    SkillResult skill_result{ };
    skill_result.type                   = (int)SRT_ADD_HP;
    skill_result.hTarget                = m_pOwner->GetHandle();
    skill_result.hitAddStat.target_stat = m_pOwner->GetHealth();
    skill_result.hitAddStat.nIncStat    = nAddHP;

    m_vResultList.emplace_back(skill_result);

    SkillResult skillResult{ };
    skillResult.type                   = (int)SRT_ADD_MP;
    skillResult.hTarget                = m_pOwner->GetHandle();
    skillResult.hitAddStat.target_stat = m_pOwner->GetMana();
    skillResult.hitAddStat.nIncStat    = nAddHP;
    m_vResultList.emplace_back(skillResult);
}

void Skill::ACTIVATE_FIELD_PROP()
{
    auto fp = sMemoryPool.GetObjectInWorld<FieldProp>(m_hTarget);
    if (fp != nullptr)
    {
        AddSkillResult(m_vResultList, fp->UseProp(m_pOwner->As<Player>()), 0, 0);
    }
}

void Skill::TOWN_PORTAL()
{
    if (m_pOwner->IsPlayer())
    {
        auto pPlayer = m_pOwner->As<Player>();

        auto pos = pPlayer->GetLastTownPosition();
        pPlayer->PendWarp((int)pos.GetPositionX(), (int)pos.GetPositionY(), 0);
        pos = pPlayer->GetCurrentPosition(sWorld.GetArTime());
        pPlayer->SetMove(pos, 0, 0);
    }
}

void Skill::MULTIPLE_MAGICAL_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    bool v10         = m_SkillBase->is_physical_act != 0;
    //auto attack_point = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto nMagicPoint = m_pOwner->m_Attribute.nMagicPoint;
    auto nDamage     = nMagicPoint * (GetVar(0) + GetVar(1) * m_nRequestedSkillLevel + GetVar(2) * GetSkillEnhance()) + GetVar(3) + GetVar(4) * m_nRequestedSkillLevel + GetVar(5) * GetSkillEnhance();

    if (m_nCurrentFire != 0)
    {
        m_nCurrentFire++;
    }
    else
    {
        m_nTotalFire   = (int)(m_SkillBase->var[6] + (m_SkillBase->var[7] * m_nRequestedSkillLevel));
        m_nCurrentFire = 1;
    }

    auto Damage = pTarget->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental,
                                                  m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                  (m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl)), 0);

    AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, Damage, pTarget->GetHandle());
    m_nFireTime = (uint)((m_SkillBase->var[8] * 100) + m_nFireTime);
}

void Skill::TOGGLE_AURA(Unit *pTarget)
{
    m_pOwner->ToggleAura(this);

    if (m_SkillBase->target == 21)
    {
        // TODO: Party functor
    }
    AddSkillResult(m_vResultList, true, 0, m_pOwner->GetHandle());
}

void Skill::SKILL_RESURRECTION(Unit *pTarget)
{
    if (pTarget == nullptr || pTarget->GetHealth() != 0)
        return;

    auto prev_hp = pTarget->GetHealth();
    auto prev_mp = pTarget->GetMana();

    pTarget->AddHealth((int)CalculatePct(pTarget->GetMaxHealth(), (m_SkillBase->var[0] * 100)));
    pTarget->AddMana((int)CalculatePct(pTarget->GetMaxMana(), (m_SkillBase->var[1] * 100)));

    SkillResult skillResult{ };
    skillResult.type                    = SRT_REBIRTH;
    skillResult.hTarget                 = pTarget->GetHandle();
    skillResult.hitRebirth.target_hp    = pTarget->GetHealth();
    skillResult.hitRebirth.nIncHP       = std::max((int)(pTarget->GetHealth() - prev_hp), 0);
    skillResult.hitRebirth.nIncMP       = std::max((int)(pTarget->GetMana() - prev_mp), 0);
    skillResult.hitRebirth.nRecoveryEXP = 0;
    skillResult.hitRebirth.target_mp    = (int16)pTarget->GetMana();
    m_vResultList.emplace_back(skillResult);
}

void Skill::MAGIC_MULTIPLE_REGION_DAMAGE(Unit *pTarget)
{
    if (!pTarget)
        return;

    auto attack_point = m_pOwner->GetAttackPointRight((ElementalType)m_SkillBase->elemental, m_SkillBase->is_physical_act != 0, m_SkillBase->is_harmful != 0);

    if (m_nCurrentFire != 0)
    {
        m_nCurrentFire++;
    }
    else
    {
        m_nTotalFire   = (int)(m_SkillBase->var[6] + (m_SkillBase->var[7] * m_nRequestedSkillLevel));
        m_nCurrentFire = 1;
    }

    auto magic_point = m_pOwner->m_Attribute.nMagicPoint;

    auto nDamage = (int)(magic_point
                         * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                         + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel) + (m_SkillBase->var[5] * m_nEnhance));

    auto  ct            = sWorld.GetArTime();
    float fEffectLength = m_SkillBase->var[9] * 12;
    m_fRange = fEffectLength;

    std::vector<Unit *> vTargetList{ };
    nDamage = EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(ct),
                                            m_pOwner->GetLayer(),
                                            pTarget->GetCurrentPosition(ct),
                                            true,
                                            fEffectLength,
                                            -1,
                                            0.0f,
                                            nDamage,
                                            true,
                                            m_pOwner,
                                            (int)m_SkillBase->var[10],
                                            (int)m_SkillBase->var[11],
                                            vTargetList,
                                            true);

    m_nTargetCount = (int)vTargetList.size();
    for (auto &target : vTargetList)
    {
        auto hitBonus = m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - target->GetLevel());
        int  nFlag    = m_SkillBase->critical_bonus + m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl;
        auto damage   = target->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental, hitBonus, nFlag, 0);
        AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, target->GetHandle());
    }
    m_nFireTime = (uint)((m_SkillBase->var[8] * 100) + m_nFireTime);
}

void Skill::MAGIC_SINGLE_REGION_DAMAGE(Unit *pTarget)
{
    std::vector<Unit *> vTargetList{ };
    if (pTarget == nullptr)
        return;

    if (m_SkillBase->effect_type == SKILL_EFFECT_TYPE::EF_MAGIC_SINGLE_REGION_DAMAGE_USING_CORPSE)
    {
        if (!pTarget->IsMonster() || pTarget->GetHealth() != 0)
            return;
        sWorld.RemoveObjectFromWorld(pTarget);
    }

    auto nMagicPoint = static_cast<int>(m_pOwner->m_Attribute.nMagicPoint); // m_pOwner->GetMagicPoint();
    auto nDamage     = (int)(nMagicPoint * (GetVar(0) + GetVar(1) * m_nRequestedSkillLevel + GetVar(2) * GetSkillEnhance()) + GetVar(3) + GetVar(4) * m_nRequestedSkillLevel + GetVar(5) * GetSkillEnhance());

    float effectLength{ };
    float distributeType{ };
    int   targetMax{ };

    if (m_SkillBase->effect_type == SKILL_EFFECT_TYPE::EF_MAGIC_SINGLE_REGION_DAMAGE_ADD_RANDOM_STATE)
    {
        effectLength   = GetVar(12) * 12;
        distributeType = GetVar(13);
        targetMax      = GetVar(14);
    }
    else
    {
        effectLength   = GetVar(9) * 12;
        distributeType = GetVar(10);
        targetMax      = GetVar(11);
    }
    m_fRange         = effectLength;

    uint ct = sWorld.GetArTime();

    nDamage = EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(ct),
                                            m_pOwner->GetLayer(),
                                            pTarget->GetCurrentPosition(ct),
                                            true,
                                            effectLength,
                                            -1,
                                            0.0f,
                                            nDamage,
                                            true,
                                            m_pOwner,
                                            (int)distributeType,
                                            targetMax,
                                            vTargetList,
                                            true);

    m_nTargetCount = static_cast<int>(vTargetList.size());
    for (const auto &unit : vTargetList)
    {
        auto hitBonus = m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - unit->GetLevel());
        int  nFlag    = m_SkillBase->critical_bonus + m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl;
        auto damage   = unit->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental, hitBonus, nFlag, 0);
        AddSkillDamageResult(m_vResultList, 1, (uint8)m_SkillBase->elemental, damage, unit->GetHandle());
    }
}

void Skill::ADD_ENERGY()
{
    for (int i = 0; i < m_nRequestedSkillLevel && m_pOwner->GetInt32Value(UNIT_FIELD_ENERGY) < m_pOwner->GetInt32Value(UNIT_FIELD_MAX_ENERGY); i++)
        m_pOwner->AddEnergy();

    m_vResultList.push_back({ });
}

void Skill::process_target(uint t, SkillTargetFunctor &fn, Unit *pTarget)
{
    switch (GetSkillBase()->GetSkillTargetType())
    {
        case TARGET_TYPE::TARGET_TARGET:
        case TARGET_TYPE::TARGET_TARGET_EXCEPT_CASTER:
        {
            if (GetSkillBase()->GetSkillTargetType() == TARGET_TARGET_EXCEPT_CASTER && pTarget == m_pOwner)
                break;

            if (GetSkillBase()->IsNeedTarget())
            {
                if (pTarget)
                    fn.onCreature(this, t, m_pOwner, pTarget);
            }
            else
                fn.onCreature(this, t, m_pOwner, m_pOwner);
            break;
        }
        case TARGET_TYPE::TARGET_REGION:
        case TARGET_TYPE::TARGET_REGION_WITH_TARGET:
        case TARGET_TYPE::TARGET_REGION_WITHOUT_TARGET:
        {
            if (!m_pOwner->IsPlayer() || !pTarget->IsPlayer())
                break;

            auto pPlayer       = m_pOwner->As<Player>();
            auto pTargetPlayer = m_pOwner->As<Player>();
            if (pPlayer != pTargetPlayer && pPlayer->GetPartyID() != 0 && pPlayer->GetPartyID() != pTargetPlayer->GetPartyID())
                break;

            /// @Partyfunctor
            break;
        }
        case TARGET_TYPE::TARGET_GUILD:
        {
            if (!m_pOwner->IsPlayer() || !pTarget->IsPlayer())
                break;

            auto pPlayer       = m_pOwner->As<Player>();
            auto pTargetPlayer = m_pOwner->As<Player>();
            if (pPlayer != pTargetPlayer && pPlayer->GetGuildID() != 0 && pPlayer->GetGuildID() != pTargetPlayer->GetGuildID())
                break;

            /// @GuildFunctor
            break;
        }
        case TARGET_TYPE::TARGET_ATTACKTEAM:
        {
            if (!m_pOwner->IsPlayer() || !pTarget->IsPlayer())
                break;

            auto pPlayer       = m_pOwner->As<Player>();
            auto pTargetPlayer = m_pOwner->As<Player>();
            if (pPlayer != pTargetPlayer && pPlayer->GetPartyID() != 0 && pPlayer->GetPartyID() != pTargetPlayer->GetPartyID())
                break;

            /// @AttackPartyfunctor
            break;
        }
        case TARGET_TYPE::TARGET_SUMMON:
        {
            Summon *pSummon{nullptr};
            if (pTarget->IsSummon() && pTarget->IsInWorld())
                pSummon = pTarget->As<Summon>();
            else if (pTarget->IsPlayer() && pTarget->As<Player>()->GetMainSummon() != nullptr && pTarget->As<Player>()->GetMainSummon()->IsInWorld())
                pSummon = pTarget->As<Player>()->GetMainSummon();

            if (pSummon == nullptr)
                break;

            auto pos = pSummon->GetPosition();

            if (auto tmpPos = pTarget->GetPosition(); GetSkillBase()->GetFireRange() < pos.GetExactDist2d(&tmpPos))
                break;

            fn.onCreature(this, t, m_pOwner, pSummon);
            break;
        }
        case TARGET_TYPE::TARGET_PARTY_SUMMON:
        {
            /// @TODO: Party Functor
            break;
        }
        case TARGET_TYPE::TARGET_SELF_WITH_SUMMON:
        {
            Player *pTargetPlayer{nullptr};
            if (pTarget->IsPlayer())
                pTargetPlayer = pTarget->As<Player>();
            else if (pTarget->IsSummon() && pTarget->As<Summon>()->GetMaster() != nullptr)
                pTargetPlayer = pTarget->As<Summon>()->GetMaster();

            if (pTargetPlayer == nullptr || !pTargetPlayer->IsInWorld())
                break;

            fn.onCreature(this, t, m_pOwner, pTargetPlayer);

            auto pos = m_pOwner->GetPosition();
            {
                Summon *pSummon = pTargetPlayer->GetMainSummon();
                if (pSummon != nullptr && pSummon->IsInWorld() && pSummon->GetHealth() != 0 && GetSkillBase()->GetFireRange() >= pos.GetExactDist2d(pSummon))
                    fn.onCreature(this, t, m_pOwner, pSummon);
            }
            {
                Summon *pSummon = pTargetPlayer->GetSubSummon();
                if (pSummon != nullptr && pSummon->IsInWorld() && pSummon->GetHealth() != 0 && GetSkillBase()->GetFireRange() >= pos.GetExactDist2d(pSummon))
                    fn.onCreature(this, t, m_pOwner, pTargetPlayer->GetSubSummon());
            }
            break;
        }
        case TARGET_TYPE::TARGET_PARTY_WITH_SUMMON:
        {
            /// @Todo PartyFunctor
            break;
        }
        case TARGET_TYPE::TARGET_MASTER:
        {
            if (!m_pOwner->IsSummon())
                break;

            Player *pTargetPlayer = m_pOwner->As<Summon>()->GetMaster();
            if (pTargetPlayer == nullptr || !pTargetPlayer->IsInWorld())
                break;

            fn.onCreature(this, t, m_pOwner, pTargetPlayer);
            break;
        }
        case TARGET_TYPE::TARGET_SELF_WITH_MASTER:
        {
            if (!m_pOwner->IsSummon() || !m_pOwner->IsInWorld())
                break;

            fn.onCreature(this, t, m_pOwner, m_pOwner);

            Player *pTargetPlayer = m_pOwner->As<Summon>()->GetMaster();
            if (pTargetPlayer == nullptr || !pTargetPlayer->IsInWorld())
                break;

            if (pTargetPlayer->GetExactDist2d(m_pOwner) > 525.0f)
                break;

            fn.onCreature(this, t, m_pOwner, pTargetPlayer);
            break;
        }
        default:
        {
            auto result = string_format("TARGET_TYPE %d in process_target not handled yet.", m_SkillBase->target);
            if (m_pOwner->IsPlayer())
                Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Player>(), result);
            else if (m_pOwner->IsSummon())
            {
                Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Summon>()->GetMaster(), result);
            }
            NG_LOG_DEBUG("skill", "%s", result.c_str());
            break;
        }
    }
}

void Skill::SKILL_ADD_HP_MP(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nDecHP{0};
    int nDecMP{0};
    int nIncHP{0};
    int nIncMP{0};

    switch (GetSkillBase()->GetSkillEffectType())
    {
        case EF_ADD_HP_MP_BY_SUMMON_DEAD:
        {
            if (!m_pOwner->IsPlayer())
                return;
            auto pSummon = m_pOwner->As<Player>()->m_pMainSummon;
            if (pSummon == nullptr || pSummon->GetHealth() == 0 || pSummon->GetHandle() == pTarget->GetHandle())
            {
                AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
                return;
            }
            nDecHP = pSummon->GetHealth();
            pSummon->damage(m_pOwner, nDecHP, false);
            DamageInfo info{ };
            info.nDamage   = nDecHP;
            info.target_hp = pSummon->GetHealth();
            AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, ElementalType::TYPE_NONE, info, pSummon->GetHandle());
        }
            break;
        case EF_ADD_HP_MP_BY_SUMMON_DAMAGE:
        {
            if (!m_pOwner->IsPlayer())
                return;

            auto pSummon = m_pOwner->As<Player>()->m_pMainSummon;
            if (pSummon == nullptr || pSummon->GetHealth() == 0)
            {
                AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
                return;
            }
            nDecHP = pSummon->GetMaxHealth() * GetVar(10);
            pSummon->AddHealth(-nDecHP);

            SkillResult skillResult{ };
            skillResult.type                   = static_cast<uint8_t>(TS_SKILL__HIT_TYPE::SHT_ADD_HP);
            skillResult.hTarget                = pSummon->GetHandle();
            skillResult.hitAddStat.target_stat = pTarget->GetMana();
            skillResult.hitAddStat.nIncStat    = -nDecHP;
            m_vResultList.emplace_back(skillResult);
        }
            break;
        case EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP:
        {
            if (!m_pOwner->IsPlayer())
                return;

            auto pSummon = m_pOwner->As<Player>()->m_pMainSummon;
            if (pSummon == nullptr || pSummon->GetHealth() == 0)
            {
                AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
                return;
            }

            nDecHP = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(4) * GetSkillEnhance();
            nDecMP = GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();

            pSummon->AddHealth(-nDecHP);
            pSummon->AddMana(-nDecMP);

            SkillResult skillResult{ };
            skillResult.type                   = static_cast<uint8_t >(TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP);
            skillResult.hTarget                = pSummon->GetHandle();
            skillResult.hitAddHPMPSP.target_hp = pSummon->GetHealth();
            skillResult.hitAddHPMPSP.target_mp = static_cast<int16_t >(pSummon->GetMana());
            skillResult.hitAddHPMPSP.nIncHP    = -nDecHP;
            skillResult.hitAddHPMPSP.nIncMP    = -nDecMP;
            m_vResultList.emplace_back(skillResult);
        }
            break;
        default:
            break;
    }

    if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_STEAL_SUMMON_HP_MP)
    {
        nIncHP = nDecHP * GetVar(6) + GetVar(7) * GetSkillEnhance();
        nIncMP = nDecMP * GetVar(6) + GetVar(7) * GetSkillEnhance();
    }
    else if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_WITH_LIMIT_PERCENT)
    {
        int nLimitHP = (GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(2) * GetSkillEnhance()) * pTarget->GetMaxHealth();
        int nLimitMP = (GetVar(3) + GetVar(4) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance()) * pTarget->GetMaxMana();

        if (nLimitHP > pTarget->GetHealth())
            nIncHP = nLimitHP - pTarget->GetHealth();
        if (nLimitMP > pTarget->GetMana())
            nIncMP = nLimitMP - pTarget->GetMana();
    }
    else
    {
        nIncHP = m_pOwner->GetMagicPoint((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful()) * (GetVar(0) + GetVar(1) * GetRequestedSkillLevel())
                 + GetVar(2) * GetRequestedSkillLevel()
                 + pTarget->GetMaxHealth() * GetVar(3) * GetRequestedSkillLevel()
                 + GetVar(4) * GetSkillEnhance();

        nIncMP = m_pOwner->GetMagicPoint((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful()) * (GetVar(5) + GetVar(6) * GetRequestedSkillLevel())
                 + GetVar(7) * GetRequestedSkillLevel()
                 + pTarget->GetMaxMana() * GetVar(8) * GetRequestedSkillLevel()
                 + GetVar(9) * GetSkillEnhance();

        if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP ||
            GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_SUMMON_DEAD)
        {
            nIncHP += GetVar(10);
            nIncMP += GetVar(11);
        }

        if (GetSkillBase()->GetSkillEffectType() == EF_ADD_HP_MP_BY_SUMMON_DEAD)
        {
            nIncHP += pTarget->GetMaxHealth() * GetVar(12);
            nIncMP += pTarget->GetMaxMana() * GetVar(13);
        }
    }

    nIncHP = pTarget->Heal(nIncHP);
    nIncMP = pTarget->MPHeal(nIncMP);

    SkillResult skillResult{ };
    skillResult.type                   = static_cast<uint8_t >(TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP);
    skillResult.hTarget                = pTarget->GetHandle();
    skillResult.hitAddHPMPSP.target_hp = pTarget->GetHealth();
    skillResult.hitAddHPMPSP.target_mp = static_cast<int16_t >(pTarget->GetMana());
    skillResult.hitAddHPMPSP.nIncHP    = nIncHP;
    skillResult.hitAddHPMPSP.nIncMP    = nIncMP;

    m_vResultList.emplace_back(skillResult);
}

void Skill::PHYSICAL_MULTIPLE_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int elemental_type = GetSkillBase()->GetElementalType();

    int nDamage = m_pOwner->GetAttackPointRight((ElementalType)elemental_type, GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
    nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(10) * GetSkillEnhance();
    nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

    int nCount = GetVar(4) + GetVar(5) * GetRequestedSkillLevel();

    for (int i = 0; i < nCount; ++i)
    {
        DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);

        AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType >(elemental_type), Damage, pTarget->GetHandle());
    }

    m_nFireCount = static_cast<uint32_t >(nCount);
    m_nFireTime += GetVar(6) * nCount * 100;
}

void Skill::PHYSICAL_MULTIPLE_REGION_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    auto elemental_type = GetSkillBase()->GetElementalType();
    int  nDamage        = m_pOwner->GetAttackPointRight((ElementalType)elemental_type, GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    int32_t  nCount{0};
    bool     bTargetOrigin{false};
    int32_t  nRegionType{-1};
    float    fRegionProperty{0.0f};
    uint32_t nFireInterval{0};

    switch (GetSkillBase()->GetSkillEffectType())
    {
        case EF_PHYSICAL_MULTIPLE_REGION_DAMAGE:
            nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(10) * GetSkillEnhance();
            nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

            nCount = GetVar(7) + GetVar(8) * GetRequestedSkillLevel();

            nFireInterval = GetVar(9) * 100;
            break;
        case EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF:
            bTargetOrigin = false;
            [[fallthrough]];
        case EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE:
            nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel();
            nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

            nCount = GetVar(9) + GetVar(10) * GetRequestedSkillLevel();

            nFireInterval = GetVar(11) * 100;

            nRegionType     = static_cast< int >( GetVar(7));
            fRegionProperty = GetVar(8);
            break;
        default:
            return;
    }

    float fEffectLength = GetVar(4) * 12;
    m_fRange              = fEffectLength;

    std::vector<Unit *> vTargetList{ };
    auto                t = sWorld.GetArTime();

    nDamage = EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), bTargetOrigin, fEffectLength, nRegionType, fRegionProperty, nDamage, true, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);
    for (int i = 0; i < vTargetList.size(); i++)
    {
        for (auto &target : vTargetList)
        {
            if (target->GetHealth() == 0)
                continue;

            DamageInfo Damage = target->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - target->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
            AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, (ElementalType)elemental_type, Damage, target->GetHandle());
        }
    }
    m_nFireCount += nFireInterval * nCount;
}

void Skill::TAUNT(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nHate  = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(7) * GetSkillEnhance();
    int nRatio = GetVar(5) + GetVar(6) * GetRequestedSkillLevel() + GetVar(9) * GetSkillEnhance();

    bool bSuccess = rand32() % 100 < nRatio;

    if (bSuccess && m_pOwner->IsEnemy(pTarget, false) && pTarget->IsMonster())
    {
        std::pair<float, int> HateMod = m_pOwner->GetHateMod((GetSkillBase()->IsPhysicalSkill()) ? 1 : 2, GetSkillBase()->IsHarmful());

        nHate += HateMod.second;
        nHate *= HateMod.first;

        ///@Todo: AddHateRatio
        pTarget->As<Monster>()->AddHate(m_pOwner->GetHandle(), 1/*m_pOwner->GetHateRatio()*/ * nHate, true, true);
    }

    AddSkillResult(m_vResultList, bSuccess, SkillResult::ADD_HATE, pTarget->GetHandle());
}

void Skill::PHYSICAL_SINGLE_REGION_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int elemental_type = GetSkillBase()->GetElementalType();
    int nDamage        = m_pOwner->GetAttackPointRight((ElementalType)elemental_type, GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    bool     bTargetOrigin{true};
    int      nRegionType{-1};
    float    fRegionProperty{0.0f};
    float    fKnockbackRange{0.0f};
    uint32_t nKnockbackTime{0};
    int      nCastingCancelRate{0};

    switch (GetSkillBase()->GetSkillEffectType())
    {
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE:
            nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(10) * GetSkillEnhance();
            nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();
            break;
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF:
            bTargetOrigin   = false;
            [[fallthrough]];
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK:
            nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel();
            nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel();
            nRegionType     = static_cast< int >( GetVar(7));
            fRegionProperty = GetVar(8);
            fKnockbackRange = (GetVar(9) + GetVar(10) * GetRequestedSkillLevel()) * 12;
            nKnockbackTime  = (GetVar(11) + GetVar(12) * GetRequestedSkillLevel()) * 100;
            break;
        case EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL:
            nCastingCancelRate = GetVar(12) + GetVar(13) * GetRequestedSkillLevel() + GetVar(14) * GetSkillEnhance();
            break;
        default:
            return;
    }

    float fEffectLength = GetVar(4) * 12;
    m_fRange              = fEffectLength;
    std::vector<Unit *> vTargetList{ };
    auto                t = sWorld.GetArTime();

    nDamage = EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), bTargetOrigin, fEffectLength, nRegionType, fRegionProperty, nDamage, true, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);

    for (auto &pDealTarget : vTargetList)
    {
        DamageInfo Damage = pDealTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pDealTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
        if (!Damage.bBlock && !Damage.bMiss && !Damage.bPerfectBlock)
        {
            if ((GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK
                 || GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF)
                && !(pDealTarget->IsMonster() && pDealTarget->As<Monster>()->IsBossMonster()))
            {
                // Knockback here
            }
            else if (GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL)
            {
                // Cancel skill from target here
                AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
            }
            else
            {
                AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
            }
        }
        else
        {
            AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
        }
    }
}

void Skill::PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int elemental_type = GetSkillBase()->GetElementalType();
    int nDamage        = m_pOwner->GetAttackPointRight((ElementalType)elemental_type, GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
    nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(13) * GetSkillEnhance();
    nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

    float fEffectLength = GetVar(4) * 12;
    m_fRange              = fEffectLength;

    std::vector<Unit *> vTargetList{ };
    auto                t = sWorld.GetArTime();

    nDamage = EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), GetVar(8) != 0, fEffectLength, GetVar(7), GetVar(10), nDamage, GetVar(9) != 0, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);

    for (auto &pDealTarget : vTargetList)
    {
        DamageInfo Damage = pDealTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pDealTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
        AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType >(elemental_type), Damage, pDealTarget->GetHandle());
    }
}

bool Skill::ProcAura()
{
    auto   t = sWorld.GetArTime();
    Summon *pSummon{nullptr};

    if (GetAuraMPDecTime() < t + 7500)
    {
        float fDecMP    = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(2) * GetSkillEnhance();
        int   nDecHavoc = 0;

        /// @Todo: Havoc

        if (m_pOwner->GetMana() < fDecMP /* @TODO: Havoc */ )
            return false;

        m_pOwner->SetMana(static_cast<int32_t>(m_pOwner->GetMana() - fDecMP ));
        SetAuraMPDecTime(t);

        if (m_pOwner->IsInWorld())
            Messages::BroadcastHPMPMessage(m_pOwner, 0, m_pOwner->GetMana() - fDecMP, true);

        if (m_pOwner->IsSummon())
        {
            pSummon = m_pOwner->As<Summon>();
            if (pSummon->GetMaster() && pSummon->IsInWorld())
                Messages::SendHPMPMessage(pSummon->GetMaster(), m_pOwner, 0, m_pOwner->GetMana() - fDecMP, false);
        }
        /// @Todo: Havoc
    }

    if (!m_pOwner->IsInWorld())
        return true;

    std::vector<uint32> vList{ };
    sWorld.EnumMovableObject(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), GetSkillBase()->GetFireRange(), vList, true, true);

    if (vList.empty())
        return true;

    auto et = t + TOGGLE_LIVE_TIME;

    bool bApplySummon{false};

    for (auto &pTargetHandle : vList)
    {
        auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(pTargetHandle);
        if (pTarget == nullptr /* @TODO: IsPet */)
            continue;
        auto nTargetType = GetSkillBase()->GetSkillTargetType();

        if (GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_DIFFERENTIAL_AURA)
        {
            if (pSummon != nullptr)
                pTarget = pSummon;
            else if (pTarget->IsSummon())
            {
                if (m_pOwner->IsPlayer() && m_pOwner->As<Player>() != pTarget->As<Summon>()->GetMaster())
                    continue;
            }
            else
                continue;

            if (GetVar(5) != 0)
                pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(5)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");
            if (GetVar(6) != 0)
                pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(6)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");
            if (GetVar(7) != 0)
                pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(7)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

            bApplySummon = true;
            break;
        }
        else if (nTargetType == TARGET_TARGET)
        {
            if (pTarget != m_pOwner)
                continue;
        }
        else if (pTarget->IsSummon())
        {
            if (nTargetType != TARGET_PARTY_SUMMON
                && nTargetType != TARGET_PARTY_WITH_SUMMON
                && nTargetType != TARGET_SUMMON
                && nTargetType != TARGET_CREATURE_TYPE_NONE
                && nTargetType != TARGET_CREATURE_TYPE_FIRE
                && nTargetType != TARGET_CREATURE_TYPE_WATER
                && nTargetType != TARGET_CREATURE_TYPE_WIND
                && nTargetType != TARGET_CREATURE_TYPE_EARTH
                && nTargetType != TARGET_CREATURE_TYPE_LIGHT
                && nTargetType != TARGET_CREATURE_TYPE_DARK
                && nTargetType != TARGET_SELF_WITH_SUMMON
                && nTargetType != TARGET_SELF_WITH_MASTER)
                continue;

            if (nTargetType == TARGET_PARTY_SUMMON || nTargetType == TARGET_PARTY_WITH_SUMMON)
            {
                if (m_pOwner->IsPlayer() && m_pOwner->As<Player>()->GetPartyID())
                {
                    if (m_pOwner->As<Player>()->GetPartyID() != pTarget->As<Summon>()->GetMaster()->GetPartyID())
                        continue;
                }
                else
                {
                    if (m_pOwner->IsPlayer() && m_pOwner->As<Player>() != pTarget->As<Summon>()->GetMaster())
                        continue;
                }
            }
            else if (nTargetType == TARGET_SELF_WITH_MASTER)
            {
                if (m_pOwner != pTarget)
                    continue;
            }
            else
            {
                if (m_pOwner->IsPlayer() && m_pOwner->As<Player>() != pTarget->As<Summon>()->GetMaster())
                    continue;
                /// @TODO: SummonElementalType
            }
        }
        else if (pTarget->IsPlayer())
        {
            if (nTargetType != TARGET_PARTY &&
                nTargetType != TARGET_PARTY_WITH_SUMMON &&
                nTargetType != TARGET_SELF_WITH_SUMMON &&
                nTargetType != TARGET_MASTER &&
                nTargetType != TARGET_SELF_WITH_MASTER)
                continue;

            if (nTargetType == TARGET_PARTY || nTargetType == TARGET_PARTY_WITH_SUMMON)
            {
                Player *pOwnPlayer{nullptr};

                if (m_pOwner->IsPlayer())
                    pOwnPlayer = m_pOwner->As<Player>();
                if (m_pOwner->IsSummon())
                    pOwnPlayer = m_pOwner->As<Summon>()->GetMaster();

                if (pOwnPlayer == nullptr)
                    continue;

                if (pOwnPlayer->GetPartyID() == 0 && pTarget != m_pOwner)
                    continue;
                if (pTarget->IsPlayer() && pOwnPlayer->GetPartyID() != pTarget->As<Player>()->GetPartyID())
                    continue;
            }
            else if (nTargetType == TARGET_SELF_WITH_SUMMON)
            {
                if (pTarget != m_pOwner)
                    continue;
            }
            else if (nTargetType == TARGET_SELF_WITH_MASTER)
            {
                if (m_pOwner->IsSummon() && m_pOwner->As<Summon>()->GetMaster() != pTarget)
                    continue;
            }
        }
        else
        {
            continue;
        }

        pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetSkillBase()->GetStateId()), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

        if (GetVar(3) != 0)
            pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(3)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

        if (GetVar(4) != 0)
            pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(4)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

    }

    if (GetSkillBase()->GetSkillEffectType() == EF_TOGGLE_DIFFERENTIAL_AURA)
    {
        if (!bApplySummon)
            return false;

        Unit *pTarget{nullptr};
        if (pSummon)
            pTarget = pSummon->GetMaster();
        else
            pTarget = m_pOwner;

        pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetSkillBase()->GetStateId()), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

        if (GetVar(3) != 0)
            pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(3)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");

        if (GetVar(4) != 0)
            pTarget->AddState(SG_NORMAL, static_cast<StateCode>(GetVar(4)), m_pOwner->GetHandle(), m_SkillBase->GetStateLevel(GetRequestedSkillLevel(), GetSkillEnhance()), t, et, true, 0, "");
    }
    return true;
}

void Skill::PHYSICAL_SINGLE_DAMAGE_ABSORB(Unit *pTarget)
{
    if (!pTarget)
        return;

    int elemental_type = GetSkillBase()->GetElementalType();
    int nDamage        = m_pOwner->GetAttackPointRight((ElementalType)elemental_type, GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    nDamage *= GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(10) * GetSkillEnhance();
    nDamage += GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

    DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
    AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pTarget->GetHandle());

    int nAddHP = Damage.nDamage * GetVar(4);
    int nAddMP = Damage.nDamage * GetVar(5);
    m_pOwner->AddHealth(nAddHP);
    m_pOwner->AddMana(nAddMP);

    SkillResult skill_result{ };
    skill_result.type                   = TS_SKILL__HIT_TYPE::SHT_ADD_HP;
    skill_result.hTarget                = m_pOwner->GetHandle();
    skill_result.hitAddStat.nIncStat    = nAddHP;
    skill_result.hitAddStat.target_stat = m_pOwner->GetHealth();
    m_vResultList.push_back(skill_result);

    skill_result.type                   = TS_SKILL__HIT_TYPE::SHT_ADD_MP;
    skill_result.hTarget                = m_pOwner->GetHandle();
    skill_result.hitAddStat.nIncStat    = nAddMP;
    skill_result.hitAddStat.target_stat = m_pOwner->GetMana();
    m_vResultList.push_back(skill_result);
}

void Skill::ADD_REGION_STATE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    auto t = sWorld.GetArTime();

    float fEffectLength = GetVar(5) * 12;

    int nTargetType = GetVar(6);

    std::vector<uint32> vResult{ };
    m_fRange = fEffectLength;

    StateSkillFunctor mySkillFunctor(&m_vResultList);
    sWorld.EnumMovableObject(pTarget->GetCurrentPosition(t), pTarget->GetLayer(), fEffectLength, vResult, true, true);

    m_nTargetCount = 0;

    for (auto &it : vResult)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(it);

        if (pUnit == nullptr /*|| pUnit->IsPet()*/ )
            continue;
        if (pUnit->IsPlayer() && !GetSkillBase()->IsUseableOnAvatar())
            continue;
        if (pUnit->IsMonster() && !GetSkillBase()->IsUseableOnMonster())
            continue;
        if (pUnit->IsSummon() && !GetSkillBase()->IsUseableOnSummon())
            continue;

        if (nTargetType == 1)
        {
            if (m_pOwner->IsEnemy(pUnit, true))
                continue;
        }
        else if (nTargetType == 2)
        {
            if (!m_pOwner->IsAlly(pUnit))
                continue;
        }
        else if (nTargetType == 3)
        {
            if (!m_pOwner->IsEnemy(pUnit, true))
                continue;
        }

        mySkillFunctor.onCreature(this, t, m_pOwner, pUnit);

        ++m_nTargetCount;
    }
}

void Skill::ADD_STATE_BY_SELF_COST(Unit *pTarget)
{
    if (pTarget == nullptr || !pTarget->IsSummon())
        return;

    float fCostHP     = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(2) * GetSkillEnhance();
    float fCostSP     = GetVar(3) + GetVar(4) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();
    float fCostEnergy = GetVar(6) + GetVar(7) * GetRequestedSkillLevel() + GetVar(8) * GetSkillEnhance();
    float fCostMP     = GetVar(9) + GetVar(10) * GetRequestedSkillLevel() + GetVar(11) * GetSkillEnhance();

    if (pTarget->GetHealth() < fCostHP)
        return;
    ///@Todo: SP
    if (fCostEnergy > 0 && pTarget->GetUInt32Value(UNIT_FIELD_ENERGY) < fCostEnergy)
        return;
    if (pTarget->GetMana() < fCostMP)
        return;

    pTarget->AddHealth(static_cast<int>(-fCostHP ));
    ///@Todo: SP
    if (fCostEnergy > 0)
        pTarget->RemoveEnergy(static_cast<int>(fCostEnergy));
    else if (fCostEnergy == -1)
        pTarget->RemoveEnergy(pTarget->GetUInt32Value(UNIT_FIELD_ENERGY));
    pTarget->AddMana(static_cast<int>(-fCostMP));

    auto t = sWorld.GetArTime();

    StateSkillFunctor mySkillFunctor{&m_vResultList};
    mySkillFunctor.onCreature(this, t, m_pOwner, pTarget);
}

void Skill::ADD_REGION_STATE_BY_SELF_COST(Unit *pTarget)
{
    if (pTarget == nullptr || !pTarget->IsSummon())
        return;

    float fCostHP     = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(2) * GetSkillEnhance();
    float fCostSP     = GetVar(3) + GetVar(4) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();
    float fCostEnergy = GetVar(6) + GetVar(7) * GetRequestedSkillLevel() + GetVar(8) * GetSkillEnhance();

    float fEffectLength = GetVar(9) * 12;

    int nTargetType = GetVar(10);

    auto t = sWorld.GetArTime();

    std::vector<uint32_t> vResult;
    m_fRange = fEffectLength;

    StateSkillFunctor mySkillFunctor{&m_vResultList};

    sWorld.EnumMovableObject(pTarget->GetCurrentPosition(t), pTarget->GetLayer(), fEffectLength, vResult, true, true);

    m_nTargetCount = 0;

    for (auto &handle : vResult)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(handle);

        if (!m_pOwner->IsPlayer() || pUnit == nullptr || !pUnit->IsSummon() || pUnit->As<Summon>()->GetMaster()->GetPartyID() ||
            pUnit->As<Summon>()->GetMaster()->GetPartyID() != m_pOwner->As<Player>()->GetPartyID())
            continue;

        if (nTargetType == 1)
        {
            if (m_pOwner->IsEnemy(pUnit, true))
                continue;
        }
        else if (nTargetType == 2)
        {
            if (!m_pOwner->IsAlly(pUnit))
                continue;
        }
        else if (nTargetType == 3)
        {
            if (!m_pOwner->IsEnemy(pUnit, true))
                continue;
        }

        if (pUnit->GetHealth() < fCostHP)
            continue;
        ///@Todo: SP
        if (pUnit->GetUInt32Value(UNIT_FIELD_ENERGY) < fCostEnergy)
            continue;

        pUnit->AddHealth(static_cast<int32_t>(-fCostHP ));
        ///@Todo: SP
        pUnit->RemoveEnergy(static_cast<int32_t>(-fCostEnergy ));

        mySkillFunctor.onCreature(this, t, m_pOwner, pUnit);
        ++m_nTargetCount;
    }
}

void Skill::PHYSICAL_DIRECTIONAL_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nDamage      = 0;
    int nAttackPoint = m_pOwner->GetAttackPointRight((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    int elemental_type = GetSkillBase()->GetElementalType();
    nDamage = nAttackPoint + GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();

    int nAccAdd = 0;
    /* @Todo
    if( m_pOwner->IsBackOf( *pTarget ) ) nAccAdd = GetVar( 2 );
    else if( m_pOwner->IsSideOf( *pTarget ) ) nAccAdd = GetVar( 3 );
    else*/ nAccAdd = GetVar(4);

    DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()) + nAccAdd, GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);

    AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType>(elemental_type), Damage, pTarget->GetHandle());

    return;
}

void Skill::SINGLE_PHYSICAL_DAMAGE_ABSORB(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nDamage = 0;

    int nAttackPoint = m_pOwner->GetAttackPointRight((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    int elemental_type = GetSkillBase()->GetElementalType();
    nDamage = nAttackPoint + GetVar(0) + GetVar(1) * GetRequestedSkillLevel();

    DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);

    AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pTarget->GetHandle());

    float fHPMod = GetVar(4) * GetSkillEnhance();
    float fMPMod = GetVar(5) * GetSkillEnhance();

    int nAddHP = Damage.nDamage * GetVar(2) + Damage.nDamage * fHPMod;
    int nAddMP = Damage.nDamage * GetVar(3) + Damage.nDamage * fMPMod;
    m_pOwner->AddHealth(nAddHP);
    m_pOwner->AddMana(nAddMP);

    SkillResult skill_result{ };
    skill_result.type                   = TS_SKILL__HIT_TYPE::SHT_ADD_HP;
    skill_result.hTarget                = m_pOwner->GetHandle();
    skill_result.hitAddStat.nIncStat    = nAddHP;
    skill_result.hitAddStat.target_stat = m_pOwner->GetHealth();
    m_vResultList.push_back(skill_result);

    skill_result.type                   = TS_SKILL__HIT_TYPE::SHT_ADD_MP;
    skill_result.hTarget                = m_pOwner->GetHandle();
    skill_result.hitAddStat.nIncStat    = nAddMP;
    skill_result.hitAddStat.target_stat = m_pOwner->GetMana();
    m_vResultList.push_back(skill_result);
}

void Skill::SINGLE_PHYSICAL_DAMAGE_T1(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nDamage = 0;

    int nAttackPoint = m_pOwner->GetAttackPointRight((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    if (GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD)
    {
        if (m_nCurrentFire == 0)
        {
            m_nCurrentFire = 1;
            m_nTotalFire   = 2;

            if (!PHYSICAL_DAMAGE_RUSH(pTarget, m_nRushDamage))
                m_nCurrentFire = 2;
            return;
        }
        else
        {
            ///@TODO:
            /// sWorld.MoveObject(m_pOwner, m_RushPos, m_fRushFace);
            nDamage += m_nRushDamage;
            ++m_nCurrentFire;
        }
    }
    int elemental_type = GetSkillBase()->GetElementalType();
    nDamage += nAttackPoint + GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(4) * GetSkillEnhance();

    DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);

    if (!Damage.bBlock && !Damage.bMiss && !Damage.bPerfectBlock
        && (GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD
            || GetSkillBase()->GetSkillEffectType() == EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK_OLD)
        && !(pTarget->IsMonster() && pTarget->As<Monster>()->IsBossMonster()))
    {
        float fRange = GetVar(5) + GetVar(6) * GetRequestedSkillLevel() + GetVar(7) * GetSkillEnhance();
        fRange *= 12;

        uint32_t knock_back_time = (GetVar(8) + GetVar(9) * GetRequestedSkillLevel() + GetVar(10) * GetSkillEnhance()) * 100;

        AFFECT_KNOCK_BACK(pTarget, fRange, knock_back_time);
        AddSkillDamageWithKnockBackResult(m_vResultList, SkillResult::DAMAGE_WITH_KNOCK_BACK, elemental_type, Damage, pTarget->GetHandle(), pTarget->GetPositionX(), pTarget->GetPositionY(), knock_back_time);
    }
    else
    {
        AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType>(elemental_type), Damage, pTarget->GetHandle());
    }

}

void Skill::SINGLE_PHYSICAL_DAMAGE_T2(Unit *pTarget)
{
    if (pTarget == nullptr)
        return;

    int nDamage      = 0;
    int nAttackPoint = m_pOwner->GetAttackPointRight((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());

    nDamage = nAttackPoint * (GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(4) * GetSkillEnhance());
    int nMax = nAttackPoint + GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();
    nDamage = std::min(nDamage, nMax);

    int elemental_type = GetSkillBase()->GetElementalType();

    DamageInfo Damage = pTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
    AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, (ElementalType)elemental_type, Damage, pTarget->GetHandle());
}

bool Skill::PHYSICAL_DAMAGE_RUSH(Unit *pTarget, int &pnAdditionalDamage)
{
    SkillResult result{ };
    result.type    = TS_SKILL__HIT_TYPE::SHT_RUSH;
    result.hTarget = m_pOwner->GetHandle();

    Position RushPos{ };

    float face{ };
    float fDistance{ };

    pnAdditionalDamage = 0;

    if (!AFFECT_RUSH_OLD(pTarget, fDistance, RushPos, face))
    {
        result.hitRush.bResult = false;
        m_vResultList.push_back(result);
        return false;
    }

    pnAdditionalDamage = static_cast<int32_t>((fDistance / 12.0f) * GetVar(2));

    result.hitRush.bResult = true;
    result.hitRush.x       = RushPos.GetPositionX();
    result.hitRush.y       = RushPos.GetPositionY();
    result.hitRush.speed   = -116;

    m_vResultList.push_back(result);
    return true;
}

bool Skill::AFFECT_RUSH_OLD(Unit *pTarget, float &pfRushDistance, Position &pRushPos, float &pface)
{

    auto     t            = sWorld.GetArTime();
    Position original_pos = m_pOwner->GetCurrentPosition(t);
    auto     tmpPos       = pTarget->GetCurrentPosition(t);
    int      nDelay       = original_pos.GetExactDist2d(&tmpPos) / ((float)-116 / 30);
    Position target_pos   = pTarget->GetCurrentPosition(t + nDelay);

    float x{ }, y{ }, m{ }, face{ };

    x = target_pos.GetPositionX() - original_pos.GetPositionX();
    y = target_pos.GetPositionY() - original_pos.GetPositionY();

    face = std::atan2(y, x);

    m = std::sqrt(x * x + y * y);

    if (m <= (m_pOwner->GetUnitSize() + pTarget->GetUnitSize()) / 2)
        return false;

    x /= m;
    y /= m;

    m -= (m_pOwner->GetUnitSize() + pTarget->GetUnitSize()) / 2;

    Position pos{ };

    pos.SetCurrentXY(original_pos.GetPositionY() + x * m, original_pos.GetPositionY() + y * m);

    if (GameContent::IsBlocked(pos.GetPositionX(), pos.GetPositionY()))
        return false;

    m_RushPos   = pos;
    m_fRushFace = face;

    pfRushDistance = m;
    pRushPos       = pos;
    pface          = face;

    m_nFireTime += m / ((float)-116 / 30) + 20;

    return true;
}

int Skill::AFFECT_KNOCK_BACK(Unit *pTarget, float fRange, uint32_t knock_back_time)
{
    Position caster_pos  = m_pOwner->GetCurrentPosition(sWorld.GetArTime());
    Position OriginalPos = pTarget->GetCurrentPosition(sWorld.GetArTime());

    float x{ }, y{ }, m{ };

    x = OriginalPos.GetPositionX() - caster_pos.GetPositionX();
    y = OriginalPos.GetPositionY() - caster_pos.GetPositionY();

    m = std::sqrt(x * x + y * y);

    if (m > 0.0f)
    {
        x /= m;
        y /= m;
    }
    else
    {
        x = 1.0f;
        y = 0.0f;
    }

    Position pos{ };
    pos.SetCurrentXY(OriginalPos.GetPositionX() + x * fRange, OriginalPos.GetPositionY() + y * fRange);

    uint32_t next_movable_time = pTarget->GetNextMovableTime();

    if (pTarget->IsKnockbackable())
    {
        next_movable_time = std::max(next_movable_time, sWorld.GetArTime() + knock_back_time);

        Position newPos = GetMovableKnockBackPosition(OriginalPos, pos);

        /// @TODO HIGH PRIORITY
        //sWorld.MoveObject( pTarget, newPos, pTarget->GetFace() );
        pTarget->SetNextMovableTime(next_movable_time);
        Player *pTargetPlayer{nullptr};

        if (pTarget->IsPlayer())
            pTargetPlayer = pTarget->As<Player>();
        else if (pTarget->IsSummon())
        {
            pTargetPlayer = pTarget->As<Summon>()->GetMaster();

            /// @Todo: check Ride object
        }

        /// @Todo: Remove riding
    }

    return next_movable_time;
}

Position Skill::GetMovableKnockBackPosition(Position &OriginalPos, Position &TargetPos)
{
    if (GameContent::IsBlocked(TargetPos.GetPositionX(), TargetPos.GetPositionY()))
        return OriginalPos;
    return TargetPos;
}

void Skill::MAKE_AREA_EFFECT_PROP_BY_FIELD_PROP(bool bIsTrap)
{
    WorldObject *pObj = sMemoryPool.GetObjectInWorld<WorldObject>(m_hTarget);
    if (pObj == nullptr || !pObj->IsFieldProp())
        return;

    auto pProp = pObj->As<FieldProp>();

    auto posTarget = pProp->GetPosition();
    bool bRet      = pProp->UseProp(m_pOwner->As<Player>());

    AddSkillResult(m_vResultList, bRet, 0, 0);

    if (!bRet)
        return;

    if (bIsTrap && m_pOwner->GetTrapHandle() != 0)
    {
        auto pTrap = sMemoryPool.GetObjectInWorld<SkillProp>(m_pOwner->GetTrapHandle());
        if (pTrap != nullptr)
            pTrap->PendRemove();
    }

    int   nMagicPoint = m_pOwner->GetMagicPoint((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
    float fHateRatio  = 1; //m_pOwner->GetHateRatio();

    auto pPtr = SkillProp::Create(m_pOwner->GetHandle(), this, nMagicPoint, fHateRatio);

    if (bIsTrap && pPtr != nullptr)
        m_pOwner->SetTrapHandle(pPtr->GetHandle());

    pPtr->SetCurrentXY(posTarget.GetPositionX(), posTarget.GetPositionY());
    pPtr->SetLayer(posTarget.GetLayer());

    sWorld.AddObjectToWorld(pPtr);
}

void Skill::MAKE_AREA_EFFECT_PROP(Unit *pTarget, bool bIsTrap)
{
    if (pTarget == nullptr)
        return;

    if (bIsTrap && m_pOwner->GetTrapHandle() != 0)
    {
        auto pTrap = sMemoryPool.GetObjectInWorld<SkillProp>(m_pOwner->GetTrapHandle());
        if (pTrap != nullptr)
            pTrap->PendRemove();
    }

    Position pos         = pTarget->GetCurrentPosition(sWorld.GetArTime() + PREDICTION_AIMING_TIME);
    int      nMagicPoint = m_pOwner->GetMagicPoint((ElementalType)GetSkillBase()->GetElementalType(), GetSkillBase()->IsPhysicalSkill(), GetSkillBase()->IsHarmful());
    float    fHateRatio  = 1; //m_pOwner->GetHateRatio();

    auto pPtr = SkillProp::Create(m_pOwner->GetHandle(), this, nMagicPoint, fHateRatio);

    if (bIsTrap && pPtr != nullptr)
        m_pOwner->SetTrapHandle(pPtr->GetHandle());

    pPtr->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
    pPtr->SetLayer(m_pOwner->GetLayer());

    sWorld.AddObjectToWorld(pPtr);
}

