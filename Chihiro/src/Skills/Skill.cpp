/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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
#include "Unit.h"
#include "Database/DatabaseEnv.h"
#include "World.h"
#include "ObjectMgr.h"
#include "MemPool.h"
#include "ClientPackets.h"
#include "SkillFunctor.h"
#include "Messages.h"
#include "FieldPropManager.h"
#include "RegionContainer.h"

Skill::Skill(Unit *pOwner, int64 _uid, int _id) : m_nErrorCode(0)
{
    m_nSkillUID = _uid;
    m_nSkillID = _id;
    m_pOwner = pOwner;
    cool_time = 0;
    m_nSummonID = 0;
    m_nSkillLevel = 0;
    m_SkillBase = sObjectMgr->GetSkillBase(m_nSkillID);
    if(m_SkillBase != nullptr)
    {
        short et = m_SkillBase->effect_type;
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
    m_nErrorCode = 0;
    m_Status = SkillStatus::SS_IDLE;
    m_nCastTime = 0;
    m_nCastingDelay = 0;
    m_nFireTime = 0;
    m_nRequestedSkillLevel = 0;
    m_hTarget = 0;
    m_nCurrentFire = 0;
    m_nTotalFire = 0;
    m_nTargetCount = 1;
    m_nFireCount = 1;
    m_targetPosition.Relocate(0, 0, 0, 0);
    m_targetPosition.SetLayer(0);
}

void Skill::SetRequestedSkillLevel(int nLevel)
{
    int tl = m_nSkillLevel + m_nSkillLevelAdd;
    if(nLevel <= tl)
        tl = nLevel;
    m_nRequestedSkillLevel = (uint8)tl;
}


void Skill::DB_InsertSkill(Unit *pUnit, int64 skillUID, int skill_id, int skill_level, int cool_time)
{
    auto owner_uid = pUnit->GetUInt32Value(UNIT_FIELD_UID);
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_REP_SKILL);
    stmt->setInt64(0, skillUID);
    stmt->setInt32(1, pUnit->GetSubType() == ST_Player ? owner_uid : 0);
    stmt->setInt32(2, pUnit->GetSubType() == ST_Summon ? owner_uid : 0);
    stmt->setInt32(3, skill_id);
    stmt->setInt32(4, skill_level);
    stmt->setInt32(5, cool_time);
    CharacterDatabase.Execute(stmt);
}

