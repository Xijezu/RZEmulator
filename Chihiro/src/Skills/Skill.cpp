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
    if (m_SkillBase != nullptr)
    {
        short et        = m_SkillBase->effect_type;
        if (et == 107
            || et == 202
            || et == 204
            || et == 206
            || et == 212
            || et == 151
            || et == 152
            || et == 108
            || et == 232
            || et == 233
            || et == 263
            || et == 30004
            || et == 30005
            || et == 30009
            || et == 30017
            || et == 30018)
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

int Skill::GetCurrentMPCost()
{
    auto cmp1 = m_SkillBase->cost_mp + (m_nEnhance * m_SkillBase->cost_mp_per_enhance) + (m_nRequestedSkillLevel * m_SkillBase->cost_mp_per_skl);
    auto cmp2 = (m_SkillBase->cost_mp_per_skl_per * m_nRequestedSkillLevel) + m_SkillBase->cost_mp_per;
    return (int)(m_pOwner->GetManaCostRatio((ElementalType)m_SkillBase->elemental, m_SkillBase->is_physical_act != 0, m_SkillBase->is_harmful != 0)
                 * (m_pOwner->GetMana() * cmp2 / 100.0f + cmp1));
}

int Skill::GetCurrentHPCost()
{
    auto cmp1 = ((m_SkillBase->cost_hp_per_skl_per * m_nRequestedSkillLevel) + m_SkillBase->cost_hp_per) * m_pOwner->GetHealth();
    auto cmp2 = m_SkillBase->cost_hp + (m_nRequestedSkillLevel * m_SkillBase->cost_hp_per_skl);
    return (int)(cmp1 / 100.0f + cmp2);
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
                details.hitDamage.damage.target_hp   = skill_result.damage.target_hp;
                details.hitDamage.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE >(skill_result.damage.damage_type);
                details.hitDamage.damage.damage      = skill_result.damage.damage;
                details.hitDamage.damage.flag        = skill_result.damage.flag;
                std::copy(std::begin(skill_result.damage.elemental_damage), std::end(skill_result.damage.elemental_damage),
                          std::begin(details.hitDamage.damage.elemental_damage));
                break;
            case SHT_DAMAGE_WITH_KNOCK_BACK:
                details.hitDamageWithKnockBack.damage.target_hp   = skill_result.damage_kb.target_hp;
                details.hitDamageWithKnockBack.damage.damage_type = static_cast<TS_SKILL__DAMAGE_TYPE >(skill_result.damage_kb.damage_type);
                details.hitDamageWithKnockBack.damage.damage      = skill_result.damage_kb.damage;
                details.hitDamageWithKnockBack.damage.flag        = skill_result.damage_kb.flag;
                std::copy(std::begin(skill_result.damage_kb.elemental_damage), std::end(skill_result.damage_kb.elemental_damage),
                          std::begin(details.hitDamageWithKnockBack.damage.elemental_damage));
                details.hitDamageWithKnockBack.x               = skill_result.damage_kb.x;
                details.hitDamageWithKnockBack.y               = skill_result.damage_kb.y;
                details.hitDamageWithKnockBack.speed           = skill_result.damage_kb.speed;
                details.hitDamageWithKnockBack.knock_back_time = skill_result.damage_kb.knock_back_time;
                break;
            case SHT_RESULT:
                details.hitResult.bResult      = skill_result.result.bResult;
                details.hitResult.success_type = static_cast<TS_SKILL_RESULT_SUCESS_TYPE >(skill_result.result.success_type);
                break;
            case SHT_ADD_HP:
            case SHT_ADD_MP:
                details.hitAddStat = skill_result.hitAddStat;
                break;
            case SHT_ADD_HP_MP_SP:
                details.hitAddHPMPSP.target_hp = skill_result.add_hp_mp_sp.target_hp;
                details.hitAddHPMPSP.nIncHP    = skill_result.add_hp_mp_sp.nIncHP;
                details.hitAddHPMPSP.nIncMP    = skill_result.add_hp_mp_sp.nIncMP;
                details.hitAddHPMPSP.nIncSP    = skill_result.add_hp_mp_sp.nIncSP;
                details.hitAddHPMPSP.target_mp = skill_result.add_hp_mp_sp.target_mp;
                break;
            case SHT_REBIRTH:
                details.hitRebirth.target_hp    = skill_result.rebirth.target_hp;
                details.hitRebirth.nIncHP       = skill_result.rebirth.nIncHP;
                details.hitRebirth.nIncMP       = skill_result.rebirth.nIncMP;
                details.hitRebirth.nRecoveryEXP = skill_result.rebirth.nRecoveryEXP;
                details.hitRebirth.target_mp    = skill_result.rebirth.target_mp;
                break;
            case SHT_RUSH:
                details.hitRush.bResult = skill_result.rush.bResult;
                details.hitRush.x       = skill_result.rush.x;
                details.hitRush.y       = skill_result.rush.y;
                details.hitRush.face    = skill_result.rush.face;
                details.hitRush.speed   = skill_result.rush.speed;
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
    if (sWorld.GetArTime() < m_nFireTime || m_Status == SkillStatus::SS_IDLE)
        return;

    if (m_Status == SkillStatus::SS_CAST)
        m_Status = SkillStatus::SS_FIRE;

    /*if(pTarget != nullptr && !pTarget->IsInWorld())
    {
        m_pOwner->CancelSkill();


    }*/

    auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(m_hTarget);

    if (m_pOwner == nullptr || m_pOwner->GetHealth() == 0)
    {
        m_pOwner->CancelSkill();
        return;
    }

    if (m_Status == SkillStatus::SS_FIRE)
    {
        // First fire
        if (!m_bMultiple || m_nCurrentFire == 0)
        {
            // Set remaining cooltime of the skill
            SetRemainCoolTime(GetSkillCoolTime());

            // @todo:
            // Set remaining cooltime for every other skill in the same cool_time_group_id
            if (m_SkillBase->cool_time_group_id != 0)
            {
//                         v85 = v3->m_pOwner;
//                         *(v1 - 248) = v84->cool_time_group_id;
//                         *(v1 - 252) = &::::_mySkillFunctor::_vftable_;
//                         *(v1 - 244) = v83;
//                         this.m_Owner.EnumActiveSkill((v1 - 252));
            }
            // Send skill list with updated skill cooltimes
            Player *player{nullptr};
            if (m_pOwner->IsPlayer())
            {
                player = m_pOwner->As<Player>();
                if (player != nullptr)
                    Messages::SendSkillList(player, m_pOwner, m_nSkillID);
            }
            else if (m_pOwner->IsSummon())
            {
                auto pSummon = m_pOwner->As<Summon>();
                if (pSummon != nullptr)
                    player = pSummon->GetMaster();
                if (player != nullptr)
                    Messages::SendSkillList(player, m_pOwner, m_nSkillID);
            }
            // @todo: Add aura handling here, see Pyrok/Pseudocode

            // Calculate skill cost
            if (GetCurrentMPCost() > 0)
                m_pOwner->SetMana(m_pOwner->GetMana() - GetCurrentMPCost());
            if (GetCurrentHPCost() > 0)
                m_pOwner->SetHealth(m_pOwner->GetHealth() - GetCurrentHPCost());
            if (m_SkillBase->GetCostEnergy(m_nRequestedSkillLevel) > 0)
                m_pOwner->RemoveEnergy(m_SkillBase->GetCostEnergy(m_nRequestedSkillLevel));

        }

        bool bIsSuccess = false;

        FireSkill(pTarget, bIsSuccess);
        if (pTarget == nullptr)
        {

        }
        broadcastSkillMessage(m_pOwner, 0, 0, 0);

        if (!m_bMultiple || m_nCurrentFire == m_nTotalFire)
        {
            m_nFireTime += m_SkillBase->delay_common;
            m_Status = SkillStatus::SS_COMPLETE;
        }

        if (bIsSuccess)
        {
            Player *pOwner = m_pOwner->IsSummon() ? ((Summon *)m_pOwner)->GetMaster() : (Player *)m_pOwner;
            Messages::SendSkillList(pOwner, m_pOwner, m_nSkillID);
        }
    }

    if (m_Status == SkillStatus::SS_COMPLETE && sWorld.GetArTime() >= m_nFireTime)
    {
        broadcastSkillMessage(m_pOwner, 0, 0, 5);
        m_pOwner->OnCompleteSkill();
        Init();
    }
}

void Skill::FireSkill(Unit *pTarget, bool &bIsSuccess)
{
    auto t = sWorld.GetArTime();
    bool bHandled{true};
    m_vResultList.clear();
    switch (m_SkillBase->effect_type)
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
            /*
            case EF_PHYSICAL_SINGLE_DAMAGE_T1:
            {
                SINGLE_PHYSICAL_DAMAGE_T1(pTarget);
                break;
            }
            case EF_PHYSICAL_SINGLE_DAMAGE_T2:
            {
                SINGLE_PHYSICAL_DAMAGE_T2(pTarget);
                break;
            }
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
            }
            case EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK_OLD:
            {
                SINGLE_PHYSICAL_DAMAGE_T1(pTarget);
                break;
            }
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
            }/*
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
            }
            case EF_CREATE_ITEM:
            {
                CREATE_ITEM(pTarget, bIsSuccess);
                break;
            }
                * /
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
                }
                case EF_AREA_EFFECT_HEAL_BY_FIELD_PROP:
                {
                    MAKE_AREA_EFFECT_PROP_BY_FIELD_PROP( false );
                    break;
                }*/
        case EF_ADD_HP:
        {
            HealingSkillFunctor mySkillFunctor(&m_vResultList);
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
            /*
            case EF_ADD_MP:
            {
                RECOVERY_MP_SKILL_FUNCTOR mySkillFunctor( &m_vResultList );
                process_target( t, mySkillFunctor, pTarget );
                break;
            }*/
        case EF_ADD_HP_BY_ITEM:
        {
            HealingSkillFunctor mySkillFunctor(&m_vResultList, true);
            process_target(t, mySkillFunctor, pTarget);
            break;
        }
            /*
            case EF_ADD_MP_BY_ITEM:
            {
                RECOVERY_MP_SKILL_FUNCTOR mySkillFunctor( &m_vResultList, true );
                mySkillFunctor.nResult = 0;
                process_target( t, mySkillFunctor, pTarget );
                break;
            }*/
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
            bHandled = false;
            break;
    }
    if (!bHandled)
    {
        switch (m_SkillBase->id)
        {
            case SKILL_CREATURE_TAMING:
                CREATURE_TAMING();
                bHandled = true;
                break;
            case SKILL_RETURN:
            case SKILL_TOWN_PORTAL:
                TOWN_PORTAL();
                bHandled = true;
                break;
            case SKILL_GAIA_FORCE_SAVING:
                ADD_ENERGY();
                bHandled = true;
                break;
            default:
                auto result = string_format("Unknown skill casted - ID %u, effect_type %u", m_SkillBase->id, m_SkillBase->effect_type);
                NG_LOG_INFO("skill", "%s", result.c_str());
                if (m_pOwner->IsPlayer())
                    Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Player>(), result);
                else if (m_pOwner->IsSummon())
                {
                    Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Summon>()->GetMaster(), result);
                }
                break;
        }
    }

    if (bHandled)
    {
        PostFireSkill(pTarget);
    }

    if (m_SkillBase->is_harmful != 0)
    {
        auto mob = pTarget->As<Monster>();
        if (mob != nullptr && mob->IsMonster() && mob->IsCastRevenger())
        {
            mob->AddHate(m_pOwner->GetHandle(), 1, true, true);
        }
    }
    bIsSuccess = bHandled;
}

