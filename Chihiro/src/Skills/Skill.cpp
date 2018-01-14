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
#include "ArRegion.h"

Skill::Skill(Unit *pOwner, int64 _uid, int _id) : m_nErrorCode(0)
{
    m_nSkillUID = _uid;
    m_nSkillID = _id;
    m_pOwner = pOwner;
    cool_time = 0;
    m_nSummonID = 0;
    m_nSkillLevel = 0;
    m_SkillBase = sObjectMgr->GetSkillBase(m_nSkillID);
    Init();
}

void Skill::Init()
{
    m_nErrorCode = 0;
    /*m_Status = 0;*/
    m_nCastTime = 0;
    m_nCastingDelay = 0;
    m_nFireTime = 0;
    m_nRequestedSkillLevel = 0;
    m_hTarget = 0;
    /*m_nCurrentFire = 0;
    m_nTotalFire = 0;*/
    m_nTargetCount = 1;
    m_nFireCount = 1;
    m_targetPosition.Relocate(0, 0, 0, 0);
    m_targetPosition.SetLayer(0);
}


void Skill::DB_InsertSkill(Unit *pUnit, int64 skillUID, uint owner_uid, uint summon_uid, uint skill_id, uint skill_level)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SKILL);
    stmt->setInt64(0, skillUID);
    stmt->setInt32(1, owner_uid);
    stmt->setInt32(2, summon_uid);
    stmt->setInt32(3, skill_id);
    stmt->setInt32(4, skill_level);
    stmt->setInt32(5, 0); // cool_time
    CharacterDatabase.Execute(stmt);
}

void Skill::DB_UpdateSkill(Unit *pUnit, int64 skill_uid, uint skill_level)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_SKILL);
    stmt->setInt32(0, skill_level);
    stmt->setInt32(1, 0); // cool_time
    auto uid = pUnit->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(2, uid);
    stmt->setInt64(3, skill_uid);
    CharacterDatabase.Execute(stmt);
}