int Skill::Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    m_vResultList.clear();
    auto current_time = sWorld->GetArTime();
    uint delay        = 0xffffffff;

    if(m_nSkillLevel + m_nSkillLevelAdd < nSkillLevel)
        nSkillLevel = m_nSkillLevel + m_nSkillLevelAdd;
    SetRequestedSkillLevel(nSkillLevel);
    if (!CheckCoolTime(current_time))
    {
        return TS_RESULT_COOL_TIME;
    }

    if (m_SkillBase->vf_is_not_need_weapon == 0)
    {
        bool bOk{false};
        if (m_SkillBase->vf_shield_only != 0)
            bOk = m_pOwner->IsWearShield();
        else
            bOk = m_SkillBase->IsUseableWeapon(m_pOwner->GetWeaponClass());
        if (!bOk)
        {
            Init();
            return TS_RESULT_LIMIT_WEAPON;
        }
    }

    /* ******* SKILL COST CALCULATION ******* */
    int   nHP       = m_pOwner->GetHealth();
    int   nMP       = m_pOwner->GetMana();
    float decHP     = 0;
    float decMP     = 0;
    int hp_cost = 0;
    int mana_cost = 0;

    if (m_pOwner->GetMaxHealth() != 0)
        decHP = 100 * nHP / m_pOwner->GetMaxHealth();
    if (m_pOwner->GetMaxMana() != 0)
        decMP = 100 * nMP / m_pOwner->GetMaxMana();

    if ((m_SkillBase->effect_type == EF_TOGGLE_AURA || m_SkillBase->effect_type == EF_TOGGLE_DIFFERENTIAL_AURA) && m_pOwner->IsActiveAura(this))
    {
        mana_cost = 0;
    }
    else
    {
        auto cmp1 = m_SkillBase->cost_mp + (m_nEnhance * m_SkillBase->cost_mp_per_enhance) + (m_nRequestedSkillLevel * m_SkillBase->cost_mp_per_skl);
        auto cmp2 = (m_SkillBase->cost_mp_per_skl_per * m_nRequestedSkillLevel) + m_SkillBase->cost_mp_per;
        mana_cost = (int)(m_pOwner->GetManaCostRatio((ElementalType)m_SkillBase->elemental, m_SkillBase->is_physical_act != 0, m_SkillBase->is_harmful != 0)
                          * (m_pOwner->GetMana() * cmp2 / 100.0f + cmp1));
    }

    if(m_SkillBase->need_hp <= 0)
    {
        if(m_SkillBase->need_hp < 0 && decHP > -m_SkillBase->need_hp)
        {
            Init();
            return TS_RESULT_NOT_ACTABLE;
        }
    }
    else
    {
        if(decHP < m_SkillBase->need_hp)
        {
            Init();
            return TS_RESULT_NOT_ENOUGH_HP;
        }
    }
    if(decMP < m_SkillBase->need_mp || nMP < mana_cost)
    {
        Init();
        return TS_RESULT_NOT_ENOUGH_MP;
    }

    if(m_pOwner->GetLevel() < m_SkillBase->need_level)
    {
        Init();
        return TS_RESULT_NOT_ENOUGH_LEVEL;
    }
    if(m_pOwner->GetJP() < m_SkillBase->cost_jp + m_nEnhance * m_SkillBase->cost_jp_per_enhance)
    {
        Init();
        return TS_RESULT_NOT_ENOUGH_JP;
    }


    /* ******* SKILL COST CALCULATION ******* */

    m_Status = SkillStatus::SS_CAST;
    switch(m_SkillBase->effect_type)
    {
        case SKILL_EFFECT_TYPE::EF_SUMMON:
            m_nErrorCode = PrepareSummon(nSkillLevel, handle, pos, current_time);
            break;
        case SKILL_EFFECT_TYPE::EF_ACTIVATE_FIELD_PROP:
        case SKILL_EFFECT_TYPE::EF_REGION_HEAL_BY_FIELD_PROP:
        case SKILL_EFFECT_TYPE::EF_AREA_EFFECT_HEAL_BY_FIELD_PROP:
        {
            m_nErrorCode = TS_RESULT_NOT_ACTABLE;
            if(m_pOwner->IsPlayer())
            {
                auto pProp = sMemoryPool->GetObjectInWorld<FieldProp>(handle);
                if(pProp != nullptr && pProp->m_pFieldPropBase->nActivateSkillID == m_SkillBase->id )
                {
                    if(pProp->m_nUseCount >= 1 && pProp->IsUsable(dynamic_cast<Player*>(m_pOwner)))
                    {
                        delay = pProp->GetCastingDelay();
                        if (sRegion->IsVisibleRegion(m_pOwner, pProp) == 0)
                            return TS_RESULT_NOT_ACTABLE;
                        pProp->Cast();
                        m_nErrorCode = TS_RESULT_SUCCESS;
                    }
                }
            }
        }
            break;
        default:
            break;
    } // END SWITCH

    m_pOwner->SetMana(nMP - mana_cost);

    // Check for Creature Taming since it doesn't have an effect type
    if (m_SkillBase->id == SkillId::CreatureTaming)
    {
        m_nErrorCode = PrepareTaming(nSkillLevel, handle, pos, current_time);
    }

    auto m_nOriginalDelay = delay;
    if (delay == 0xffffffff)
    {
        delay = m_SkillBase->GetCastDelay(nSkillLevel, 0);
        if (m_nSkillID > 0 || m_nSkillID < -5)
        {
            if (delay < 0)
                delay = (uint)(delay + 4294967296);
            delay     = (uint)(delay / (m_pOwner->m_Attribute.nCastingSpeed / 100.0f));
            delay     = (uint)((float)delay * (m_pOwner->GetCastingMod((ElementalType)m_SkillBase->elemental,
                                                                       m_SkillBase->is_physical_act == 1, m_SkillBase->is_harmful != 0,
                                                                       m_nOriginalDelay)));
        }
    }
    m_nCastingDelay       = m_nOriginalDelay;
    m_hTarget             = handle;
    m_nCastTime           = current_time;
    m_nFireTime           = current_time + delay;

    auto error_code = m_nErrorCode;

    if (m_nErrorCode == TS_RESULT_SUCCESS)
    {
        broadcastSkillMessage(m_pOwner, 0, mana_cost, 1);
    }
    else
    {
        Init();
    }

    return error_code;
}