void Skill::PostFireSkill(Unit *pTarget)
{
    std::vector<Unit *> vNeedStateList{ };
    for (const auto     &sr : m_vResultList)
    {
        auto pDealTarget = sMemoryPool.GetObjectInWorld<Unit>(sr.hTarget);
        if (pDealTarget != nullptr && pDealTarget->GetHealth() != 0)
        {

            // Add State Result begin)
            if (sr.type == 0 || sr.type == 1 || sr.type == 2 || sr.type == 20 || sr.type == 21 || sr.type == 22 || sr.type == 10)
            {

                if (pDealTarget != nullptr && pDealTarget->GetHealth() != 0
                    && m_SkillBase->effect_type != EF_ADD_STATE
                    && m_SkillBase->effect_type != EF_ADD_REGION_STATE
                    && m_SkillBase->effect_type != EF_ADD_STATE_BY_USING_ITEM
                    && m_SkillBase->effect_type != EF_PHYSICAL_SINGLE_DAMAGE_WITH_SHIELD
                    && m_SkillBase->effect_type != EF_ADD_STATE_BY_SELF_COST
                    && m_SkillBase->effect_type != EF_ADD_REGION_STATE_BY_SELF_COST
                    && m_SkillBase->effect_type != EF_REMOVE_STATE_GROUP
                    && m_SkillBase->state_id != 0
                    && (sr.damage.flag ^ 2) != 0)
                {
                    vNeedStateList.emplace_back(pDealTarget);
                }
            }
            // Add State Result end
            int nDamage{0};
            if (!m_pOwner->IsMonster() && m_SkillBase->effect_type != EF_REGION_REMOVE_HATE && m_SkillBase->effect_type != EF_REMOVE_HATE)
            {
                if (sr.type != SRT_DAMAGE && sr.type != SRT_MAGIC_DAMAGE && sr.type != SRT_DAMAGE_WITH_KNOCK_BACK)
                {
                    if (sr.type == SRT_ADD_HP || sr.type == SRT_ADD_MP)
                    {
//                                                 nDamage = sr.*(&std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v3 + 104))->end.z
//                                                       + 1);
                    }
                    else if (sr.type == SRT_ADD_HP_MP_SP)
                    {
//                                                 v54 = *(&std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v3 + 104))->end.z
//                                                       + 1);
//                                                 v55 = *(&std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v3 + 104))->end.face
//                                                       + 1)
//                                                     + v54;
//                                                 nDamage = *(&std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v3 + 104))->end_time
//                                                       + 1)
//                                                     + v55;
                    }
                }
                else
                {
                    nDamage = sr.damage.damage;
                }

                float hm      = 1.0f; // (m_pOwner->GetMagicalHateMod((ElementalType)m_SkillBase.elemental, m_SkillBase.is_spell_act == 0, m_SkillBase.is_harmful != 0)
                // * (this.m_SkillBase.GetHatePoint(m_nRequestedSkillLevel, nDamage, m_nEnhance) * m_pOwner->m_fHateRatio));
                auto  HateMod = m_pOwner->GetHateMod(m_SkillBase->is_physical_act == 0 ? 2 : 1, m_SkillBase->is_harmful != 0);
                nDamage     = (int)((hm + HateMod.second) * HateMod.first);
                if (nDamage == 0)
                    nDamage = 1;

                if (pDealTarget->IsMonster())
                {
                    pDealTarget->As<Monster>()->AddHate(m_pOwner->GetHandle(), nDamage, true, true);
                }
            }
        }
    }

    if (!vNeedStateList.empty())
    {
        StateSkillFunctor fn{&m_vResultList};
        auto              t = sWorld.GetArTime();
        for (auto         &unit : vNeedStateList)
        {
            process_target(t, fn, unit);
        }
    }
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

    sWorld.AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
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

    sWorld.AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
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

    sWorld.AddSkillDamageResult(m_vResultList, 1, (uint8)elemental_type, damage, pTarget->GetHandle());

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
        sWorld.AddSkillResult(m_vResultList, fp->UseProp(m_pOwner->As<Player>()), 0, 0);
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

    sWorld.AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, Damage, pTarget->GetHandle());
    m_nFireTime = (uint)((m_SkillBase->var[8] * 100) + m_nFireTime);
}