int Skill::Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    m_vResultList.clear();
    auto current_time = sWorld->GetArTime();
    uint delay        = 0xffffffff;

    if (!CheckCoolTime(current_time))
    {
        return TS_RESULT_COOL_TIME;
    }

    switch(m_SkillBase->effect_type)
    {
        case EffectType::ET_Summon:
            m_nErrorCode = PrepareSummon(nSkillLevel, handle, pos, current_time);
            break;
        case EffectType::ActivateFieldProp:
        case EffectType::RegionHealByFieldProp:
        case EffectType::AreaAffectHealByHieldProp:
        {
            m_nErrorCode = TS_RESULT_NOT_ACTABLE;
            if(m_pOwner->GetSubType() == ST_Player)
            {
                auto pProp = sMemoryPool->GetObjectInWorld<FieldProp>(handle);
                if(pProp != nullptr && pProp->m_pFieldPropBase->nActivateSkillID == m_SkillBase->id )
                {
                    if(pProp->m_nUseCount >= 1 && pProp->IsUsable(dynamic_cast<Player*>(m_pOwner)))
                    {
                        delay = pProp->GetCastingDelay();
                        if (sArRegion->IsVisibleRegion(m_pOwner, pProp) == 0)
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
        m_nRequestedSkillLevel = (uint8)nSkillLevel;
        m_pOwner->m_castingSkill = this;
        broadcastSkillMessage((int)(m_pOwner->GetPositionX() / g_nRegionSize),
                              (int)(m_pOwner->GetPositionY() / g_nRegionSize), m_pOwner->GetLayer(),
                              0, 0, 1);
    }
    else
    {
        Init();
        broadcastSkillMessage((int)(m_pOwner->GetPositionX() / g_nRegionSize),
                              (int)(m_pOwner->GetPositionY() / g_nRegionSize), m_pOwner->GetLayer(),
                              0, 0, 5);
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

    pct << (uint8)0;
    pct << (float)0;
    pct << (uint8)1;
    pct << (uint8)1;
    pct << (uint16)m_vResultList.size();

    if (!m_vResultList.empty()) {
        for (auto &sr : m_vResultList) {
            // Odd fixed padding for the skill result
            auto wpos = pct.wpos();
            pct.fill("", m_vResultList.size() * 45);
            pct.wpos(wpos);

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
                    pct << sr.damage.target_hp;
                    pct << sr.addHPType.nIncHP;
                default:
                    break;
            }
        }
    }
}

void Skill::broadcastSkillMessage(int rx, int ry, uint8 layer, int cost_hp, int cost_mp, int nType)
{
    XPacket skillPct(TS_SC_SKILL);
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld->Broadcast((uint)rx, (uint)ry, layer, skillPct);
}

void Skill::broadcastSkillMessage(int rx1, int ry1, int rx2, int ry2, uint8 layer, int cost_hp, int cost_mp, int nType)
{
    XPacket skillPct(TS_SC_SKILL);
    assembleMessage(skillPct, nType, cost_hp, cost_mp);
    sWorld->Broadcast((uint)rx1, (uint)ry1,(uint) rx2, (uint)ry2, layer, skillPct);
}

void Skill::ProcSkill()
{
    if(sWorld->GetArTime() < m_nFireTime)
        return;
    m_pOwner->m_castingSkill = nullptr;

    bool bIsSuccess = false;
    FireSkill(sMemoryPool->GetObjectInWorld<Unit>(m_hTarget), bIsSuccess);
    if(bIsSuccess)
    {
        SetRemainCoolTime(GetSkillCoolTime());
        Player* pOwner = m_pOwner->GetSubType() == ST_Summon ? ((Summon*)m_pOwner)->GetMaster() : (Player*)m_pOwner;
        Messages::SendSkillList(pOwner, m_pOwner, m_nSkillID);
    }

    broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                          (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                          0, 0, 0);
    broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                          (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                          0, 0, 5);

    Init();
}

void Skill::FireSkill(Unit *pTarget, bool& bIsSuccess)
{
    bool bHandled{true};
    switch(m_SkillBase->effect_type) {
        case EffectType::ET_Summon:
            DO_SUMMON();
            break;
        case EffectType::Unsummon:
            DO_UNSUMMON();
            break;
        case EffectType::AddState: {
            FireSkillStateSkillFunctor fn{ };
            fn.onCreature(this, sWorld->GetArTime(), m_pOwner, sMemoryPool->GetObjectInWorld<Unit>(m_hTarget));
        }
            break;
        case EffectType::MagicSingleDamage:
        case EffectType::MagicSingleDamageAddRandomState:
            SINGLE_MAGICAL_DAMAGE(pTarget);
            break;
        case 30001:// EffectType::PhysicalSingleDamage
            SINGLE_PHYSICAL_DAMAGE(pTarget);
            break;
        case EffectType::ActivateFieldProp:
            ACTIVATE_FIELD_PROP();
            break;
        case 501:
            HEALING_SKILL_FUNCTOR(pTarget);
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
                if(m_pOwner->GetSubType() == ST_Player)
                    Messages::SendChatMessage(50,"@SYSTEM", dynamic_cast<Player*>(m_pOwner), result);
                break;
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
    broadcastSkillMessage((int)(m_pOwner->GetPositionX() / g_nRegionSize),
                          (int)(m_pOwner->GetPositionY() / g_nRegionSize), m_pOwner->GetLayer(),
                          0, 0, SkillState::ST_Cancel);
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

    if (m_pOwner->GetSubType() == ST_Summon)
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
    if(m_pOwner->GetSubType() != ST_Player)
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

void Skill::CREATURE_TAMING()
{
    auto pTarget = sMemoryPool->GetObjectInWorld<Monster>(m_hTarget);
    if(pTarget == nullptr || pTarget->GetSubType() != ST_Mob || m_pOwner->GetSubType() != ST_Player)
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

    if (m_SkillBase->effect_type == PhysicalSingleDamageRush || m_SkillBase->effect_type == PhysicalSingleDamageRushKnockback)
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
    skillResult.damage.target_hp = pTarget->GetHealth();
    skillResult.addHPType.nIncHP = (int)heal;
    m_vResultList.emplace_back(skillResult);
    //sWorld->AddSkillDamageResult(m_vResultList, 1, m_SkillBase->elemental, heal, pTarget->GetHandle());
}

void Skill::TOWN_PORTAL()
{
    if(m_pOwner->GetSubType() == ST_Player)
    {
        auto pPlayer = dynamic_cast<Player*>(m_pOwner);

        auto pos = pPlayer->GetLastTownPosition();
        pPlayer->PendWarp((int)pos.GetPositionX(), (int)pos.GetPositionY(), 0);
        pos = pPlayer->GetCurrentPosition(sWorld->GetArTime());
        pPlayer->SetMove(pos, 0, 0);
    }
}