uint16 Skill::PrepareSummon(int nSkillLevel, uint handle, Position pos, uint current_time)
{
    //auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if (item == nullptr || item->m_pItemBase == nullptr || item->m_pItemBase->group != ItemGroup::SummonCard || item->m_Instance.OwnerHandle != m_pOwner->GetHandle())
    {
        return TS_RESULT_NOT_ACTABLE;
    }
    auto player = dynamic_cast<Player *>(m_pOwner);
    if (player == nullptr)
        return TS_RESULT_NOT_ACTABLE;
    int i = 0;
    while (item->m_nHandle != player->m_aBindSummonCard[i]->m_nHandle)
    {
        ++i;
        if (i >= 6)
            return TS_RESULT_NOT_ACTABLE;
    }
    auto summon = item->m_pSummon;
    if (summon == nullptr)
        return TS_RESULT_NOT_EXIST;
    if (summon->IsInWorld())
        return TS_RESULT_NOT_ACTABLE;
    Position tmpPos = player->GetCurrentPosition(current_time);
    summon->SetCurrentXY(tmpPos.GetPositionX(), tmpPos.GetPositionY());
    summon->SetLayer(player->GetLayer());
    summon->StopMove();
    do
    {
        do
        {
            summon->AddNoise(rand32(), rand32(), 70);
            m_targetPosition = summon->GetCurrentPosition(current_time);
        } while (pos.GetPositionX() == m_targetPosition.GetPositionX() && pos.GetPositionY() == m_targetPosition.GetPositionY());
    } while (tmpPos.GetExactDist2d(&m_targetPosition) < 24.0f);
    return TS_RESULT_SUCCESS;
}

void Skill::assembleMessage(XPacket &pct, int nType, int cost_hp, int cost_mp) {
    pct << (uint16)m_SkillBase->id;
    pct << (uint8)m_nRequestedSkillLevel;
    pct << (uint32)m_pOwner->GetHandle();
    pct << (uint32)m_hTarget;
    pct << m_targetPosition.GetPositionX();
    pct << m_targetPosition.GetPositionY();
    pct << m_targetPosition.GetPositionZ();
    pct << (uint8)m_targetPosition.GetLayer();
    pct << (uint8)nType;
    pct << (int16)cost_hp;
    pct << (int16)cost_mp;
    pct << (int)m_pOwner->GetHealth();
    pct << (int16)m_pOwner->GetMana();

    if (nType != SkillState::ST_Fire) {
        if (nType <= SkillState::ST_Fire)
            return;
        if (nType <= SkillState::ST_CastingUpdate || nType == SkillState::ST_Complete) {
            pct << (uint32)(m_nFireTime - m_nCastTime);
            pct << (uint16)m_nErrorCode;
            pct.fill("", 3);
            return;
        }
        if (nType == SkillState::ST_Cancel) {
            pct << (uint8)0;
            return;
        }
        if (nType != SkillState::ST_RegionFire)
            return;
    }

    pct << (uint8)(m_bMultiple ? 1 : 0);
    pct << (float)0;
    pct << (uint8)m_nTargetCount;
    pct << (uint8)m_nFireCount;
    pct << (uint16)m_vResultList.size();

    int fillSize = 45;

    if (!m_vResultList.empty()) {
        for (const auto &sr : m_vResultList) {
            // Odd fixed padding for the skill result
            auto pos = pct.wpos();
            pct.fill("", fillSize);
            pct.wpos(pos);

            pct << (uint8)sr.type;
            pct << (uint)sr.damage.hTarget;

            switch (sr.type) {
                case SR_ResultType::SRT_Damage:
                case SR_ResultType::SRT_MagicDamage:
                    pct << sr.damage.target_hp;
                    pct << sr.damage.damage_type;
                    pct << sr.damage.damage;
                    pct << sr.damage.flag;
                    for (uint16 i : sr.damage.elemental_damage)
                        pct << i;
                    break;
                case SR_ResultType::SRT_AddHP:
                case SRT_AddMP:
                    pct << sr.addHPType.target_hp;
                    pct << sr.addHPType.nIncHP;
                default:
                    break;
            }
        }
    }
}