void Skill::TOGGLE_AURA(Unit *pTarget)
{
    m_pOwner->ToggleAura(this);

    if (m_SkillBase->target == 21)
    {
        // TODO: Party functor
    }
    sWorld.AddSkillResult(m_vResultList, true, 0, m_pOwner->GetHandle());
}

void Skill::MANA_SKILL_FUNCTOR(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    bool v10           = m_SkillBase->is_physical_act != 0;
    //auto attack_point = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto magic_point   = m_pOwner->m_Attribute.nMagicPoint;
    auto target_max_hp = pTarget->GetMaxHealth();

    auto heal = magic_point *
                (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel))
                + m_SkillBase->var[2] + (m_SkillBase->var[3] * m_nRequestedSkillLevel) + (m_nEnhance * m_SkillBase->var[6])
                + target_max_hp * (m_SkillBase->var[4] + (m_SkillBase->var[5] * m_nRequestedSkillLevel) + m_SkillBase->var[7] * m_nRequestedSkillLevel);

    heal = pTarget->MPHeal((int)heal);

    SkillResult skillResult{ };
    skillResult.type                   = SRT_ADD_MP;
    skillResult.hTarget                = pTarget->GetHandle();
    skillResult.hitAddStat.target_stat = pTarget->GetHealth();
    skillResult.hitAddStat.nIncStat    = (int)heal;
    m_vResultList.emplace_back(skillResult);
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
    skillResult.type                 = SRT_REBIRTH;
    skillResult.hTarget              = pTarget->GetHandle();
    skillResult.rebirth.target_hp    = pTarget->GetHealth();
    skillResult.rebirth.nIncHP       = std::max((int)(pTarget->GetHealth() - prev_hp), 0);
    skillResult.rebirth.nIncMP       = std::max((int)(pTarget->GetMana() - prev_mp), 0);
    skillResult.rebirth.nRecoveryEXP = 0;
    skillResult.rebirth.target_mp    = (int16)pTarget->GetMana();
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
    nDamage = GameContent::EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(ct),
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
        sWorld.AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, target->GetHandle());
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

    nDamage = GameContent::EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(ct),
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
        sWorld.AddSkillDamageResult(m_vResultList, 1, (uint8)m_SkillBase->elemental, damage, unit->GetHandle());
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
// .text:004C4967 pos             = ArPosition ptr -90h
// .text:004C4967 var_80          = ArPosition ptr -80h
// .text:004C4967 var_70          = dword ptr -70h
// .text:004C4967 var_5C          = dword ptr -5Ch
// .text:004C4967 fo              = StructSkill::process_target::__l23::myPartyFunctor ptr -48h
// .text:004C4967 _fo             = GuildManager::GuildFunctor ptr -34h
// .text:004C4967 this            = StructSkill::process_target::__l45::myPartyFunctor ptr -20h
// .text:004C4967 pSummon         = dword ptr -0Ch
// .text:004C4967 var_4           = dword ptr -4
// .text:004C4967 t               = dword ptr  8
// .text:004C4967 functor         = dword ptr  0Ch
// .text:004C4967 pTargetPlayer   = dword ptr  10h
//
// Data           :   enregistered ecx, Object Ptr, Type: struct StructSkill * const, this
// Data           :   ebp Relative, [00000008], Param, Type: unsigned long, t
// Data           :   ebp Relative, [0000000C], Param, Type: struct SKILL_TARGET_FUNCTOR &, functor
// Data           :   ebp Relative, [00000010], Param, Type: struct StructCreature *, pTarget
// Typedef        :   StructSkill::process_target::__l23::myPartyFunctor, Type: struct StructSkill::process_target::__l23::myPartyFunctor
// Data           :   ebp Relative, [FFFFFFB8], Local, Type: struct StructSkill::process_target::__l23::myPartyFunctor, fo
// Typedef        :   StructSkill::process_target::__l34::myGuildFunctor, Type: struct StructSkill::process_target::__l34::myGuildFunctor
// Data           :   ebp Relative, [FFFFFFCC], Local, Type: struct StructSkill::process_target::__l34::myGuildFunctor, fo
// Typedef        :   StructSkill::process_target::__l45::myPartyFunctor, Type: struct StructSkill::process_target::__l45::myPartyFunctor
// Data           :   ebp Relative, [FFFFFFE0], Local, Type: struct StructSkill::process_target::__l45::myPartyFunctor, fo
// Data           :   ebp Relative, [FFFFFF70], Local, Type: struct ArPosition, pos
// Data           :   ebp Relative, [FFFFFFF4], Local, Type: struct StructSummon *, pSummon
// Typedef        :   StructSkill::process_target::__l69::myPartyFunctor, Type: struct StructSkill::process_target::__l69::myPartyFunctor
// Data           :   ebp Relative, [FFFFFFA4], Local, Type: struct StructSkill::process_target::__l69::myPartyFunctor, fo
// Data           :   ebp Relative, [FFFFFF80], Local, Type: struct ArPosition, pos
// Data           :   ebp Relative, [00000010], Local, Type: struct StructPlayer *, pTargetPlayer
// Typedef        :   StructSkill::process_target::__l106::myPartyFunctor, Type: struct StructSkill::process_target::__l106::myPartyFunctor
// Data           :   ebp Relative, [FFFFFF90], Local, Type: struct StructSkill::process_target::__l106::myPartyFunctor, fo

