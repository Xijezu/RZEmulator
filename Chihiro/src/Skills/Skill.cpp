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
        case EF_SUMMON:
            m_nErrorCode = PrepareSummon(nSkillLevel, handle, pos, current_time);
            break;
        case EF_ACTIVATE_FIELD_PROP:
        case EF_REGION_HEAL_BY_FIELD_PROP:
        case EF_AREA_EFFECT_HEAL_BY_FIELD_PROP:
        {
            m_nErrorCode = TS_RESULT_NOT_ACTABLE;
            if(m_pOwner->IsPlayer())
            {
                auto pProp = sMemoryPool->GetObjectInWorld<FieldProp>(handle);
                if(pProp != nullptr && pProp->m_pFieldPropBase->nActivateSkillID == m_SkillBase->id )
                {
                    if(pProp->m_nUseCount >= 1 && pProp->IsUsable(m_pOwner->As<Player>()))
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
    if (m_SkillBase->id == SKILL_CREATURE_TAMING)
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
    auto item = sMemoryPool->GetObjectInWorld<Item>(handle);
    if (item == nullptr || item->m_pItemBase == nullptr || item->m_pItemBase->group != GROUP_SUMMONCARD || item->m_Instance.OwnerHandle != m_pOwner->GetHandle())
    {
        return TS_RESULT_NOT_ACTABLE;
    }
    auto player = m_pOwner->As<Player>();
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

    if (nType != SKILL_STATUS::ST_FIRE) {
        if (nType <= SKILL_STATUS::ST_FIRE)
            return;
        if (nType <= SKILL_STATUS::ST_CASTING_UPDATE || nType == SKILL_STATUS::ST_COMPLETE) {
            pct << (uint32)(m_nFireTime - m_nCastTime);
            pct << (uint16)m_nErrorCode;
            pct.fill("", 3);
            return;
        }
        if (nType == SKILL_STATUS::ST_CANCEL) {
            pct << (uint8)0;
            return;
        }
        if (nType != SKILL_STATUS::ST_REGION_FIRE)
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
                case SRT_DAMAGE:
                case SRT_MAGIC_DAMAGE:
                    pct << sr.damage.target_hp;
                    pct << sr.damage.damage_type;
                    pct << sr.damage.damage;
                    pct << sr.damage.flag;
                    for (uint16 i : sr.damage.elemental_damage)
                        pct << i;
                    break;
                case SRT_ADD_HP:
                case SRT_ADD_MP:
                    pct << sr.addHPType.target_hp;
                    pct << sr.addHPType.nIncHP;
                case SRT_REBIRTH:
                    pct << sr.rebirth.target_hp;
                    pct << sr.rebirth.nIncHP;
                    pct << sr.rebirth.nIncMP;
                    pct << sr.rebirth.nRecoveryEXP;
                    pct << sr.rebirth.target_mp;
                    break;
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
        case EF_SUMMON:
            DO_SUMMON();
            break;
        case EF_UNSUMMON:
            DO_UNSUMMON();
            break;
        case EF_ADD_STATE:
        {
            FireSkillStateSkillFunctor fn{ };
            fn.pvList = m_vResultList;
            process_target(sWorld->GetArTime(), fn, pTarget);
        }
            break;
        case EF_MAGIC_SINGLE_DAMAGE:
        case EF_MAGIC_SINGLE_DAMAGE_ADD_RANDOM_STATE:
            SINGLE_MAGICAL_DAMAGE(pTarget);
            break;
        case EF_MAGIC_DAMAGE_WITH_ABSORB_HP_MP:
            SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(pTarget);
            break;
        case EF_MAGIC_MULTIPLE_DAMAGE:
        case EF_MAGIC_MULTIPLE_DAMAGE_DEAL_SUMMON_HP:
            MULTIPLE_MAGICAL_DAMAGE(pTarget);
            break;
        case 30001:// EffectType::PhysicalSingleDamage
            SINGLE_PHYSICAL_DAMAGE(pTarget);
            break;
        case EF_ACTIVATE_FIELD_PROP:
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
        case EF_RESURRECTION:
            SKILL_RESURRECTION(pTarget);
            break;
        default:
            bHandled = false;
            break;
    }
    if(!bHandled) {
        switch(m_SkillBase->id) {
            case SKILL_CREATURE_TAMING:
                CREATURE_TAMING();
                bHandled = true;
                break;
            case SKILL_RETURN:
            case SKILL_TOWN_PORTAL:
                TOWN_PORTAL();
                bHandled = true;
                break;
            default:
                auto result = string_format("Unknown skill casted - ID %u, effect_type %u", m_SkillBase->id, m_SkillBase->effect_type);
                MX_LOG_INFO("skill", result.c_str());
                if(m_pOwner->IsPlayer())
                    Messages::SendChatMessage(50,"@SYSTEM", m_pOwner->As<Player>(), result);
                else if(m_pOwner->IsSummon())
                {
                    Messages::SendChatMessage(50,"@SYSTEM",m_pOwner->As<Summon>()->GetMaster(), result);
                }
                break;
        }
    }

    if(bHandled)
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
    std::vector<Unit*> vNeedStateList{};
    for(const auto& sr : m_vResultList)
    {
        auto pDealTarget = sMemoryPool->GetObjectInWorld<Unit>(sr.hTarget);
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

    if(!vNeedStateList.empty())
    {
        FireSkillStateSkillFunctor fn{};
        fn.pvList = m_vResultList;
        auto t = sWorld->GetArTime();

        for(auto& unit : vNeedStateList)
        {
            process_target(t, fn, unit);
        }
    }
}

void Skill::DO_SUMMON()
{
    auto player = m_pOwner->As<Player>();
    if(player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if(item == nullptr)
        return;

    if(item->m_pItemBase->group != GROUP_SUMMONCARD)
        return;

    auto summon = item->m_pSummon;
    if(summon == nullptr || summon->GetMaster()->GetHandle() != player->GetHandle())
        return;

    if(!summon->IsInWorld())
        player->DoSummon(summon, m_targetPosition);

}

void Skill::DO_UNSUMMON()
{
    auto player = m_pOwner->As<Player>();
    if(player == nullptr)
        return;

    auto item = player->FindItemByHandle(m_hTarget);
    if(item == nullptr)
        return;

    if(item->m_pItemBase->group != GROUP_SUMMONCARD)
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

    auto player = m_pOwner->As<Player>();
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

    auto item = player->FindItem((uint)nTameItemCode, ITEM_FLAG_SUMMON, false);
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

    bool bResult = sWorld->SetTamer(pTarget, m_pOwner->As<Player>(), m_nRequestedSkillLevel);
    if(bResult) {
        pTarget->AddHate(m_pOwner->GetHandle(), 1, true, true);
    }
}

void Skill::SINGLE_PHYSICAL_DAMAGE(Unit *pTarget)
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
    skill_result.type = (int)SRT_ADD_HP;
    skill_result.hTarget = m_pOwner->GetHandle();
    skill_result.addHPType.type = (int)SRT_ADD_HP;
    skill_result.addHPType.hTarget = m_pOwner->GetHandle();
    skill_result.addHPType.target_hp = m_pOwner->GetHealth();
    skill_result.addHPType.nIncHP = nAddHP;

    m_vResultList.emplace_back(skill_result);

    SkillResult skillResult{};
    skillResult.type = (int)SRT_ADD_MP;
    skillResult.hTarget = m_pOwner->GetHandle();
    skillResult.addHPType.type = (int)SRT_ADD_MP;
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
        sWorld->AddSkillDamageResult(m_vResultList, fp->UseProp(m_pOwner->As<Player>()), 0, 0);
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
    skillResult.type = SRT_ADD_HP;
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
        auto pPlayer = m_pOwner->As<Player>();

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
//    /run scv(get_creature_handle(0), "hp", 0)
    auto magic_point   = m_pOwner->m_Attribute.nMagicPoint;
    auto target_max_hp = pTarget->GetMaxHealth();

    auto heal = magic_point *
                (m_SkillBase->var[0] + (m_SkillBase->var[1] * m_nRequestedSkillLevel))
                + m_SkillBase->var[2] + (m_SkillBase->var[3] * m_nRequestedSkillLevel) + (m_nEnhance * m_SkillBase->var[6])
                + target_max_hp * (m_SkillBase->var[4] + (m_SkillBase->var[5] * m_nRequestedSkillLevel) + m_SkillBase->var[7] * m_nRequestedSkillLevel);

    heal = pTarget->MPHeal((int)heal);

    SkillResult skillResult{};
    skillResult.type = SRT_ADD_MP;
    skillResult.damage.hTarget = pTarget->GetHandle();
    skillResult.addHPType.target_hp = pTarget->GetHealth();
    skillResult.addHPType.nIncHP = (int)heal;
    m_vResultList.emplace_back(skillResult);
}

void Skill::SKILL_RESURRECTION(Unit *pTarget)
{
    if(pTarget == nullptr || pTarget->GetHealth() != 0)
        return;

    auto prev_hp = pTarget->GetHealth();
    auto prev_mp = pTarget->GetMana();

    pTarget->AddHealth((int)CalculatePct(pTarget->GetMaxHealth(), (m_SkillBase->var[0] * 100)));
    pTarget->AddMana((int)CalculatePct(pTarget->GetMaxMana(), (m_SkillBase->var[1] * 100)));

    SkillResult skillResult{};
    skillResult.type = SRT_REBIRTH;
    skillResult.damage.hTarget = pTarget->GetHandle();
    skillResult.rebirth.target_hp = pTarget->GetHealth();
    skillResult.rebirth.nIncHP = std::max((int)(pTarget->GetHealth() - prev_hp), 0);
    skillResult.rebirth.nIncMP = std::max((int)(pTarget->GetMana() - prev_mp), 0);
    skillResult.rebirth.nRecoveryEXP = 0;
    skillResult.rebirth.target_mp = (int16)pTarget->GetMana();
    m_vResultList.emplace_back(skillResult);
}

void Skill::process_target(uint t, SkillTargetFunctor& fn, Unit *pTarget)
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

    switch(m_SkillBase->target)
    {
        case 1:
        case 6:
            if (m_SkillBase->target == 6 && pTarget == m_pOwner)
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

        case 2:
        case 3:
        case 4:
            if (pTarget == nullptr)
                return;
            fn.onCreature(this, t, m_pOwner, pTarget);
            return;
        default:
            return;
    }

/*
            if (this.m_SkillBase.target > 31 )
            {
                v35 = this.m_SkillBase.target - 32;
                if (this.m_SkillBase.target == 32)
                {
                    if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)() )
                        return;
                    ::::myPartyFunctor::myPartyFunctor(&v63, v4, t, v4->m_pOwner, functor);
                    v27 = *(v52 + 4244);
                    if ( !v27 )
                    {
                        (**v52)(v52);
                        v20 = v63;
                        v21 = &v63;
                        goto LABEL_109;
                    }
                    v28 = &v63;
                    goto LABEL_111;
                }
                v36 = v35 - 13;
                if ( v36 )
                {
                    v37 = v36 - 6;
                    if ( !v37 )
                    {
                        if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)()
                          && !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle)()
                          || !(pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget)
                          && !pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle(pTarget) )
                            return;
                        if ( (v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)() )
                        {
                            v41 = v4->m_pOwner;
                        }
                        else
                        {
                            if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle)() )
                                return;
                            v41 = v4->m_pOwner[1].baseclass_0.baseclass_0.prev_ry;
                        }
                        if ( !v41 )
                            return;
                        v42 = (pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget);
                        v43 = pTarget;
                        if ( !v42 )
                        {
                            if ( !pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle(pTarget) )
                                return;
                            v43 = pTarget[1].baseclass_0.baseclass_0.prev_ry;
                        }
                        if ( !v43
                          || v41 != v43
                          && (v44 = *(&v41[1].baseclass_0.m_nRefCount + 1)) != 0
                          && v44 != *(&v43[1].baseclass_0.m_nRefCount + 1) )
                            return;
                        ::::myPartyFunctor::myPartyFunctor(&v62, v4, t, v4->m_pOwner, functor);
                        v27 = *(&v41[1].baseclass_0.m_nRefCount + 1);
                        if ( !v27 )
                        {
                            v41->baseclass_0.baseclass_0.baseclass_0.vfptr->GetHandle(v41);
                            v20 = v62;
                            v21 = &v62;
                            goto LABEL_109;
                        }
                        v28 = &v62;
                        goto LABEL_111;
                    }
                    v38 = v37 - 50;
                    if ( !v38 )
                    {
                        if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle)()
                          || (v40 = v4->m_pOwner[1].baseclass_0.baseclass_0.prev_ry) == 0
                          || !*(v40 + 68) )
                            return;
                        v59 = v4->m_pOwner[1].baseclass_0.baseclass_0.prev_ry;
                        v56 = v4->m_pOwner;
        LABEL_47:
                        (functor->vfptr->onCreature)(v4, t, v56, v59);
                        return;
                    }
                    if ( v38 != 1
                      || !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle)()
                      || !v4->m_pOwner->baseclass_0.baseclass_0.bIsInWorld
                      || (functor->vfptr->onCreature(functor, v4, t, v4->m_pOwner, v4->m_pOwner),
                          (v39 = v4->m_pOwner[1].baseclass_0.baseclass_0.prev_ry) == 0)
                      || !*(v39 + 68)
                      || ArPosition::GetDistance((v39 + 108), &v4->m_pOwner->baseclass_0.baseclass_0.mv.baseclass_0) > 525.0 )
                        return;
                    v29 = functor->vfptr;
                    v58 = v39;
                    v55 = v4->m_pOwner;
        LABEL_40:
                    (v29->onCreature)(v4, t, v55, v58);
                    return;
                }
                v45 = pTarget;
                if ( !(pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget) )
                {
                    if ( !pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle(pTarget)
                      || (v45 = pTarget[1].baseclass_0.baseclass_0.prev_ry) == 0 )
                        return;
                }
                pTargetPlayer = v45;
                if ( !v45 )
                    return;
                if ( !v45->baseclass_0.baseclass_0.bIsInWorld )
                    return;
                (functor->vfptr->onCreature)(v4, t, v4->m_pOwner, v45);
                v46 = &v4->m_pOwner->baseclass_0.baseclass_0.mv;
                LODWORD(v61.x) = *v46;
                v46 += 4;
                LODWORD(v61.y) = *v46;
                v46 += 4;
                LODWORD(v61.z) = *v46;
                LODWORD(v61.face) = *(v46 + 4);
                v47 = pTargetPlayer[1].m_csEnemy.m_cs.DebugInfo;
                if ( v47 )
                {
                    if ( LOBYTE(v47[2].CriticalSection) )
                    {
                        if ( v47[6].ContentionCount )
                        {
                            LODWORD(v69) = 12 * v4->m_pSkillBase->valid_range;
                            v69 = SLODWORD(v69);
                            v68 = v69;
                            v48 = ArPosition::GetDistance(&v61, &v47[3].ProcessLocksList.Blink);
                            if ( v48 <= v68 )
                                (functor->vfptr->onCreature)(v4, t, v4->m_pOwner, v47);
                        }
                    }
                }
                v49 = pTargetPlayer;
                v50 = pTargetPlayer[1].m_csEnemy.m_cs.LockCount;
                if ( !v50
                  || !*(v50 + 68)
                  || !*(v50 + 212)
                  || (pTargetPlayera = (12 * v4->m_pSkillBase->valid_range),
                      v68 = pTargetPlayera,
                      v51 = ArPosition::GetDistance(&v61, (v50 + 108)),
                      v51 > v68) )
                    return;
                v58 = v49[1].m_csEnemy.m_cs.LockCount;
                v55 = v4->m_pOwner;
        LABEL_39:
                v29 = functor->vfptr;
                goto LABEL_40;
                     (v29->onCreature)(v4, t, v55, v58);
                    return;

            }
            if (this.m_SkillBase.target == 31 )
            {
                v30 = pTarget;
                if ( !pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[2].GetHandle(pTarget)
                  || !pTarget->baseclass_0.baseclass_0.bIsInWorld )
                {
                    if ( !(pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget)
                      || (v31 = pTarget[1].m_csEnemy.m_cs.DebugInfo) == 0
                      || !LOBYTE(v31[2].CriticalSection) )
                        return;
                    v30 = pTarget[1].m_csEnemy.m_cs.DebugInfo;
                }
                pSummon = v30;
                if ( !v30
                  || (v32 = 12 * v4->m_pSkillBase->valid_range,
                      v33 = (v30 + 108),
                      LODWORD(pos.x) = *v33,
                      v33 += 4,
                      LODWORD(v69) = v32,
                      LODWORD(pos.y) = *v33,
                      v33 += 4,
                      v69 = v32,
                      LODWORD(pos.z) = *v33,
                      v68 = v69,
                      LODWORD(pos.face) = *(v33 + 4),
                      v34 = ArPosition::GetDistance(&pos, &pTarget->baseclass_0.baseclass_0.mv.baseclass_0),
                      v34 > v68) )
                    return;
                v58 = pSummon;
                goto LABEL_38;
            }
            if (this.m_SkillBase.target == 1 )
                goto LABEL_41;
            if (this.m_SkillBase.target <= 1 )
                return;
            if (this.m_SkillBase.target <= 4 )
            {
                if ( !pTarget )
                    return;
                v58 = pTarget;
        LABEL_38:
                v55 = v4->m_pOwner;
                goto LABEL_39;
         LABEL_39:
                v29 = functor->vfptr;
                goto LABEL_40;

            }
            if (this.m_SkillBase.target == 6 )
            {
        LABEL_41:
                if ( v5->target == 6 && pTarget == v4->m_pOwner )
                    return;
                if ( v5->is_need_target )
                {
                    if ( !pTarget )
                        return;
                    v59 = pTarget;
                    v56 = v4->m_pOwner;
                }
                else
                {
                    v59 = v4->m_pOwner;
                    v56 = v4->m_pOwner;
                }
                goto LABEL_47;
            }
            if (this.m_SkillBase.target == 21 )
            {
                if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)()
                  || !(pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget)
                  || (v24 = v4->m_pOwner, v24 != pTarget)
                  && (v25 = *(&v24[1].baseclass_0.m_nRefCount + 1)) != 0
                  && v25 != *(&pTarget[1].baseclass_0.m_nRefCount + 1) )
                    return;
                ::::myPartyFunctor::myPartyFunctor(&fo, v4, t, v24, functor);
                v27 = *(v26 + 4244);
                if ( !v27 )
                {
                    (**v26)(v26);
                    v20 = fo.baseclass_0.vfptr;
                    v21 = &fo;
                    goto LABEL_109;
                }
                v28 = &fo;
        LABEL_111:
                v57 = v28;
                v54 = v27;
                goto LABEL_112;
            }
            if (this.m_SkillBase.target == 22 )
            {
                if ( !(v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)()
                  || !(pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget)
                  || (v17 = v4->m_pOwner, v17 != pTarget)
                  && (v18 = v17[1].quadTreeItem.y) != 0
                  && v18 != pTarget[1].quadTreeItem.y )
                    return;
                ::::myGuildFunctor::myGuildFunctor(&_fo, v4, t, v17, functor);
                if ( *(v19 + 4256) )
                {
                    v22 = *(v19 + 4256);
                    v23 = GuildManager::GetInstance();
                    v15 = GuildManager::DoEachMember(v23, v22, &_fo);
                    goto LABEL_113;
                }
                (**v19)(v19);
                v20 = _fo.vfptr;
                v21 = &_fo;
        LABEL_109:
                (*v20)(v21);
                return;
            }
            if (this.m_SkillBase.target == 23 )
            {
                if ( (v4->m_pOwner->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)() )
                {
                    if ( (pTarget->baseclass_0.baseclass_0.baseclass_0.vfptr[1].onProcess)(pTarget) )
                    {
                        v7 = v4->m_pOwner;
                        if ( v7 == pTarget
                          || (v8 = *(&v7[1].baseclass_0.m_nRefCount + 1)) == 0
                          || v8 == *(&pTarget[1].baseclass_0.m_nRefCount + 1) )
                        {
                            ::::myPartyFunctor::myPartyFunctor(&thisa, v4, t, v7, functor);
                            v9 = *(&v7[1].baseclass_0.m_nRefCount + 1);
                            v10 = *(&v7[1].baseclass_0.m_nRefCount + 1);
                            v11 = PartyManager::GetInstance();
                            v12 = PartyManager::GetAttackTeamLeadPartyID(v11, v10);
                            if ( v12 )
                            {
                                v13 = v12;
                                v14 = PartyManager::GetInstance();
                                v15 = PartyManager::DoEachAttackTeamMember(v14, v13, &thisa.baseclass_0);
        LABEL_113:
                                v4->m_nTargetCount = v15;
                                return;
                            }
                            if ( !v9 )
                            {
                                v16 = v7->baseclass_0.baseclass_0.baseclass_0.vfptr->GetHandle(v7);
                                thisa.baseclass_0.vfptr->operator()(&thisa, v16);
                                v4->m_nTargetCount = 1;
                                return;
                            }
                            v57 = &thisa;
                            v54 = v9;
        LABEL_112:
                            v53 = PartyManager::GetInstance();
                            v15 = PartyManager::DoEachMember(v53, v54, v57);
                            goto LABEL_113;
                        }
                    }
                }
            }

*/
}