void Skill::broadcastSkillMessage(Unit* pUnit, int cost_hp, int cost_mp, int nType)
{
    if(pUnit == nullptr)
        return;

    auto rx = (uint) (pUnit->GetPositionX() /g_nRegionSize);
    auto ry = (uint)(pUnit->GetPositionY() / g_nRegionSize);
    uint8 layer = pUnit->GetLayer();
    XPacket skillPct(TS_SC_SKILL);
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld->Broadcast((uint)rx, (uint)ry, layer, skillPct);
}

void Skill::broadcastSkillMessage(Unit* pUnit1, Unit* pUnit2, int cost_hp, int cost_mp, int nType)
{
    if(pUnit1 == nullptr || pUnit2 == nullptr)
        return;

    XPacket skillPct(TS_SC_SKILL);
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld->Broadcast((uint)(pUnit1->GetPositionX() /g_nRegionSize), (uint)(pUnit1->GetPositionY() /g_nRegionSize),
                      (uint) (pUnit2->GetPositionX() /g_nRegionSize), (uint)(pUnit2->GetPositionY() /g_nRegionSize),
                      pUnit1->GetLayer(), skillPct);
}

void Skill::ProcSkill()
{
    if(sWorld->GetArTime() < m_nFireTime)
        return;

    if(m_Status == SkillStatus::SS_CAST)
        m_Status = SkillStatus::SS_FIRE;

    /*if(pTarget != nullptr && !pTarget->IsInWorld())
    {
        m_pOwner->CancelSkill();

    }*/

    if(m_Status == SkillStatus::SS_FIRE)
    {
        bool bIsSuccess = false;

        FireSkill(sMemoryPool->GetObjectInWorld<Unit>(m_hTarget), bIsSuccess);
        broadcastSkillMessage(m_pOwner, 0, 0, 0);

        if (!m_bMultiple || m_nCurrentFire == m_nTotalFire)
        {
            m_nFireTime += m_SkillBase->delay_common;
            m_Status = SkillStatus::SS_COMPLETE;
        }

        if (bIsSuccess)
        {
            SetRemainCoolTime(GetSkillCoolTime());
            Player *pOwner = m_pOwner->IsSummon() ? ((Summon *)m_pOwner)->GetMaster() : (Player *)m_pOwner;
            Messages::SendSkillList(pOwner, m_pOwner, m_nSkillID);
        }
    }

    if(m_Status == SkillStatus::SS_COMPLETE && sWorld->GetArTime() >= m_nFireTime)
    {
        broadcastSkillMessage(m_pOwner, 0, 0, 5);
        m_pOwner->OnCompleteSkill();
        Init();
    }
}