//             v4 = this;
//             v5 = this.m_SkillBase;
//             v6 = this.m_SkillBase.target;

    switch (m_SkillBase->target)
    {
        case TARGET_TYPE::TARGET_TARGET: // 1
        case TARGET_TYPE::TARGET_TARGET_EXCEPT_CASTER: // 6
            if (m_SkillBase->target == TARGET_TYPE::TARGET_TARGET_EXCEPT_CASTER && pTarget == m_pOwner)
                return;
            if (m_SkillBase->is_need_target != 0)
            {
                if (pTarget == nullptr)
                    return;
                fn.onCreature(this, t, m_pOwner, pTarget);
            }
            else
            {
                fn.onCreature(this, t, m_pOwner, m_pOwner);
            }
            return;

        case TARGET_TYPE::TARGET_REGION_WITH_TARGET: // 2
        case TARGET_TYPE::TARGET_REGION_WITHOUT_TARGET: // 3
        case TARGET_TYPE::TARGET_REGION: // 4
            if (pTarget == nullptr)
                return;
            fn.onCreature(this, t, m_pOwner, pTarget);
            return;
        case TARGET_TYPE::TARGET_PARTY: // 21
            /*
    if ( !(unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[1].onProcess)() )
      return;
    if ( !(unsigned __int8)((int (__thiscall *)(_DWORD))pTarget->vfptr[1].onProcess)(pTarget) )
      return;
    v15 = v4->m_pOwner;
    if ( v15 != pTarget )
    {
      v16 = *((_DWORD *)&v15[1].m_nRefCount + 1);
      if ( v16 )
      {
        if ( v16 != *((_DWORD *)&pTarget[1].m_nRefCount + 1) )
          return;
      }
    }
    `StructSkill::process_target'::`23'::myPartyFunctor::myPartyFunctor(&fo, v4, t, v15, functor);
    v18 = *(_DWORD *)(v17 + 4228);
    if ( !v18 )
    {
      (**(void (__thiscall ***)(_DWORD))v17)(v17);
      v10 = (void (__thiscall **)(_DWORD))fo.vfptr;
      v11 = &fo;
      goto LABEL_16;
    }
    v19 = (StructSkill::process_target::__l45::myPartyFunctor *)&fo;
LABEL_26:
    v48 = v19;
    v47 = v18;
    goto LABEL_27;
             */
        case TARGET_TYPE::TARGET_GUILD: // 22
            /*
    if ( (unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[1].onProcess)() )
    {
      if ( (unsigned __int8)((int (__thiscall *)(_DWORD))pTarget->vfptr[1].onProcess)(pTarget) )
      {
        v7 = v4->m_pOwner;
        if ( v7 == pTarget || (v8 = v7[1].quadTreeItem.y) == 0 || v8 == pTarget[1].quadTreeItem.y )
        {
          `StructSkill::process_target'::`34'::myGuildFunctor::myGuildFunctor(&v53, v4, t, v7, functor);
          if ( !*(_DWORD *)(v9 + 4240) )
          {
            (**(void (__thiscall ***)(_DWORD))v9)(v9);
            v10 = (void (__thiscall **)(_DWORD))v53.vfptr;
            v11 = &v53;
LABEL_16:
            (*v10)(v11);
            return;
          }
          v12 = *(_DWORD *)(v9 + 4240);
          v13 = GuildManager::GetInstance();
          v14 = GuildManager::DoEachMember(v13, v12, (GuildManager::GuildFunctor *)&v53.vfptr);
          goto LABEL_28;
        }
      }
    }
             */
        case TARGET_TYPE::TARGET_ATTACKTEAM: // 23
            /*
    if ( !(unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[1].onProcess)() )
      return;
    if ( !(unsigned __int8)((int (__thiscall *)(_DWORD))pTarget->vfptr[1].onProcess)(pTarget) )
      return;
    v21 = v4->m_pOwner;
    if ( v21 != pTarget )
    {
      v22 = *((_DWORD *)&v21[1].m_nRefCount + 1);
      if ( v22 )
      {
        if ( v22 != *((_DWORD *)&pTarget[1].m_nRefCount + 1) )
          return;
      }
    }
    `StructSkill::process_target'::`45'::myPartyFunctor::myPartyFunctor(&v54, v4, t, v21, functor);
    v23 = *((_DWORD *)&v21[1].m_nRefCount + 1);
    v24 = *((_DWORD *)&v21[1].m_nRefCount + 1);
    v25 = PartyManager::GetInstance();
    v26 = PartyManager::GetAttackTeamLeadPartyID(v25, v24);
    if ( v26 )
    {
      v27 = v26;
      v28 = PartyManager::GetInstance();
      v14 = PartyManager::DoEachAttackTeamMember(v28, v27, (PartyManager::PartyFunctor *)&v54.vfptr);
      goto LABEL_28;
    }
    if ( !v23 )
    {
      v29 = v21->vfptr->GetHandle((ArSchedulerObject *)v21);
      v54.vfptr->operator()((PartyManager::PartyFunctor *)&v54, v29);
      v4->m_nTargetCount = 1;
      return;
    }
    v48 = &v54;
    v47 = v23;
LABEL_27:
    v20 = PartyManager::GetInstance();
    v14 = PartyManager::DoEachMember(v20, v47, (PartyManager::PartyFunctor *)&v48->vfptr);
LABEL_28:
    v4->m_nTargetCount = v14;
    return;
             */
        case TARGET_TYPE::TARGET_SUMMON: // 31
        {
            Summon *pSummon{nullptr};
            if (pTarget != nullptr && pTarget->IsInWorld())
            {
                pSummon = pTarget->As<Summon>();
            }
            else
            {
                /*
                 if ( !(unsigned __int8)((int (__thiscall *)(StructCreature *))pTarget->vfptr[1].onProcess)(pTarget) )
                   return;
                 v44 = pTarget[1].m_anWear[22];
                 if ( !v44 || !v44->bIsInWorld )
                   return;
                 pSummon = (StructSummon *)pTarget[1].m_anWear[22];
                 */
            }
            if (pSummon == nullptr)
                return;

            if (pSummon->GetExactDist2d(pTarget) > 12 * m_SkillBase->valid_range)
                return;

            fn.onCreature(this, t, m_pOwner, pSummon);
            return;
        }
        case TARGET_TYPE::TARGET_PARTY_SUMMON: // 32
            /*
      if ( !(unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[1].onProcess)() )
        return;
      `StructSkill::process_target'::`69'::myPartyFunctor::myPartyFunctor(&v51, v4, t, v4->m_pOwner, functor);
      v18 = *(_DWORD *)(v43 + 4228);
      if ( !v18 )
      {
        (**(void (__thiscall ***)(_DWORD))v43)(v43);
        v10 = (void (__thiscall **)(_DWORD))v51.vfptr;
        v11 = &v51;
        goto LABEL_16;
      }
      v19 = (StructSkill::process_target::__l45::myPartyFunctor *)&v51;
      goto LABEL_26;
    }
             */
        case TARGET_TYPE::TARGET_SELF_WITH_SUMMON: // 45
            /*
        v37 = pTarget;
          if ( !(unsigned __int8)((int (__thiscall *)(StructCreature *))pTarget->vfptr[1].onProcess)(pTarget) )
          {
            if ( !(unsigned __int8)pTarget->vfptr[2].GetHandle((ArSchedulerObject *)pTarget) )
              return;
            v37 = *(StructCreature **)&pTarget[1].bIsRegionChanging;
            if ( !v37 )
              return;
          }
          pTargetPlayer = (StructPlayer *)v37;
          if ( !v37 || !v37->bIsInWorld )
            return;
          ((void (__stdcall *)(StructSkill *, unsigned int, StructCreature *, StructCreature *))functor->vfptr->onCreature)(
            v4,
            t,
            v4->m_pOwner,
            v37);
          v38 = (int)&v4->m_pOwner->mv;
          v56.x = *(float *)v38;
          v38 += 4;
          v56.y = *(float *)v38;
          v38 += 4;
          v56.z = *(float *)v38;
          v56.face = *(float *)(v38 + 4);
          v39 = (StructCreature *)&pTargetPlayer->m_pMainSummon->vfptr;
          if ( v39
            && v39->bIsInWorld
            && v39->m_nHP
            && (pSummon = (StructSummon *)(12 * v4->m_pSkillBase->valid_range),
                *(float *)&pSummon = (double)(signed int)pSummon,
                v57 = *(float *)&pSummon,
                v40 = ArPosition::GetDistance(&v56, (ArPosition *)&v39->mv.x),
                v40 <= v57) )
          {
            v34 = functor;
            functor->vfptr->onCreature(functor, v4, t, v4->m_pOwner, v39);
          }
          else
          {
            v34 = functor;
          }
          v41 = pTargetPlayer->m_pSubSummon;
          if ( !v41 )
            return;
          if ( !v41->bIsInWorld )
            return;
          if ( !v41->m_nHP )
            return;
          *(float *)&functora = (double)(12 * v4->m_pSkillBase->valid_range);
          v57 = *(float *)&functora;
          v42 = ArPosition::GetDistance(&v56, (ArPosition *)&v41->mv.x);
          if ( v42 > v57 )
            return;
          v50 = pTargetPlayer->m_pSubSummon;
        }
        v34->vfptr->onCreature(v34, v4, t, v4->m_pOwner, (StructCreature *)v50);
        return;
             */
        case TARGET_TYPE::TARGET_MASTER: // 101
            /*
            if ( (unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[2].GetHandle)() )
            {
              v36 = *(_DWORD *)&v4->m_pOwner[1].bIsRegionChanging;
              if ( v36 )
              {
                if ( *(_BYTE *)(v36 + 68) )
                  ((void (__stdcall *)(StructSkill *, unsigned int, StructCreature *, _DWORD))functor->vfptr->onCreature)(
                    v4,
                    t,
                    v4->m_pOwner,
                    *(_DWORD *)&v4->m_pOwner[1].bIsRegionChanging);
              }
            }
            return;
             */
        case TARGET_TYPE::TARGET_SELF_WITH_MASTER: // 102
/*
          if ( v33 != 1 )
            return;
          if ( !(unsigned __int8)((int (*)(void))v4->m_pOwner->vfptr[2].GetHandle)() )
            return;
          if ( !v4->m_pOwner->bIsInWorld )
            return;
          v34 = functor;
          functor->vfptr->onCreature(functor, v4, t, v4->m_pOwner, v4->m_pOwner);
          v35 = *(StructSummon **)&v4->m_pOwner[1].bIsRegionChanging;
          if ( !v35
            || !v35->bIsInWorld
            || ArPosition::GetDistance((ArPosition *)&v35->mv.x, (ArPosition *)&v4->m_pOwner->mv.x) > 525.0 )
          {
            return;
          }
          v50 = v35;
 */
        default:
            auto result = string_format("TARGET_TYPE %d in process_target not handled yet.", m_SkillBase->target);
            if (m_pOwner->IsPlayer())
                Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Player>(), result);
            else if (m_pOwner->IsSummon())
            {
                Messages::SendChatMessage(50, "@SYSTEM", m_pOwner->As<Summon>()->GetMaster(), result);
            }
            NG_LOG_DEBUG("skill", "%s", result.c_str());
            return;
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
                sWorld.AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
                return;
            }
            nDecHP = pSummon->GetHealth();
            pSummon->damage(m_pOwner, nDecHP, false);
            DamageInfo info{ };
            info.nDamage   = nDecHP;
            info.target_hp = pSummon->GetHealth();
            sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, ElementalType::TYPE_NONE, info, pSummon->GetHandle());
        }
            break;
        case EF_ADD_HP_MP_BY_SUMMON_DAMAGE:
        {
            if (!m_pOwner->IsPlayer())
                return;

            auto pSummon = m_pOwner->As<Player>()->m_pMainSummon;
            if (pSummon == nullptr || pSummon->GetHealth() == 0)
            {
                sWorld.AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
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
                sWorld.AddSkillResult(m_vResultList, false, static_cast<int32_t >(SkillResult::SUMMON_DEAD), pTarget->GetHandle());
                return;
            }

            nDecHP = GetVar(0) + GetVar(1) * GetRequestedSkillLevel() + GetVar(4) * GetSkillEnhance();
            nDecMP = GetVar(2) + GetVar(3) * GetRequestedSkillLevel() + GetVar(5) * GetSkillEnhance();

            pSummon->AddHealth(-nDecHP);
            pSummon->AddMana(-nDecMP);

            SkillResult skillResult{ };
            skillResult.type                   = static_cast<uint8_t >(TS_SKILL__HIT_TYPE::SHT_ADD_HP_MP_SP);
            skillResult.hTarget                = pSummon->GetHandle();
            skillResult.add_hp_mp_sp.target_hp = pSummon->GetHealth();
            skillResult.add_hp_mp_sp.target_mp = static_cast<int16_t >(pSummon->GetMana());
            skillResult.add_hp_mp_sp.nIncHP    = -nDecHP;
            skillResult.add_hp_mp_sp.nIncMP    = -nDecMP;
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
    skillResult.add_hp_mp_sp.target_hp = pTarget->GetHealth();
    skillResult.add_hp_mp_sp.target_mp = static_cast<int16_t >(pTarget->GetMana());
    skillResult.add_hp_mp_sp.nIncHP    = nIncHP;
    skillResult.add_hp_mp_sp.nIncMP    = nIncMP;

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

        sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType >(elemental_type), Damage, pTarget->GetHandle());
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

    nDamage = GameContent::EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), bTargetOrigin, fEffectLength, nRegionType, fRegionProperty, nDamage, true, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);
    for (int i = 0; i < vTargetList.size(); i++)
    {
        for (auto &target : vTargetList)
        {
            if (target->GetHealth() == 0)
                continue;

            DamageInfo Damage = target->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - target->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
            sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, (ElementalType)elemental_type, Damage, target->GetHandle());
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

    sWorld.AddSkillResult(m_vResultList, bSuccess, SkillResult::ADD_HATE, pTarget->GetHandle());
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

    nDamage = GameContent::EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), bTargetOrigin, fEffectLength, nRegionType, fRegionProperty, nDamage, true, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);

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
                sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
            }
            else
            {
                sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
            }
        }
        else
        {
            sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pDealTarget->GetHandle());
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

    nDamage = GameContent::EnumSkillTargetsAndCalcDamage(m_pOwner->GetCurrentPosition(t), m_pOwner->GetLayer(), pTarget->GetCurrentPosition(t), GetVar(8) != 0, fEffectLength, GetVar(7), GetVar(10), nDamage, GetVar(9) != 0, m_pOwner, GetVar(5), GetVar(6), vTargetList, true);

    for (auto &pDealTarget : vTargetList)
    {
        DamageInfo Damage = pDealTarget->DealPhysicalSkillDamage(m_pOwner, nDamage, (ElementalType)elemental_type, GetSkillBase()->GetHitBonus(GetSkillEnhance(), m_pOwner->GetLevel() - pDealTarget->GetLevel()), GetSkillBase()->GetCriticalBonus(GetRequestedSkillLevel()), 0);
        sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType >(elemental_type), Damage, pDealTarget->GetHandle());
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
    sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pTarget->GetHandle());

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

    sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, static_cast<ElementalType>(elemental_type), Damage, pTarget->GetHandle());

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

    sWorld.AddSkillDamageResult(m_vResultList, SkillResult::DAMAGE, elemental_type, Damage, pTarget->GetHandle());

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