void Skill::FireSkill(Unit *pTarget, bool& bIsSuccess)
{
    bool bHandled{true};
    m_vResultList.clear();
    switch(m_SkillBase->effect_type) {
        case SKILL_EFFECT_TYPE::EF_SUMMON:
            DO_SUMMON();
            break;
        case SKILL_EFFECT_TYPE::EF_UNSUMMON:
            DO_UNSUMMON();
            break;
        case SKILL_EFFECT_TYPE::EF_ADD_STATE: {
            FireSkillStateSkillFunctor fn{ };
            fn.onCreature(this, sWorld->GetArTime(), m_pOwner, sMemoryPool->GetObjectInWorld<Unit>(m_hTarget));
        }
            break;
        case SKILL_EFFECT_TYPE::EF_MAGIC_SINGLE_DAMAGE:
        case SKILL_EFFECT_TYPE::EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE:
            SINGLE_MAGICAL_DAMAGE(pTarget);
            break;
        case EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP:
            SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(pTarget);
            break;
        case SKILL_EFFECT_TYPE::EF_MAGIC_MULTIPLE_DAMAGE:
        case SKILL_EFFECT_TYPE::EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP:
            MULTIPLE_MAGICAL_DAMAGE(pTarget);
            break;
        case 30001:// EffectType::PhysicalSingleDamage
            SINGLE_PHYSICAL_DAMAGE(pTarget);
            break;
        case SKILL_EFFECT_TYPE::EF_ACTIVATE_FIELD_PROP:
            ACTIVATE_FIELD_PROP();
            break;
        case EF_TOGGLE_AURA:
        case EF_TOGGLE_DIFFERENTIAL_AURA:
            TOGGLE_AURA(pTarget);
            break;
        case EF_ADD_HP:
        case EF_ADD_HP_BY_ITEM:
            HEALING_SKILL_FUNCTOR(pTarget);
            break;
        case EF_ADD_MP:
        case EF_ADD_MP_BY_ITEM:
            MANA_SKILL_FUNCTOR(pTarget);
            break;
        default:
            bHandled = false;
            break;
    }

    if(!bHandled) {
        switch(m_SkillBase->id) {
            case SkillId::CreatureTaming:
                CREATURE_TAMING();
                bHandled = true;
                break;
            case SkillId::Return:
            case SkillId::TownPortal:
                TOWN_PORTAL();
                bHandled = true;
                break;
            default:
                auto result = string_format("Unknown skill casted - ID %u, effect_type %u", m_SkillBase->id, m_SkillBase->effect_type);
                MX_LOG_INFO("skill", result.c_str());
                if(m_pOwner->IsPlayer())
                    Messages::SendChatMessage(50,"@SYSTEM", dynamic_cast<Player*>(m_pOwner), result);
                else if(m_pOwner->IsSummon())
                {
                    Messages::SendChatMessage(50,"@SYSTEM", dynamic_cast<Summon*>(m_pOwner)->GetMaster(), result);
                }
                break;
        }
    }
    if (m_SkillBase->is_harmful != 0)
    {
        auto mob = dynamic_cast<Monster*>(pTarget);
        if (mob != nullptr && mob->IsMonster() && mob->IsCastRevenger())
        {
            mob->AddHate(m_pOwner->GetHandle(), 1, true, true);
        }
    }
    bIsSuccess = bHandled;
}


void Skill::DO_SUMMON()
{
    auto player = dynamic_cast<Player*>(m_pOwner);
    if(player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if(item == nullptr)
        return;

    if(item->m_pItemBase->group != ItemGroup::SummonCard)
        return;

    auto summon = item->m_pSummon;
    if(summon == nullptr || summon->GetMaster()->GetHandle() != player->GetHandle())
        return;

    if(!summon->IsInWorld())
        player->DoSummon(summon, m_targetPosition);

}

void Skill::DO_UNSUMMON()
{
    auto player = dynamic_cast<Player*>(m_pOwner);
    if(player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if(item == nullptr)
        return;

    if(item->m_pItemBase->group != ItemGroup::SummonCard)
        return;

    auto summon = item->m_pSummon;
    if(summon == nullptr || summon->GetMaster()->GetHandle() != player->GetHandle())
        return;

    if(summon->IsInWorld())
        player->DoUnSummon(summon);
}

bool Skill::Cancel()
{
    Init();
    broadcastSkillMessage(m_pOwner, 0, 0, SkillState::ST_Cancel);
    return true;
}

bool Skill::CheckCoolTime(uint t) const
{
    return m_nNextCoolTime < t;
}

uint Skill::GetSkillCoolTime() const
{
    int l{0};
    if(m_SkillBase == nullptr)
        return 0;

    if (m_pOwner->IsSummon())
        l = m_nRequestedSkillLevel;
    else
        l = m_nEnhance;

    return (uint)(m_pOwner->GetCoolTimeSpeed() * m_pOwner->GetCoolTimeMod((ElementalType)m_SkillBase->elemental, m_SkillBase->is_physical_act != 0, m_SkillBase->is_harmful != 0) * m_SkillBase->GetCoolTime(l));
}

void Skill::SetRemainCoolTime(uint time)
{
    m_nNextCoolTime = time + sWorld->GetArTime();
}

uint Skill::GetSkillEnhance() const
{
    return m_nEnhance;
}

uint16 Skill::PrepareTaming(int nSkillLevel, uint handle, Position pos, uint current_time)
{
    if(!m_pOwner->IsPlayer())
        return TS_RESULT_NOT_ACTABLE;

    auto player = dynamic_cast<Player*>(m_pOwner);
    auto monster = sMemoryPool->GetObjectInWorld<Monster>(handle);
    if(monster == nullptr)
    {
        return TS_RESULT_NOT_ACTABLE;
    }

    auto nTameItemCode = monster->GetTameItemCode();
    if(nTameItemCode == 0 || monster->GetTamer() != 0)
    {
        return TS_RESULT_NOT_ACTABLE;
    }

    auto item = player->FindItem((uint)nTameItemCode, FlagBits::FB_Summon, false);
    if(item == nullptr)
    {
        return TS_RESULT_NOT_ACTABLE;
    }
    if(player->m_hTamingTarget != 0)
    {
        return TS_RESULT_ALREADY_TAMING;
    }
    return TS_RESULT_SUCCESS;
}



/************************* SKILL RESULTS *************************/
// Function       :   protected bool StructSkill::AFFECT_RUSH_OLD(struct StructCreature *, float *, struct ArPosition *, float *)
// Function       :   protected int StructSkill::AFFECT_KNOCK_BACK(struct StructCreature *, float, unsigned long)
// Function       :   protected bool StructSkill::PHYSICAL_DAMAGE_RUSH(struct StructCreature *, int *)
// Function       :   protected void StructSkill::ADD_REGION_STATE(struct StructCreature *)
// Function       :   protected void StructSkill::ADD_STATE_BY_SELF_COST(struct StructCreature *)
// Function       :   protected void StructSkill::ADD_REGION_STATE_BY_SELF_COST(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_DIRECTIONAL_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_PHYSICAL_DAMAGE_T1(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_PHYSICAL_DAMAGE_T2(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_PHYSICAL_DAMAGE_T2_ADD_ENERGY(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_PHYSICAL_DAMAGE_T1(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_PHYSICAL_DAMAGE_T2(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_PHYSICAL_DAMAGE_T3(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_PHYSICAL_DAMAGE_ABSORB(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_REGION_DAMAGE_OLD(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SPECIAL_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_PHYSICAL_DAMAGE_T3(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_PHYSICAL_DAMAGE_T4(struct StructCreature *)
// Function       :   protected bool StructSkill::RUSH(struct StructCreature *, float)
// Function       :   protected bool StructSkill::AFFECT_RUSH(struct StructCreature *, float *, struct ArPosition *, float *, float)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_DAMAGE_ABSORB(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_REALTIME_MULTIPLE_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_MULTIPLE_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_MULTIPLE_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_MAGICAL_DAMAGE_T1(struct StructCreature *)
// Function       :   protected void StructSkill::SINGLE_MAGICAL_DAMAGE_T2(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_MAGICAL_DAMAGE_T1(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_MAGICAL_DAMAGE_T2(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_MAGICAL_DAMAGE_T3(struct StructCreature *)
// Function       :   protected void StructSkill::MULTIPLE_MAGICAL_DAMAGE_AT_ONCE(struct StructCreature *)
// Function       :   protected void StructSkill::MAGIC_SINGLE_REGION_DAMAGE_OLD(struct StructCreature *)
// Function       :   protected void StructSkill::MAGIC_MULTIPLE_REGION_DAMAGE_OLD(struct StructCreature *)
// Function       :   protected void StructSkill::MAGIC_MULTIPLE_REGION_DAMAGE_T2(struct StructCreature *)
// Function       :   protected void StructSkill::MAGIC_SPECIAL_REGION_DAMAGE_OLD(struct StructCreature *)
// Function       :   protected void StructSkill::MAGIC_ABSORB_DAMAGE(struct StructCreature *)
void Skill::CREATURE_TAMING()
{
    auto pTarget = sMemoryPool->GetObjectInWorld<Monster>(m_hTarget);
    if(pTarget == nullptr || !pTarget->IsMonster() || !m_pOwner->IsPlayer())
        return;

    bool bResult = sWorld->SetTamer(pTarget, dynamic_cast<Player*>(m_pOwner), m_nRequestedSkillLevel);
    if(bResult) {
        pTarget->AddHate(m_pOwner->GetHandle(), 1, true, true);
    }
}

void Skill::SINGLE_PHYSICAL_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    if (m_SkillBase->effect_type == SKILL_EFFECT_TYPE::EF_PHYSICAL_SINGLE_DAMAGE_RUSH || m_SkillBase->effect_type == SKILL_EFFECT_TYPE::EF_PHYSICAL_SINGLE_DAMAGE_RUSH_KNOCKBACK)
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


    sWorld->AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
}

void Skill::SINGLE_MAGICAL_DAMAGE(Unit *pTarget)
{
    if(pTarget == nullptr || m_pOwner == nullptr)
        return;

    bool v10 = m_SkillBase->is_physical_act != 0;
    //auto attack_point = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto magic_point = m_pOwner->m_Attribute.nMagicPoint;
    auto nDamage = (int)(magic_point
                         * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                         + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel) + (m_SkillBase->var[5] * m_nEnhance));

    auto damage = pTarget->DealMagicalSkillDamage(m_pOwner, nDamage, (ElementalType)m_SkillBase->elemental,
                                                   m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                   m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl), 0);

    sWorld->AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, damage, pTarget->GetHandle());
}


void Skill::SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(Unit *pTarget)
{
    if(pTarget == nullptr)
        return;

    auto elemental_type = m_SkillBase->elemental;
    //auto nMagicPoint = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->elemental, this->m_SkillBase->is_physical_act == 0, m_SkillBase->is_harmful != 0);
    auto nMagicPoint = m_pOwner->m_Attribute.nMagicPoint;
    auto nDamage = (int)(nMagicPoint * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                    + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel)
                    + (m_SkillBase->var[5] * m_nEnhance));

    auto damage = pTarget->DealMagicalSkillDamage(m_pOwner,nDamage, (ElementalType)m_SkillBase->elemental,
            m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
            (m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl)),0);

    sWorld->AddSkillDamageResult(m_vResultList, 1, (uint8)elemental_type, damage, pTarget->GetHandle());

    auto nAddHP = (int)(((m_SkillBase->var[8] * m_nEnhance) + (m_SkillBase->var[6] + (m_SkillBase->var[7] * m_nRequestedSkillLevel))) * damage.nDamage);
    auto nAddMP = (int)(((m_SkillBase->var[11] * m_nEnhance) + (m_SkillBase->var[9] + (m_SkillBase->var[10] * m_nRequestedSkillLevel))) * damage.nDamage);
    m_pOwner->AddHealth(nAddHP);
    m_pOwner->AddMana(nAddMP);

    SkillResult skill_result{};
    skill_result.type = (int)SRT_AddHP;
    skill_result.hTarget = m_pOwner->GetHandle();
    skill_result.addHPType.type = (int)SRT_AddHP;
    skill_result.addHPType.hTarget = m_pOwner->GetHandle();
    skill_result.addHPType.target_hp = m_pOwner->GetHealth();
    skill_result.addHPType.nIncHP = nAddHP;

    m_vResultList.emplace_back(skill_result);

    SkillResult skillResult{};
    skillResult.type = (int)SRT_AddMP;
    skillResult.hTarget = m_pOwner->GetHandle();
    skillResult.addHPType.type = (int)SRT_AddMP;
    skillResult.addHPType.hTarget = m_pOwner->GetHandle();
    skillResult.addHPType.target_hp = (int)m_pOwner->GetMana();
    skillResult.addHPType.nIncHP = nAddHP;
    m_vResultList.emplace_back(skillResult);
}


void Skill::ACTIVATE_FIELD_PROP()
{
    auto fp = sMemoryPool->GetObjectInWorld<FieldProp>(m_hTarget);
    if(fp != nullptr)
    {
        sWorld->AddSkillDamageResult(m_vResultList, fp->UseProp(dynamic_cast<Player*>(m_pOwner)), 0, 0);
    }
}

void Skill::HEALING_SKILL_FUNCTOR(Unit *pTarget)
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

    heal = pTarget->Heal((int)heal);

    SkillResult skillResult{};
    skillResult.type = SR_ResultType::SRT_AddHP;
    skillResult.damage.hTarget = pTarget->GetHandle();
    skillResult.addHPType.target_hp = pTarget->GetHealth();
    skillResult.addHPType.nIncHP = (int)heal;
    m_vResultList.emplace_back(skillResult);
    //sWorld->AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, heal, pTarget->GetHandle());
}

void Skill::TOWN_PORTAL()
{
    if(m_pOwner->IsPlayer())
    {
        auto pPlayer = dynamic_cast<Player*>(m_pOwner);

        auto pos = pPlayer->GetLastTownPosition();
        pPlayer->PendWarp((int)pos.GetPositionX(), (int)pos.GetPositionY(), 0);
        pos = pPlayer->GetCurrentPosition(sWorld->GetArTime());
        pPlayer->SetMove(pos, 0, 0);
    }
}

void Skill::MULTIPLE_MAGICAL_DAMAGE(Unit *pTarget)
{
    if (pTarget == nullptr || m_pOwner == nullptr)
        return;

    bool v10         = m_SkillBase->is_physical_act != 0;
    //auto attack_point = m_pOwner->GetMagicPoint((ElementalType)m_SkillBase->effect_type, v10, m_SkillBase->is_harmful != 0);
    auto magic_point = m_pOwner->m_Attribute.nMagicPoint;

    auto dmg = (int)(magic_point
                     * (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel)) + (m_SkillBase->var[2] * m_nEnhance)
                     + m_SkillBase->var[3] + (m_SkillBase->var[4] * m_nRequestedSkillLevel) + (m_SkillBase->var[5] * m_nEnhance));

    if (m_nCurrentFire != 0)
    {
        m_nCurrentFire++;
    }
    else
    {
        m_nTotalFire = (int)(m_SkillBase->var[6] + (m_SkillBase->var[7] * m_nRequestedSkillLevel));
        m_nCurrentFire = 1;
    }

    auto Damage = pTarget->DealMagicalSkillDamage(m_pOwner, dmg, (ElementalType)m_SkillBase->elemental,
                                                  m_SkillBase->GetHitBonus(m_nEnhance, m_pOwner->GetLevel() - pTarget->GetLevel()),
                                                  (m_SkillBase->critical_bonus + (m_nRequestedSkillLevel * m_SkillBase->critical_bonus_per_skl)), 0);
    sWorld->AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, Damage, pTarget->GetHandle());
    m_nFireTime = (uint)((m_SkillBase->var[8] * 100) + m_nFireTime);
}

void Skill::TOGGLE_AURA(Unit *pTarget)
{
    m_pOwner->ToggleAura(this);
    if(m_SkillBase->target == 21)
    {
        // TODO: Party functor
    }
    sWorld->AddSkillDamageResult(m_vResultList, true, 0, m_pOwner->GetHandle());
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

    SkillResult skillResult{};
    skillResult.type = SR_ResultType::SRT_AddMP;
    skillResult.damage.hTarget = pTarget->GetHandle();
    skillResult.addHPType.target_hp = pTarget->GetHealth();
    skillResult.addHPType.nIncHP = (int)heal;
    m_vResultList.emplace_back(skillResult);
}
