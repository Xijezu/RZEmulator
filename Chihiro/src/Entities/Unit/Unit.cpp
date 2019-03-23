/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "Unit.h"
#include "ClientPackets.h"
#include "GameContent.h"
#include "GameRule.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "NPC.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RegionContainer.h"
#include "Skill.h"
#include "World.h"
#include "WorldSession.h"
#include <limits>

// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable : 4355)
#endif

Unit::Unit(bool isWorldObject) : WorldObject(isWorldObject), m_unitTypeMask(0)
{
#ifdef _MSC_VER
#pragma warning(default : 4355)
#endif
    _mainType = MT_StaticObject;
    _objType = OBJ_STATIC;
    _subType = ST_Object;
    _bIsInWorld = false;
}

Unit::~Unit()
{
    uint ct = sWorld.GetArTime();

    for (auto &skill : m_vSkillList)
    {
        Skill::DB_InsertSkill(this, skill->m_nSkillUID, skill->m_nSkillID, skill->m_nSkillLevel, std::max(0, static_cast<int32_t>(skill->m_nNextCoolTime - ct)));
        delete skill;
        skill = nullptr;
    }
    m_vSkillList.clear();

    for (auto &pState : m_vStateList)
    {
        pState->DeleteThis();
    }
    m_vStateList.clear();
}

void Unit::_InitTimerFieldsAndStatus()
{
    auto ct = sWorld.GetArTime();
    SetUInt32Value(UNIT_LAST_STATE_PROC_TIME, ct);
    SetUInt32Value(UNIT_LAST_UPDATE_TIME, ct);
    SetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME, ct);
    SetUInt32Value(UNIT_LAST_SAVE_TIME, ct);
    SetUInt32Value(UNIT_LAST_HATE_UPDATE_TIME, ct);
    SetFlag(UNIT_FIELD_STATUS, (STATUS_NEED_TO_CALCULATE_STAT | STATUS_ATTACKABLE | STATUS_SKILL_CASTABLE | STATUS_MOVABLE | STATUS_MAGIC_CASTABLE | STATUS_ITEM_USABLE | STATUS_MORTAL));
}

void Unit::AddToWorld()
{
    if (!IsInWorld())
    {
        WorldObject::AddToWorld();
    }
}

void Unit::RemoveFromWorld()
{
    // cleanup
    ASSERT(GetHandle());

    if (IsInWorld())
        WorldObject::RemoveFromWorld();
}

void Unit::CleanupsBeforeDelete(bool finalCleanup)
{
}

void Unit::Update(uint32_t p_time)
{
    if (!IsInWorld())
        return;
}

void Unit::EnterPacket(XPacket &pEnterPct, Unit *pUnit, Player *pPlayer)
{
    pEnterPct << (uint32_t)Messages::GetStatusCode(pUnit, pPlayer);
    pEnterPct << pUnit->GetOrientation();
    pEnterPct << (int32_t)pUnit->GetHealth();
    pEnterPct << (int32_t)pUnit->GetMaxHealth();
    pEnterPct << (int32_t)pUnit->GetMana();
    pEnterPct << (int32_t)pUnit->GetMaxMana();
    pEnterPct << (int32_t)pUnit->GetLevel();
    pEnterPct << (uint8_t)pUnit->GetUInt32Value(UNIT_FIELD_RACE);
    pEnterPct << (uint32_t)pUnit->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    pEnterPct << (uint8_t)(pUnit->HasFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ENTER) ? 1 : 0);
    pEnterPct << (int32_t)0;
}

Item *Unit::GetWornItem(ItemWearType idx)
{
    if ((uint)idx >= MAX_ITEM_WEAR || idx < 0)
        return nullptr;
    return m_anWear[idx];
}

void Unit::SetHealth(int hp)
{
    auto old_hp = GetHealth();
    SetInt32Value(UNIT_FIELD_HEALTH, hp);
    if (hp > GetMaxHealth())
        SetHealth(GetMaxHealth());
    if (hp < 0)
        SetHealth(0);
    // @todo
    //if (old_hp != GetHealth())
    //this.onHPChange(old_hp);
}

void Unit::SetMana(int mp)
{
    auto old_mp = GetMana();
    SetInt32Value(UNIT_FIELD_MANA, mp);
    if (mp > GetMaxMana())
        SetMana(GetMaxMana());
    if (mp < 0)
        SetMana(0);
    /// @todo: on mana change
}

void Unit::CleanupBeforeRemoveFromMap(bool finalCleanup)
{
}

void Unit::SetMultipleMove(std::vector<Position> &_to, uint8_t _speed, uint _start_time)
{
    ArMoveVector::SetMultipleMove(_to, _speed, _start_time, sWorld.GetArTime());
    lastStepTime = start_time;
}

void Unit::SetMove(Position _to, uint8_t _speed, uint _start_time)
{
    ArMoveVector::SetMove(_to, _speed, _start_time, sWorld.GetArTime());
    lastStepTime = start_time;
}

void Unit::processPendingMove()
{
    uint ct = sWorld.GetArTime();
    Position pos{}; //             = ArPosition ptr -10h

    if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
    {
        if (m_nMovableTime < ct)
        {
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED);
            if (IsActable() && IsMovable())
            {
                pos = GetCurrentPosition(ct);
                sWorld.SetMultipleMove(this, pos, m_PendingMovePos, m_nPendingMoveSpeed, true, ct, true);
                if (IsPlayer())
                {
                    //auto p = dynamic_cast<Player*>(this);
                    /*if (p.m_nRideIdx != 0)
                    {
                        Summon ro = p.GetRideObject();
                        RappelzServer.Instance.SetMultipleMove(ro, pos, this.m_PendingMovePos, (sbyte)this.m_nPendingMoveSpeed, true, ct, true);
                    }*/
                }
            }
        }
    }
}

void Unit::OnUpdate()
{
    uint ct = sWorld.GetArTime();
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_CALCULATE_STAT))
    {
        CalculateStat();
        RemoveFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_CALCULATE_STAT);
    }
    this->regenHPMP(ct);

    if (!m_vAura.empty())
    {
        for (auto it = m_vAura.begin(); it != m_vAura.end();)
        {
            if ((*it).first->GetAuraRefreshTime() + 500 <= ct)
            {
                if (!onProcAura((*it).first, (*it).second))
                {
                    TS_SC_AURA auraMsg{};
                    auraMsg.caster = GetHandle();
                    auraMsg.skill_id = (*it).first->GetSkillId();
                    auraMsg.status = false;

                    if (IsPlayer())
                        this->As<Player>()->SendPacket(auraMsg);
                    else if (IsSummon())
                        this->As<Summon>()->GetMaster()->SendPacket(auraMsg);

                    it = m_vAura.erase(it);
                    continue;
                }
                (*it).first->SetAuraRefreshTime(ct);
            }
            ++it;
        }
    }

    if (!IsInWorld())
        return;

    if (!m_vStateList.empty() && GetUInt32Value(UNIT_LAST_STATE_PROC_TIME) + 100 < ct)
    {
        procStateDamage(ct);
        procState(ct);
        if (ClearExpiredState(ct))
        {
            CalculateStat();
        }
        SetUInt32Value(UNIT_LAST_STATE_PROC_TIME, ct);
    }

    if (IsFeared() && (!IsMoving(sWorld.GetArTime()) || !HasFlag(UNIT_FIELD_STATUS, STATUS_MOVING_BY_FEAR)))
    {
        auto pState = GetState(StateCode::SC_FEAR);
        if (pState != nullptr)
        {
            int nMoveSpeedAdd = pState->GetValue(1);
            Position newPos{};
            float theta = static_cast<float>(irand(1, 628)) / 100.0f;
            newPos.Relocate(GetPositionX() + std::sin(theta) * 120, GetPositionY() + std::cos(theta) * 120);

            if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVING_BY_FEAR))
            {
                SetFlag(UNIT_FIELD_STATUS, STATUS_MOVING_BY_FEAR);
                auto pCaster = sMemoryPool.GetObjectInWorld<Unit>(pState->GetCaster(SG_NORMAL));
                if (pCaster != nullptr)
                {
                    auto myPos = GetCurrentPosition(sWorld.GetArTime());
                    auto distance = myPos.GetExactDist2d(pCaster);

                    if (distance > 0)
                    {
                        newPos.SetCurrentXY(myPos.GetPositionX() + (pCaster->GetPositionX() - myPos.GetPositionX()) * 120 / distance,
                                            myPos.GetPositionY() + (pCaster->GetPositionY() - myPos.GetPositionY()) * 120 / distance);
                    }
                }
            }
            int try_cnt{10};
            while (--try_cnt > 0 && GameContent::IsBlocked(newPos.GetPositionX(), newPos.GetPositionY()))
            {
                float theta = static_cast<float>(irand(1, 628)) / 100.0f;
                newPos.Relocate(GetPositionX() + std::sin(theta) * 120, GetPositionY() + std::cos(theta) * 120);
            }

            if (newPos.GetPositionX() > sWorld.getIntConfig(CONFIG_MAP_WIDTH))
                newPos.SetCurrentXY(sWorld.getIntConfig(CONFIG_MAP_WIDTH), newPos.GetPositionY());
            if (newPos.GetPositionY() > sWorld.getIntConfig(CONFIG_MAP_HEIGHT))
                newPos.SetCurrentXY(newPos.GetPositionX(), sWorld.getIntConfig(CONFIG_MAP_HEIGHT));
            if (newPos.GetPositionX() < 0)
                newPos.SetCurrentXY(0, newPos.GetPositionY());
            if (newPos.GetPositionY() < 0)
                newPos.SetCurrentXY(newPos.GetPositionX(), 0);

            if (IsInWorld() && !GameContent::IsBlocked(newPos.GetPositionX(), newPos.GetPositionY()) && GetRealMoveSpeed() != 0)
                sWorld.SetMove(this, GetPosition(), newPos, static_cast<uint8_t>(GetRealMoveSpeed() + nMoveSpeedAdd / 7), true, sWorld.GetArTime());
        }
    }
}

void Unit::regenHPMP(uint t)
{
    float prev_mp;
    int prev_hp;
    float pt;

    uint et = t - GetUInt32Value(UNIT_LAST_UPDATE_TIME);
    if (et >= 300)
    {
        float etf = (float)et / 6000.0f;
        //prev_mp = et;

        if (GetHealth() != 0)
        {
            prev_mp = GetHealth();
            prev_hp = GetMana();
            if (!HasFlag(UNIT_FIELD_STATUS, STATUS_HP_REGEN_STOPPED))
            {
                pt = GetMaxHealth() * m_Attribute.nHPRegenPercentage;
                pt = pt * 0.01f * etf; // + 0.0;
                pt = pt + m_Attribute.nHPRegenPoint * etf;
                if (IsSitDown())
                {
                    pt *= 2.0f;
                }
                /*if (this.IsSitDown()) {
                    pt *= this.m_fHealRatioByRest;
                    pt += (float) this.m_nAdditionalHealByRest;
                }*/
                if (pt < 1.0f)
                    pt = 1.0f;
                pt *= GetFloatValue(UNIT_FIELD_HP_REGEN_MOD);
                auto pti = static_cast<int>(pt);
                if (pti != 0.0)
                {
                    AddHealth(pti);
                }
                /*this.m_nHPDecPart = (int) ((pt - (double) pti) * 100.0 + (double) this.m_nHPDecPart);
                int part = this.m_nHPDecPart / 100;
                if (part != 0) {
                    this.AddHP(part);
                    this.m_nHPDecPart = this.m_nHPDecPart % 100;
                }*/
            }
            if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MP_REGEN_STOPPED))
            {
                pt = GetMaxMana() * m_Attribute.nMPRegenPercentage;
                pt = pt * 0.01f * etf; // +0.0;
                pt = pt + m_Attribute.nMPRegenPoint * etf;

                if (IsSitDown())
                {
                    pt *= 2.0f;
                }

                /*if (this.IsSitDown())
                    pt = this.m_fMPHealRatioByRest * pt;*/
                if (pt < 1.0f)
                    pt = 1.0f;
                pt *= GetFloatValue(UNIT_FIELD_MP_REGEN_MOD);

                if (pt != 0.0)
                    AddMana((int)pt);
            }
            if (prev_hp != GetHealth() || prev_mp != GetMana())
            {
                this->m_fRegenMP += (GetMana() - prev_mp);
                this->m_nRegenHP += GetHealth() - prev_hp;
                if (GetMaxHealth() == GetHealth() || GetMaxMana() == GetMana() || 100 * m_nRegenHP / GetMaxHealth() > 3 || 100 * m_fRegenMP / GetMaxMana() > 3)
                {
                    TS_SC_REGEN_HPMP hpmpPct{};
                    hpmpPct.handle = GetHandle();
                    hpmpPct.hp_regen = m_nRegenHP;
                    hpmpPct.mp_regen = m_fRegenMP;
                    hpmpPct.hp = GetHealth();
                    hpmpPct.mp = GetMana();

                    this->m_nRegenHP = 0;
                    this->m_fRegenMP = 0;
                    if (IsInWorld())
                    {
                        sWorld.Broadcast((uint)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), GetLayer(), hpmpPct);
                    }
                    /*if (this.IsSummon())
                    {
                        Summon s = (Summon) this;
                        Player player = s.m_master;
                        if (player != null)
                        {
                            if (player.bIsInWorld && (player.m_StatusFlag & CREATURE_STATUS.STATUS_LOGIN_COMPLETE) != 0)
                            {
                                if (player.m_nLogoutTime == 0)
                                    player.Connection.SendTCP(pak);
                            }
                        }
                    }*/
                }
            }
        }
        SetUInt32Value(UNIT_LAST_UPDATE_TIME, t);
    }
}

int Unit::GetBaseSkillLevel(int skill_id) const
{
    Skill *s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel;
}

Skill *Unit::GetSkill(int skill_id) const
{
    for (auto &s : m_vSkillList)
    {
        if (s->m_nSkillID == skill_id)
            return s;
    }
    return nullptr;
}

void Unit::RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID)
{
    if (nJobID == 0)
        nJobID = GetCurrentJob();

    int nNeedJP = sObjectMgr.GetNeedJpForSkillLevelUp(skill_id, skill_level, nJobID);

    if (nNeedJP > GetJP())
        return;

    SetJP(GetJP() - nNeedJP);
    if (GetJP() < 0)
        SetJP(0);

    onExpChange();

    int64_t nSkillUID = 0;
    int nPrevLevel = GetBaseSkillLevel(skill_id);
    if (nPrevLevel == 0)
        nSkillUID = sWorld.GetSkillIndex();

    auto pSkill = SetSkill(nSkillUID, skill_id, skill_level, remain_cool_time);
    if (pSkill == nullptr)
        return;

    if (pSkill->GetSkillBase()->IsPassive())
        CalculateStat();

    onRegisterSkill(pSkill->m_nSkillUID, skill_id, nPrevLevel, skill_level);
}

int Unit::GetCurrentSkillLevel(int skill_id) const
{
    auto s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel + 0;
}

Skill *Unit::SetSkill(int skill_uid, int skill_id, int skill_level, int remain_cool_time)
{
    if (!sObjectMgr.GetSkillBase(skill_id))
        return nullptr;

    auto pSkill = GetSkill(skill_id);
    if (pSkill == nullptr)
    {
        pSkill = new Skill{this, skill_uid, skill_id};
        m_vSkillList.emplace_back(pSkill);
        if (pSkill->GetSkillBase()->IsPassive())
            m_vPassiveSkillList.emplace_back(pSkill);
        else
            m_vActiveSkillList.emplace_back(pSkill);
    }

    pSkill->m_nSkillLevel = skill_level;
    if (remain_cool_time != 0)
        pSkill->SetRemainCoolTime(remain_cool_time);

    return pSkill;
}

int Unit::CastSkill(int nSkillID, int nSkillLevel, uint32_t target_handle, Position pos, uint8_t layer, bool bIsCastedByItem)
{
    Position targetPos = pos;

    if (IsUsingSkill())
        return TS_RESULT_NOT_ACTABLE;

    if (IsPlayer() && As<Player>()->IsUsingStorage())
        return TS_RESULT_NOT_ACTABLE;

    auto pSkill = GetSkill(nSkillID);
    if (pSkill == nullptr)
        return TS_RESULT_NOT_ACTABLE;

    Unit *pSkillTarget{nullptr};
    auto pObj = sMemoryPool.GetObjectInWorld<WorldObject>(target_handle);
    if (pObj != nullptr && pObj->IsUnit())
        pSkillTarget = pObj->As<Unit>();

    switch (pSkill->GetSkillBase()->GetSkillTargetType())
    {
    case TARGET_TYPE::TARGET_MASTER:
    {
        if (!IsSummon())
            return TS_RESULT_NOT_ACTABLE;
        auto pSummon = As<Summon>();
        if (pSummon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
            return TS_RESULT_NOT_ACTABLE;
    }
    break;
    case TARGET_TYPE::TARGET_SELF_WITH_MASTER:
    {
        if (!IsSummon())
            return TS_RESULT_NOT_ACTABLE;
        auto pSummon = As<Summon>();
        if (pSkillTarget->GetHandle() != GetHandle() && pSummon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
            return TS_RESULT_NOT_ACTABLE;
    }
    break;
    case TARGET_TYPE::TARGET_TARGET_EXCEPT_CASTER:
        if (pSkillTarget == this)
            return TS_RESULT_NOT_ACTABLE;
        break;
    default:
        break;
    }

    if (pSkillTarget == nullptr)
    {
        if (nSkillID == SKILL_RETURN_FEATHER && IsPlayer() /* && IsInSiegeOrRaidDungeon*/)
            return TS_RESULT_NOT_ACTABLE;
    }
    else
    {
        if (!pSkillTarget->IsInWorld())
            return TS_RESULT_NOT_ACTABLE;

        uint32_t ct = sWorld.GetArTime();

        auto enemyPosition = pSkillTarget->GetCurrentPosition(ct);
        auto myPosition = GetCurrentPosition(ct);
        auto distance = myPosition.GetExactDist2d(&enemyPosition) - GetUnitSize() / 2 - pSkillTarget->GetUnitSize() / 2;

        float range_mod = 1.2f;
        if (pSkillTarget->IsMoving())
            range_mod = 1.5f;

        if (pSkill->GetSkillBase()->GetCastRange() == -1)
        {
            if (distance > GetRealAttackRange() * range_mod)
                return TS_RESULT_TOO_FAR;
        }
        else if (distance > pSkill->GetSkillBase()->GetCastRange() * GameRule::DEFAULT_UNIT_SIZE * range_mod)
        {
            return TS_RESULT_TOO_FAR;
        }

        if (pSkill->GetSkillBase()->IsValidToCorpse() != 0 && !pSkillTarget->IsDead())
            return TS_RESULT_NOT_ACTABLE;

        if (pSkillTarget == this || (pSkillTarget->IsSummon() && pSkillTarget->As<Summon>()->GetMaster() == this))
        {
            if (!pSkill->GetSkillBase()->IsUsable(SkillBase::USE_SELF))
                return TS_RESULT_NOT_ACTABLE;
        }
        else if (IsAlly(pSkillTarget))
        {
            if (!pSkill->GetSkillBase()->IsUsable(SkillBase::USE_ALLY))
                return TS_RESULT_NOT_ACTABLE;
        }
        else if (IsEnemy(pSkillTarget, false))
        {
            if (!pSkill->GetSkillBase()->IsUsable(SkillBase::USE_ENEMY))
                return TS_RESULT_NOT_ACTABLE;
        }
        else if (!pSkill->GetSkillBase()->IsUsable(SkillBase::USE_NEUTRAL))
        {
            return TS_RESULT_NOT_ACTABLE;
        }

        if (pSkillTarget->IsPlayer() && !pSkill->GetSkillBase()->IsUseableOnAvatar())
            return TS_RESULT_NOT_ACTABLE;
        if (pSkillTarget->IsMonster() && !pSkill->GetSkillBase()->IsUseableOnMonster())
            return TS_RESULT_NOT_ACTABLE;
        if (pSkillTarget->IsSummon() && !pSkill->GetSkillBase()->IsUseableOnSummon())
            return TS_RESULT_NOT_ACTABLE;

        targetPos = pSkillTarget->GetCurrentPosition(sWorld.GetArTime());
    }

    SetDirection(targetPos);
    m_castingSkill = pSkill;
    int nResult = pSkill->Cast(nSkillLevel, target_handle, targetPos, layer, bIsCastedByItem);
    if (nResult != TS_RESULT_SUCCESS)
        m_castingSkill = nullptr;

    return nResult;
}

void Unit::onAttackAndSkillProcess()
{
    if (m_castingSkill != nullptr)
    {
        m_castingSkill->ProcSkill();
    }
    else
    {
        if (GetTargetHandle() != 0)
            processAttack();
    }
}

bool Unit::StartAttack(uint target, bool bNeedFastReaction)
{
    if (GetHealth() == 0)
    {
        return false;
    }
    else
    {
        SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, target);
        RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED);
        if ((IsUsingBow() || IsUsingCrossBow()) && IsPlayer())
            m_nNextAttackMode = 1;
        if (bNeedFastReaction)
            onAttackAndSkillProcess();
        return true;
    }
}

void Unit::processAttack()
{
    uint t = sWorld.GetArTime();
    if (!IsAttackable())
        return;

    Player *player{nullptr};

    if (GetNextAttackableTime() <= t)
    {
        if ((IsUsingCrossBow() || IsUsingBow()) && IsPlayer())
        {
            auto bullets = GetBulletCount();
            if (bullets < 1)
            {
                EndAttack();
                return;
            }
        }

        auto enemy = sMemoryPool.GetObjectInWorld<Unit>(GetTargetHandle());
        if (enemy == nullptr)
        {
            if (IsPlayer())
                Messages::SendCantAttackMessage(this->As<Player>(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);
            else if (IsSummon())
                Messages::SendCantAttackMessage(this->As<Summon>()->GetMaster(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);

            EndAttack();
            return;
        }

        if (IsDead())
        {
            CancelAttack();
            return;
        }

        if (IsMoving(t))
            return;

        if (!IsAttacking())
            return;

        if (enemy == nullptr || !IsEnemy(enemy, false) || enemy->IsDead() || sRegion.IsVisibleRegion(this, enemy) == 0)
        {
            if (IsPlayer())
                Messages::SendCantAttackMessage(this->As<Player>(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);
            else if (IsSummon())
                Messages::SendCantAttackMessage(this->As<Summon>()->GetMaster(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);

            EndAttack();
            return;
        }

        if (HasFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK))
        {
            enemy->OnUpdate();
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
        }

        if (enemy->IsDead())
        {
            if (IsPlayer())
                Messages::SendCantAttackMessage(this->As<Player>(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);
            else if (IsSummon())
                Messages::SendCantAttackMessage(this->As<Summon>()->GetMaster(), GetHandle(), GetTargetHandle(), TS_RESULT_NOT_EXIST);

            EndAttack();
            return;
        }

        auto enemyPosition = enemy->GetCurrentPosition(t);
        auto myPosition = GetCurrentPosition(t);

        auto real_distance = myPosition.GetExactDist2d(&enemyPosition) - ((enemy->GetUnitSize() * 0.5f) + (GetUnitSize() * 0.5f));
        auto attack_range = GetRealAttackRange();
        SetDirection(enemyPosition);
        if (enemy->bIsMoving)
            attack_range *= 1.5f;
        else
            attack_range *= 1.2f;

        AttackInfo Damages[4]{};

        bool _bDoubleAttack{false};

        uint attack_interval = GetAttackInterval();
        auto attInt = GetAttackInterval();
        if (attack_range < real_distance)
        {
            onCantAttack(enemy->GetHandle(), t);
            return;
        }

        int next_mode = m_nNextAttackMode;
        // If Bow/Crossbow
        if ((IsUsingBow() || IsUsingCrossBow()) && IsPlayer())
        {
            if (m_nNextAttackMode == 1)
            {
                attInt = (uint)(GetBowAttackInterval() * 0.8f);
                SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, attInt + t);
                m_nNextAttackMode = 0;
                SetFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED);

                bool bFormChanged = HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED);
                if (bFormChanged)
                {
                    // @todo: form changed
                }
                if (!bFormChanged || next_mode != 1)
                {
                    auto delay = (int)(10 * (GetNextAttackableTime() - t));
                    broadcastAttackMessage(enemy, Damages, (int)(10 * attInt), delay, _bDoubleAttack, next_mode == 1, false, false);
                }
                return;
            }
            attInt = (uint)(GetBowAttackInterval() * 0.2f);
            attack_interval = attInt + GetBowInterval();
            m_nNextAttackMode = 1;
        }
        else
        {
            next_mode = 0;
        }
        if (next_mode == 0)
        {
            if ((IsUsingCrossBow() || IsUsingBow()) && IsPlayer())
            {
                player = dynamic_cast<Player *>(this);
                player->EraseBullet(1);
            }
            m_nMovableTime = attInt + t;
            Attack(enemy, t, attack_interval, Damages, _bDoubleAttack);
        }
        SetFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED);

        if (next_mode != 1)
        {
            auto delay = (int)(10 * (GetNextAttackableTime() - t));
            broadcastAttackMessage(enemy, Damages, (int)(10 * attInt), delay, _bDoubleAttack, next_mode == 1, false, false);
        }
        return;
    }
}

void Unit::Attack(Unit *pTarget, uint ct, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack)
{
    if (ct == 0)
        ct = sWorld.GetArTime();

    SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, attack_interval + ct);

    int nHate = 0;
    int nAttackCount = 1;

    bIsDoubleAttack = false;

    if (IsUsingDoubleWeapon())
        nAttackCount *= 2;

    if (irand(1, 100) < m_Attribute.nDoubleAttackRatio)
    {
        bIsDoubleAttack = true;
        nAttackCount *= 2;
    }

    for (int i = 0; i < nAttackCount; ++i)
    {
        bool bLeftHandAttack = IsUsingDoubleWeapon() && i % 2;

        auto prev_target_hp = pTarget->GetHealth();
        auto prev_target_mp = pTarget->GetMana();
        auto prev_hp = GetHealth();
        auto prev_mp = GetMana();

        int nCriticalBonus{0};

        if (GetInt32Value(BATTLE_FIELD_CRIT_COUNT) > 0)
            nCriticalBonus = 50;

        if (bLeftHandAttack)
            arDamage[i].SetDamageInfo(pTarget->DealPhysicalNormalLeftHandDamage(this, GetAttackPointLeft(), ElementalType::TYPE_NONE, 0, nCriticalBonus));
        else
            arDamage[i].SetDamageInfo(pTarget->DealPhysicalNormalDamage(this, GetAttackPointRight(), ElementalType::TYPE_NONE, 0, nCriticalBonus));

        if (arDamage[i].bCritical)
        {
            SetInt32Value(BATTLE_FIELD_CRIT_COUNT, GetInt32Value(BATTLE_FIELD_CRIT_COUNT) + 1);
            if (GetInt32Value(BATTLE_FIELD_CRIT_COUNT) >= 3)
            {
                SetInt32Value(BATTLE_FIELD_CRIT_COUNT, 0);
            }

            ProcessAddHPMPOnCritical();
        }

        bool bSuccess = !arDamage[i].bMiss;
        if (bSuccess)
        {
            AddStateByAttack(pTarget, true, false, false, true);
            pTarget->AddStateByAttack(this, false, false, false, true);

            for (const auto &it : m_vAbsorbByNormalAttack)
            {
                if (irand(1, 100) < it.ratio)
                {
                    if (it.hp_absorb_ratio != 0)
                        AddHealth(it.hp_absorb_ratio * arDamage[i].nDamage);
                    if (it.mp_absorb_ratio != 0)
                        AddMana(it.mp_absorb_ratio * arDamage[i].nDamage);
                }
            }

            for (const auto &it : m_vHealOnAttack)
            {
                if (irand(1, 100) < it.ratio)
                {
                    if (it.hp_inc)
                        AddHealth(it.hp_inc);
                    if (it.mp_inc)
                        AddMana(it.mp_inc);
                }
            }

            for (const auto &it : m_vStealOnAttack)
            {
                if (irand(1, 100) < it.ratio)
                {
                    if (it.hp_steal)
                    {
                        int nHP = std::min(pTarget->GetHealth(), it.hp_steal);
                        AddHealth(nHP);
                        pTarget->damage(this, nHP);
                    }
                    if (it.mp_steal)
                    {
                        int nMP = std::min(pTarget->GetMana(), it.mp_steal);
                        AddMana(nMP);
                        pTarget->AddMana(-nMP);
                    }
                }
            }
        }

        // @todo: reduce Endurance here

        arDamage[i].nDamage = prev_target_hp - pTarget->GetHealth();
        arDamage[i].mp_damage = static_cast<int16_t>(prev_target_mp - pTarget->GetMana());
        arDamage[i].attacker_damage = static_cast<int16_t>(prev_hp - GetHealth());
        arDamage[i].attacker_mp_damage = static_cast<int16_t>(prev_mp - GetMana());

        arDamage[i].target_hp = pTarget->GetHealth();
        arDamage[i].target_mp = pTarget->GetMana();
        arDamage[i].attacker_hp = GetHealth();
        arDamage[i].attacker_mp = GetMana();

        nHate += arDamage[i].nDamage;
        // @todo: Havoc here
    }

    if (pTarget->IsMonster())
    {
        bool bRange = (IsUsingBow() || IsUsingCrossBow()) && IsPlayer();
        auto HateMod = GetHateMod(3, true);

        nHate += HateMod.second;
        nHate *= HateMod.first;

        if (bRange)
            pTarget->As<Monster>()->AddHate(GetHandle(), GetFloatValue(UNIT_FIELD_HATE_RATIO) * nHate * m_RangeStateAdvantage.fHate * pTarget->m_RangeStatePenalty.fHate);
        else
            pTarget->As<Monster>()->AddHate(GetHandle(), GetFloatValue(UNIT_FIELD_HATE_RATIO) * nHate * m_NormalStateAdvantage.fHate * pTarget->m_NormalStatePenalty.fHate);
    }
    else if (pTarget->IsNPC())
    {
        //pTarget->As<NPC>()->SetAttacker(pTarget);
    }
}

DamageInfo Unit::DealPhysicalNormalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo damage_info{};

    int nTargetGroup = pFrom->GetCreatureGroup();
    StateMod damageReduceByState{};

    for (auto &dit : m_vDamageReduceValueInfo)
    {
        if (dit.IsAppliableCreatureGroup(nTargetGroup) && dit.ratio > irand(1, 100))
        {
            damageReduceByState.nDamage -= dit.physical_reduce;
        }
    }

    float fReduce{0.0f};
    for (auto &dit : m_vDamageReducePercentInfo)
    {
        if (dit.IsAppliableCreatureGroup(nTargetGroup) && dit.ratio > irand(1, 100))
        {
            damageReduceByState.fDamage -= dit.physical_reduce;
            if (damageReduceByState.fDamage < 0.0f)
                damageReduceByState.fDamage = 0.0f;
        }
    }

    if (nDamage < 0)
        nDamage = 0;

    damage_info.SetDamage(DealPhysicalLeftHandDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, &damageReduceByState, &(pFrom->m_NormalStateAdvantage)));

    if (!damage_info.bMiss && !damage_info.bPerfectBlock)
    {
        for (auto &it : pFrom->m_vNormalAdditionalDamage)
        {
            if (it.ratio > irand(1, 100))
            {
                int damage{0};
                if (it.nDamage != 0)
                    damage = it.nDamage;
                else
                    damage = it.fDamage * damage_info.nDamage;

                damage = DealAdditionalLeftHandDamage(pFrom, damage, it.type).nDamage;
                damage_info.elemental_damage[it.type] += damage;
                damage_info.nDamage += damage;
            }
        }
    }

    ProcessAdditionalDamage(damage_info, DT_ADDITIONAL_LEFT_HAND_DAMAGE, pFrom->m_vNormalAdditionalDamage, pFrom, nDamage, elemental_type);

    damage_info.target_hp = GetHealth();
    return damage_info;
}

DamageInfo Unit::DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo damage_info{};

    int nTargetGroup = pFrom->GetCreatureGroup();
    StateMod damageReduceByState{};

    bool bRange = (pFrom->IsUsingBow() || pFrom->IsUsingCrossBow()) && pFrom->IsPlayer();

    if (bRange)
        damageReduceByState = m_RangeStatePenalty;
    else
        damageReduceByState = m_NormalStatePenalty;

    for (auto &dit : m_vDamageReduceValueInfo)
    {
        if (dit.IsAppliableCreatureGroup(nTargetGroup) && dit.ratio > irand(1, 100))
            damageReduceByState.nDamage -= dit.physical_reduce;
    }

    float fReduce{0.0f};
    for (auto &dit : m_vDamageReducePercentInfo)
    {
        if (dit.IsAppliableCreatureGroup(nTargetGroup) && dit.ratio > irand(1, 100))
        {
            damageReduceByState.fDamage -= dit.physical_reduce;
            if (damageReduceByState.fDamage < 0.0f)
                damageReduceByState.fDamage = 0.0f;
        }
    }

    if (nDamage < 0)
        nDamage = 0;

    if (bRange)
        damage_info.SetDamage(DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, &damageReduceByState, &(pFrom->m_RangeStateAdvantage)));
    else
        damage_info.SetDamage(DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, &damageReduceByState, &(pFrom->m_NormalStateAdvantage)));

    if (!damage_info.bMiss && !damage_info.bPerfectBlock)
    {
        std::vector<AdditionalDamageInfo> AdditionalDamage{};
        if (bRange)
            AdditionalDamage = (pFrom->m_vRangeAdditionalDamage);
        else
            AdditionalDamage = (pFrom->m_vNormalAdditionalDamage);

        for (auto &it : AdditionalDamage)
        {
            if (it.ratio > irand(1, 100))
            {
                int damage{0};
                if (it.nDamage != 0)
                    damage = it.nDamage;
                else
                    damage = it.fDamage * damage_info.nDamage;

                damage = DealAdditionalDamage(pFrom, damage, it.type).nDamage;
                damage_info.elemental_damage[it.type] += damage;
                damage_info.nDamage += damage;
            }
        }
    }

    ProcessAdditionalDamage(damage_info, DT_ADDITIONAL_DAMAGE,
                            bRange ? pFrom->m_vRangeAdditionalDamage : pFrom->m_vNormalAdditionalDamage, pFrom, nDamage, elemental_type);

    damage_info.target_hp = GetHealth();
    return damage_info;
}

Damage Unit::DealDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    int nPrevHP = GetHealth();

    Damage damage_result = pFrom->CalcDamage(this, damageType, nDamage, elemental_type, accuracy_bonus, 1.0f + (damage_penalty != nullptr ? damage_penalty->fCritical - 1.0f : 0.0f) + (damage_advantage != nullptr ? damage_advantage->fCritical - 1.0f : 0.0f), critical_bonus + (damage_advantage != nullptr ? damage_advantage->nCritical : 0) + (damage_penalty != nullptr ? damage_penalty->nCritical : 0), nFlag);

    if (!damage_result.bMiss)
    {
        if (damage_penalty != nullptr)
        {
            damage_result.nDamage += damage_penalty->nDamage;
            damage_result.nDamage *= damage_penalty->fDamage;
        }

        if (damage_advantage != nullptr)
        {
            damage_result.nDamage += damage_advantage->nDamage;
            damage_result.nDamage *= damage_advantage->fDamage;
        }
    }

    if (damage_result.nDamage < 0)
        damage_result.nDamage = 0;

    float mana_shield_absorb_ratio{0.0f};
    switch (damageType)
    {
    case DT_NORMAL_PHYSICAL_DAMAGE:
    case DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE:
    case DT_NORMAL_PHYSICAL_SKILL_DAMAGE:
    case DT_ADDITIONAL_DAMAGE:
    case DT_ADDITIONAL_LEFT_HAND_DAMAGE:
    case DT_STATE_PHYSICAL_DAMAGE:
        mana_shield_absorb_ratio = GetFloatValue(UNIT_FIELD_PHYSICAL_MANASHIELD_ABSORB_RATIO);
        break;
    case DT_ADDITIONAL_MAGICAL_DAMAGE:
    case DT_NORMAL_MAGICAL_DAMAGE:
    case DT_STATE_MAGICAL_DAMAGE:
        mana_shield_absorb_ratio = GetFloatValue(UNIT_FIELD_MAGICAL_MANASHIELD_ABSORB_RATIO);
        break;
    default:
        break;
    }

    if (mana_shield_absorb_ratio > 1.0f)
        mana_shield_absorb_ratio = 1.0f;
    else if (mana_shield_absorb_ratio < 0.0f)
        mana_shield_absorb_ratio = 0.0f;

    if (mana_shield_absorb_ratio > 0.0f)
    {
        int nAbsorbableDamage = damage_result.nDamage * mana_shield_absorb_ratio;
        if (GetMana() < nAbsorbableDamage)
            nAbsorbableDamage = GetMana();

        damage_result.nDamage -= nAbsorbableDamage;

        AddMana(-nAbsorbableDamage);
        Messages::BroadcastHPMPMessage(this, 0, nAbsorbableDamage, false);
    }

    int real_damage = onDamage(pFrom, elemental_type, damageType, damage_result.nDamage, damage_result.bCritical);
    damage(pFrom, real_damage);

    /* @todo
    if (IsUsingSkill())
        m_castingSkill->onDamage(damage_result.nDamage);*/

    if (!damage_result.bMiss)
    {
        RemoveStatesOnDamage();
        RemoveState(SC_SLEEP, GameRule::MAX_STATE_LEVEL);
        RemoveState(SC_NIGHTMARE, GameRule::MAX_STATE_LEVEL);

        if (damageType != DT_NORMAL_MAGICAL_DAMAGE && damageType != DT_STATE_MAGICAL_DAMAGE)
            RemoveState(SC_SHINE_WALL, GameRule::MAX_STATE_LEVEL);

        auto pFrozen = GetState(SC_FROZEN);
        if (pFrozen != nullptr)
        {
            int nRatio = (pFrozen->GetValue(4) + pFrozen->GetLevel() * pFrozen->GetValue(5)) * 100.0f;
            if (irand(1, 100) < nRatio)
                RemoveState(SC_FROZEN, GameRule::MAX_STATE_LEVEL);
        }

        // @todo: Endurance

        /*
        if (damageType == DT_NORMAL_PHYSICAL_DAMAGE || damageType == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE ||
            damageType == DT_NORMAL_PHYSICAL_SKILL_DAMAGE || damageType == DT_NORMAL_MAGICAL_DAMAGE ||
            damageType == DT_STATE_PHYSICAL_DAMAGE || damageType == DT_STATE_MAGICAL_DAMAGE)
        {
            if (pFrom != this && !pFrom->IsProcessingReflectDamage())
                pFrom->ProcEtherealDurabilityConsumption(true, damageType, damage_result.nDamage);
            ProcEtherealDurabilityConsumption(false, damageType, damage_result.nDamage);
        }*/
    }

    auto t = sWorld.GetArTime();
    auto myPos = GetCurrentPosition(sWorld.GetArTime());
    if (damage_result.bBlock && !m_vStateReflectInfo.empty() && pFrom->GetCurrentPosition(sWorld.GetArTime()).GetExactDist2d(&myPos) <= GameRule::REFLECT_RANGE)
    {
        for (auto &it : m_vStateReflectInfo)
        {
            if (!pFrom->IsNPC())
                pFrom->AddState(SG_NORMAL, it.nCode, GetHandle(), it.nLevel, t, t + it.nDuration);
        }
    }

    if (!IsProcessingReflectDamage() && !pFrom->IsProcessingReflectDamage() && !m_vDamageReflectInfo.empty())
    {
        SetFlag(UNIT_FIELD_STATUS, STATUS_PROCESSING_REFELCT);
        for (auto &it : m_vDamageReflectInfo)
        {
            myPos = GetCurrentPosition(sWorld.GetArTime());
            if (irand(1, 100) < it.fire_ratio && pFrom->GetCurrentPosition(sWorld.GetArTime()).GetExactDist2d(&myPos) <= it.range * GameRule::DEFAULT_UNIT_SIZE)
            {
                int nPrevHP = pFrom->GetHealth();
                int nPrevMP = pFrom->GetMana();
                int nDamageFlag = IGNORE_AVOID | IGNORE_BLOCK | IGNORE_CRITICAL;

                if (it.bIgnoreDefence)
                    nDamageFlag |= IGNORE_DEFENCE;

                if (it.nReflectDamage != 0 && (damageType == DT_NORMAL_PHYSICAL_DAMAGE || damageType == DT_NORMAL_MAGICAL_DAMAGE || damageType == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE || damageType == DT_NORMAL_PHYSICAL_SKILL_DAMAGE))
                    pFrom->DealAdditionalMagicalDamage(this, it.nReflectDamage, it.type, 0, 0, nDamageFlag);

                int nReflectDamage = it.fPhysicalReflectRatio * real_damage;
                if (nReflectDamage != 0 && (damageType == DT_NORMAL_PHYSICAL_DAMAGE || damageType == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE))
                    pFrom->DealPhysicalDamage(this, nReflectDamage, it.type, 0, 0, nDamageFlag);

                nReflectDamage = it.fPhysicalSkillReflectRatio * real_damage;
                if (nReflectDamage != 0 && (damageType == DT_NORMAL_PHYSICAL_SKILL_DAMAGE))
                    pFrom->DealPhysicalSkillDamage(this, nReflectDamage, it.type, 0, 0, nDamageFlag);

                nReflectDamage = it.fMagicalReflectRatio * real_damage;
                if (nReflectDamage != 0 && (damageType == DT_NORMAL_MAGICAL_DAMAGE))
                    pFrom->DealMagicalSkillDamage(this, nReflectDamage, it.type, 0, 0, nDamageFlag);

                Messages::BroadcastHPMPMessage(pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, false);
                if (IsPlayer())
                    Messages::SendHPMPMessage(this->As<Player>(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
                else if (IsSummon())
                    Messages::SendHPMPMessage(this->As<Summon>()->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);

                if (pFrom->IsPlayer())
                    Messages::SendHPMPMessage(pFrom->As<Player>(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
                else if (pFrom->IsSummon())
                    Messages::SendHPMPMessage(pFrom->As<Summon>()->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
            }
        }

        RemoveFlag(UNIT_FIELD_STATUS, STATUS_PROCESSING_REFELCT);
    }

    damage_result.nDamage = real_damage;
    return damage_result;
}

Damage Unit::DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_NORMAL_PHYSICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealPhysicalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealAdditionalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_ADDITIONAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealAdditionalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_ADDITIONAL_LEFT_HAND_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, type, DT_NORMAL_MAGICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealAdditionalMagicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, type, DT_ADDITIONAL_MAGICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag)
{
    Damage damage{};
    if (damage_type == DT_NORMAL_MAGICAL_DAMAGE || damage_type == DT_STATE_MAGICAL_DAMAGE)
    {
        if (pTarget->IsMagicalImmune())
        {
            damage.bMiss = true;
            damage.nDamage = 0;

            return damage;
        }
    }
    else if (pTarget->IsPhysicalImmune())
    {
        damage.bMiss = true;
        damage.nDamage = 0;

        return damage;
    }

    nDamage += nDamage * m_Expert[GetCreatureGroup()].fDamage;
    nDamage += nDamage * m_Expert[GetCreatureGroup()].fAvoid;

    int nFinalDamage{0};
    float fDamageAdjust{1.0f};

    bool bIsPhysicalDamage = (damage_type == DT_NORMAL_PHYSICAL_DAMAGE || damage_type == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE || damage_type == DT_STATE_PHYSICAL_DAMAGE || damage_type == DT_NORMAL_PHYSICAL_SKILL_DAMAGE);
    bool bIsMagicalDamage = (damage_type == DT_NORMAL_MAGICAL_DAMAGE || damage_type == DT_STATE_MAGICAL_DAMAGE);
    bool bIsStateDamage = (damage_type == DT_STATE_MAGICAL_DAMAGE || damage_type == DT_STATE_PHYSICAL_DAMAGE);
    bool bIsAdditionalDamage = (damage_type == DT_ADDITIONAL_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE || damage_type == DT_ADDITIONAL_MAGICAL_DAMAGE);
    bool bIsLeftHandDamage = (damage_type == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE);

    if (!(nFlag & IGNORE_AVOID) && bIsPhysicalDamage && !bIsStateDamage)
    {
        int nAccuracy;
        if (bIsLeftHandDamage)
            nAccuracy = GetAccuracyLeft();
        else
            nAccuracy = GetAccuracyRight();

        int nPercentage = 100;
        if (pTarget->GetAvoid() != 0)
            nPercentage = 7 + std::max(10, 88 + (GetLevel() - pTarget->GetLevel()) * 2) * ((float)nAccuracy / (float)pTarget->GetAvoid()) + accuracy_bonus;

        if (irand(1, 100) > nPercentage)
        {
            damage.bMiss = true;
            damage.nDamage = 0;

            return damage;
        }
    }

    if (!(nFlag & IGNORE_AVOID) && bIsMagicalDamage && !bIsStateDamage)
    {
        int nPercentage = 100;
        if (pTarget->GetMagicAvoid() != 0)
            nPercentage = 7 + std::max(10, 88 + (GetLevel() - pTarget->GetLevel()) * 2) * ((float)GetMagicAccuracy() / (float)pTarget->GetMagicAvoid()) + accuracy_bonus;

        if (irand(1, 100) > nPercentage)
        {
            damage.bMiss = true;
            damage.nDamage = 0;

            return damage;
        }
    }

    int nRandomDamage{0};
    int nDefence{0};

    if (!bIsAdditionalDamage && !bIsStateDamage && !IsSkillProp())
    {
        if (!(nFlag & IGNORE_DEFENCE))
        {
            if (bIsPhysicalDamage)
            {
                nDefence = pTarget->GetDefence();

                if ((pTarget->IsWearShield() || pTarget->IsSummon()) && ((irand(1, 100)) < pTarget->GetBlockChance()))
                {
                    if (irand(1, 100) < 20)
                    {
                        damage.bPerfectBlock = true;
                        damage.nDamage = 0;

                        return damage;
                    }
                    else
                    {
                        damage.bBlock = true;
                        nDefence += pTarget->GetBlockDefence();
                    }
                }
            }
            else if (bIsMagicalDamage)
            {
                nDefence = pTarget->GetMagicDefence();
            }
        }

        nFinalDamage = GetLevel() * 1.7f * std::max(1 - 0.4 * nDefence / nDamage, 0.3) + nDamage * std::max(1 - 0.5 * nDefence / nDamage, 0.05);

        if (nFinalDamage < 1)
            nFinalDamage = 1;

        if (!sWorld.getBoolConfig(CONFIG_IGNORE_RANDOM_DAMAGE))
        {
            nRandomDamage = nFinalDamage * 0.05f;
            nRandomDamage = irand(-nRandomDamage, nRandomDamage);
        }
    }
    else if (bIsStateDamage)
    {
        float fDefAdjust = 1.0f;
        if (bIsMagicalDamage && GetMagicPoint() < pTarget->GetMagicDefence())
        {
            fDefAdjust = 1.0f - (pTarget->GetMagicDefence() - GetMagicPoint()) / (((pTarget->GetMagicDefence() > 0) ? pTarget->GetMagicDefence() : 1) * 2);
        }
        else if (bIsPhysicalDamage && GetAttackPointRight() < pTarget->GetDefence())
        {
            fDefAdjust = 1.0f - (pTarget->GetDefence() - GetAttackPointRight()) / (((pTarget->GetDefence() > 0) ? pTarget->GetDefence() : 1) * 2);
        }

        nFinalDamage = nDamage * fDefAdjust;
    }
    else
    {
        nFinalDamage = nDamage;

        if (!sWorld.getBoolConfig(CONFIG_IGNORE_RANDOM_DAMAGE))
        {
            nRandomDamage = nFinalDamage * 0.05f;
            nRandomDamage = irand(-nRandomDamage, nRandomDamage);
        }
    }

    nFinalDamage += nRandomDamage;
    nFinalDamage *= fDamageAdjust;

    // ũ��Ƽ��
    if (!bIsAdditionalDamage && !(nFlag & IGNORE_CRITICAL))
    {
        int nCriticalDamage = GetCriticalDamage(nFinalDamage, critical_amp, critical_bonus);

        if (nCriticalDamage)
        {
            damage.bCritical = true;
            nFinalDamage += nCriticalDamage;
        }
    }

    if ((damage_type == DT_ADDITIONAL_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE) && IsUsingDoubleWeapon())
    {
        if (bIsLeftHandDamage)
            nFinalDamage *= (0.44f + m_nDoubleWeaponMasteryLevel * 0.02f);
        else
            nFinalDamage *= (0.90f + m_nDoubleWeaponMasteryLevel * 0.01f);
    }

    float fElementalResist = 1.0f - (pTarget->GetElementalResist(elemental_type) / 300);
    damage.nDamage = nFinalDamage * fElementalResist;
    damage.nResistedDamage = nFinalDamage - damage.nDamage;

    if ((pTarget->IsPlayer() || pTarget->IsSummon()) && pTarget != this)
    {
        if (IsPlayer())
            damage.nDamage *= GameRule::fPVPDamageRateForPlayer;
        else if (IsSummon())
            damage.nDamage *= GameRule::fPVPDamageRateForSummon;
    }

    if (bIsMagicalDamage && !bIsAdditionalDamage && nFinalDamage < 1)
        damage.nDamage = 1;

    return damage;
}

uint Unit::GetCreatureGroup() const
{
    return 0;
}

int Unit::damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead)
{
    if (IsDead())
        return 0;

    if (IsHiding())
    {
        RemoveState(SC_HIDE, GameRule::MAX_STATE_LEVEL);
        RemoveState(SC_TRACE_OF_FUGITIVE, GameRule::MAX_STATE_LEVEL);
    }

    SetHealth(GetHealth() > nDamage ? GetHealth() - nDamage : 0);
    if (IsDead())
    {
        SetUInt32Value(UNIT_FIELD_DEAD_TIME, sWorld.GetArTime());
        onDead(pFrom, decreaseEXPOnDead);
    }
    return nDamage;
}

void Unit::broadcastAttackMessage(Unit *pTarget, AttackInfo *arDamage, int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack)
{
    uint8_t attack_count = 1;
    if (bEndAttack || bCancelAttack)
        attack_count = 0;

    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED))
    {
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            attack_count *= 2;
        if (bIsDoubleAttack)
            attack_count *= 2;
    }

    TS_SC_ATTACK_EVENT pct{};
    pct.attacker_handle = GetHandle();
    if (pTarget != nullptr)
        pct.target_handle = pTarget->GetHandle();
    else
        pct.target_handle = 0;
    pct.attack_speed = static_cast<uint16_t>(tm);
    pct.attack_delay = static_cast<uint16_t>(delay);

    pct.attack_flag = AEAF_None;
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED))
    {
        if (bIsDoubleAttack)
            pct.attack_flag = AEAF_DoubleAttack;
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            pct.attack_flag = AEAF_DoubleWeapon;
        if (IsUsingBow() && IsPlayer())
            pct.attack_flag = AEAF_UsingBow;
        if (IsUsingCrossBow() && IsPlayer())
            pct.attack_flag = AEAF_UsingCrossBow;
    }

    pct.attack_action = AEAA_Attack;
    if (bIsAiming)
        pct.attack_action = AEAA_Aiming;
    else if (bEndAttack)
        pct.attack_action = AEAA_EndAttack;
    else if (bCancelAttack)
        pct.attack_action = AEAA_CancelAttack;

    for (int i = 0; i < attack_count; ++i)
    {
        ATTACK_INFO attack_info{};
        attack_info.damage = arDamage[i].nDamage;
        attack_info.mp_damage = arDamage[i].mp_damage;
        attack_info.flag = AIF_None;
        if (arDamage[i].bPerfectBlock)
            attack_info.flag = AIF_PerfectBlock;
        if (arDamage[i].bBlock)
            attack_info.flag = AIF_Block;
        if (arDamage[i].bMiss)
            attack_info.flag = AIF_Miss;
        if (arDamage[i].bCritical)
            attack_info.flag = AIF_Critical;

        std::copy(std::begin(arDamage->elemental_damage), std::end(arDamage->elemental_damage), std::begin((attack_info.elemental_damage)));

        attack_info.target_hp = arDamage[i].target_hp;
        attack_info.target_mp = arDamage[i].target_mp;
        attack_info.attacker_damage = arDamage[i].attacker_damage;
        attack_info.attacker_mp_damage = arDamage[i].attacker_mp_damage;
        attack_info.attacker_hp = arDamage[i].attacker_hp;
        attack_info.attacker_mp = arDamage[i].attacker_mp;
        pct.attack.emplace_back(attack_info);
    }
    sWorld.Broadcast((uint)GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE),
                     (uint)GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE), GetLayer(), pct);
}

void Unit::EndAttack()
{
    AttackInfo info[4]{};
    if ((IsUsingBow() || IsUsingCrossBow()) && IsPlayer() && m_nNextAttackMode == 0)
    {
        m_nNextAttackMode = 1;
        SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, sWorld.GetArTime());
    }
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED))
    {
        //auto target = dynamic_cast<Unit*>(sMemoryPool.getPtrFromId(GetTargetHandle()));
        auto target = sMemoryPool.GetObjectInWorld<Unit>(GetTargetHandle());
        if (IsPlayer() || IsSummon())
        {
            if (target != nullptr)
                broadcastAttackMessage(target, info, 0, 0, false, false, true, false);
        }
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
}

void Unit::onDead(Unit *pFrom, bool decreaseEXPOnDead)
{

    if (m_castingSkill != nullptr)
        CancelSkill();

    if (IsMoving() && IsInWorld())
    {
        auto pos = GetCurrentPosition(GetUInt32Value(UNIT_FIELD_DEAD_TIME));
        sWorld.SetMove(this, pos, pos, 0, true, sWorld.GetArTime(), true);
        if (IsPlayer() && this->As<Player>()->GetRideObject() != nullptr)
            sWorld.SetMove(this->As<Player>()->GetRideObject(), pos, pos, 0, true, sWorld.GetArTime(), true);
    }

    if (IsAttacking())
        EndAttack();

    RemoveAllAura();
    RemoveAllHate();
    removeStateByDead();
}

void Unit::AddEXP(int64_t exp, uint jp, bool bApplyStamina)
{
    SetUInt64Value(UNIT_FIELD_EXP, GetEXP() + exp);
    SetUInt32Value(UNIT_FIELD_JOBPOINT, GetJP() + jp);
    // SetTotalJP
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

void Unit::CancelSkill()
{
    if (m_castingSkill != nullptr && m_castingSkill->Cancel())
    {
        m_castingSkill = nullptr;
    }
}

void Unit::CancelAttack()
{
    AttackInfo info[4]{};
    if ((IsUsingCrossBow() || IsUsingBow()) && (IsPlayer() && m_nNextAttackMode == 0))
    {
        m_nNextAttackMode = 1;
        SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, sWorld.GetArTime());
    }
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED))
    {
        this->broadcastAttackMessage(sMemoryPool.GetObjectInWorld<Unit>(GetTargetHandle()), info, 0, 0, false, false, false, true);
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
}

bool Unit::TranslateWearPosition(ItemWearType &pos, Item *pItem, std::vector<int> *ItemList)
{
    if (pItem->GetWearType() == ItemWearType::WEAR_CANTWEAR)
        return false;

    if (!pItem->IsWearable())
        return false;

    int nLevel = m_nUnitExpertLevel > GetLevel() ? m_nUnitExpertLevel : GetLevel();
    if (pItem->GetLevelLimit() > nLevel)
        return false;

    if (pItem->GetItemBase()->use_min_level != 0 && GetLevel() < pItem->GetItemBase()->use_min_level)
        return false;

    if (pItem->GetItemBase()->use_max_level != 0 && GetLevel() > pItem->GetItemBase()->use_max_level)
        return false;

    return true;
}

ushort Unit::Puton(ItemWearType pos, Item *pItem, bool bIsTranslated)
{
    if (!pItem->IsInInventory())
        return TS_RESULT_ACCESS_DENIED;

    if (pItem->GetWearInfo() != WEAR_NONE)
        return TS_RESULT_NOT_ACTABLE;

    std::vector<int32_t> vOverlappedItemList{};

    if (!TranslateWearPosition(pos, pItem, &vOverlappedItemList))
        return TS_RESULT_NOT_ACTABLE;

    for (auto &it : vOverlappedItemList)
    {
        putoffItem(static_cast<ItemWearType>(it));
        if (m_anWear[it] != nullptr)
            return TS_RESULT_NOT_ACTABLE;
    }

    return putonItem(pos, pItem);
}

uint16_t Unit::putonItem(ItemWearType pos, Item *pItem)
{
    ASSERT(pos < MAX_SPARE_ITEM_WEAR, "putonItem: Position invalid!!");
    ASSERT(m_anWear[pos] == nullptr, "putonItem: m_anWear[pos] is empty!!");

    m_anWear[pos] = pItem;

    pItem->SetWearInfo(pos);
    pItem->SetBindedCreatureHandle(GetHandle());
    pItem->m_bIsNeedUpdateToDB = true;

    if ((pItem->IsBow() || pItem->IsCrossBow()) && pos < MAX_ITEM_WEAR)
        m_nNextAttackMode = static_cast<int32_t>(NEXT_ATTACK_MODE::AM_AIMING);

    if (IsPlayer())
        Messages::SendItemWearInfoMessage(this->As<Player>(), this, m_anWear[pos]);
    else if (IsSummon())
        Messages::SendItemWearInfoMessage(this->As<Summon>()->GetMaster(), this, m_anWear[pos]);

    return TS_RESULT_SUCCESS;
}

ushort Unit::Putoff(ItemWearType pos)
{
    if (pos == ItemWearType::WEAR_TWOHAND)
        pos = ItemWearType::WEAR_WEAPON;

    if (pos == ItemWearType::WEAR_TWOFINGER_RING)
        pos = ItemWearType::WEAR_RING;

    if ((pos >= MAX_SPARE_ITEM_WEAR && pos != ItemWearType::WEAR_TWOHAND) || pos < 0)
        return TS_RESULT_NOT_ACTABLE;

    ItemWearType absolute_pos = GetAbsoluteWearPos(pos);
    if (absolute_pos == WEAR_CANTWEAR)
        return TS_RESULT_NOT_ACTABLE;

    if (pos == ItemWearType::WEAR_BAG_SLOT)
    {
        if (GetMaxWeight() < GetFloatValue(PLAYER_FIELD_WEIGHT))
            return TS_RESULT_TOO_HEAVY;

        const auto &current_bag_base = m_anWear[absolute_pos]->GetItemBase();
        float current_bag_capacity{0};
        for (int i = 0; i < MAX_OPTION_NUMBER; ++i)
        {
            if (current_bag_base->opt_type[i] != static_cast<int32_t>(ITEM_EFFECT_PASSIVE::CARRY_WEIGHT))
                continue;

            current_bag_capacity += current_bag_base->opt_var[i][0];
        }

        if (GetMaxWeight() - current_bag_capacity < GetFloatValue(PLAYER_FIELD_WEIGHT))
            return TS_RESULT_TOO_HEAVY;
    }

    return putoffItem(absolute_pos);
}

uint16_t Unit::putoffItem(ItemWearType pos)
{
    ASSERT(pos < MAX_SPARE_ITEM_WEAR, "ItemWearType position is invald!!");

    m_anWear[pos]->SetWearInfo(WEAR_NONE);
    m_anWear[pos]->SetBindTarget(nullptr);

    if (IsPlayer())
        Messages::SendItemWearInfoMessage(this->As<Player>(), this, m_anWear[pos]);
    else if (IsSummon())
        Messages::SendItemWearInfoMessage(this->As<Summon>()->GetMaster(), this, m_anWear[pos]);

    m_anWear[pos] = nullptr;
    return TS_RESULT_SUCCESS;
}

ItemWearType Unit::GetAbsoluteWearPos(ItemWearType pos)
{
    ItemWearType result = pos;
    if (m_anWear[pos] == nullptr)
        result = WEAR_CANTWEAR;
    return result;
}

ItemClass Unit::GetWeaponClass()
{
    ItemClass result = CLASS_ETC;
    auto itemRight = GetWornItem(WEAR_RIGHTHAND);

    if (itemRight != nullptr)
    {
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        {
            Item *itemLeft = GetWornItem(WEAR_LEFTHAND);
            if (itemRight->m_pItemBase->iclass == CLASS_ONEHAND_SWORD && itemLeft->m_pItemBase->iclass == CLASS_ONEHAND_SWORD)
                return CLASS_DOUBLE_SWORD;
            if (itemRight->m_pItemBase->iclass == CLASS_DAGGER && itemLeft->m_pItemBase->iclass == CLASS_DAGGER)
                return CLASS_DOUBLE_DAGGER;
            if (itemRight->m_pItemBase->iclass == CLASS_ONEHAND_AXE && itemLeft->m_pItemBase->iclass == CLASS_ONEHAND_AXE)
                return CLASS_DOUBLE_AXE;
        }
        result = (ItemClass)itemRight->m_pItemBase->iclass;
    }
    return result;
}

bool Unit::IsWearShield()
{
    bool result{false};
    auto item = GetWornItem(WEAR_SHIELD);
    if (item != nullptr)
        result = item->m_pItemBase->iclass == CLASS_SHIELD;
    else
        result = false;
    return result;
}

float Unit::GetItemChance() const
{
    return m_Attribute.nItemChance;
}

uint16_t Unit::AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura, int nStateValue, std::string szStateValue)
{
    SetFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_UPDATE_STATE);
    auto stateInfo = sObjectMgr.GetStateInfo(code);

    if (stateInfo == nullptr)
        return TS_RESULT_NOT_EXIST;

    if (IsDead() && (stateInfo->state_time_type & AF_ERASE_ON_DEAD || stateInfo->state_time_type & AF_ERASE_ON_RESURRECT))
        return TS_RESULT_NOT_ACTABLE;

    if (IsMonster())
    {
        auto pMonster = this->As<Monster>();
        if ((stateInfo->state_time_type & AF_NOT_ACTABLE_TO_BOSS) != 0 && pMonster->IsBossMonster())
            return TS_RESULT_LIMIT_TARGET;

        if (code == SC_FEAR && (pMonster->IsDungeonConnector() || pMonster->IsAutoTrap()))
            return TS_RESULT_LIMIT_TARGET;
    }

    if (code == SC_SLEEP || code == SC_NIGHTMARE || code == SC_SEAL || code == SC_SHINE_WALL || code == SC_FEAR || code == SC_STUN || stateInfo->effect_type == SEF_TRANSFORMATION || (stateInfo->effect_type == SEF_MEZZ && (stateInfo->value[0] != 0 || stateInfo->value[1] != 0 || stateInfo->value[2] != 0 || stateInfo->value[3] != 0)))
    {
        if (IsUsingSkill())
            CancelSkill();

        Player *pThisPlayer{nullptr};
        if (IsPlayer())
            pThisPlayer = this->As<Player>();
        else if (IsSummon())
        {
            pThisPlayer = this->As<Summon>()->GetMaster();
            if (pThisPlayer != nullptr && pThisPlayer->GetRideObject() != this)
                pThisPlayer = nullptr;
        }

        if (pThisPlayer != nullptr && (pThisPlayer->IsRiding() || pThisPlayer->HasRidingState()))
        {
            auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(caster);
            if (pUnit != nullptr)
            {
                if (pThisPlayer && (pThisPlayer->IsRiding() || pThisPlayer->HasRidingState()))
                {
                    pThisPlayer->UnMount(UNMOUNT_FALL, pUnit);

                    if (IsDead() && (stateInfo->state_time_type & AF_ERASE_ON_DEAD || stateInfo->state_time_type & AF_ERASE_ON_RESURRECT))
                        return TS_RESULT_NOT_ACTABLE;
                }
            }
        }
    }

    if (stateInfo->effect_type == SEF_RIDING && IsHiding())
    {
        RemoveState(SC_HIDE, GameRule::MAX_STATE_LEVEL);
        RemoveState(SC_TRACE_OF_FUGITIVE, GameRule::MAX_STATE_LEVEL);
    }

    if (code == SC_FEAR)
        RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVING_BY_FEAR);

    int base_damage{0};
    auto pCaster = sMemoryPool.GetObjectInWorld<Unit>(caster);
    if (pCaster != nullptr)
    {
        switch (stateInfo->base_effect_id)
        {
        case BEF_PHYSICAL_STATE_DAMAGE:
        case BEF_PHYSICAL_IGNORE_DEFENCE_STATE_DAMAGE:
        case BEF_PHYSICAL_IGNORE_DEFENCE_PER_STATE_DAMAGE:
            base_damage = pCaster->GetAttackPointRight(static_cast<ElementalType>(stateInfo->elemental_type), true, true);
            break;
        case BEF_MAGICAL_STATE_DAMAGE:
        case BEF_MAGICAL_IGNORE_RESIST_STATE_DAMAGE:
        case BEF_HEAL_HP_BY_MAGIC:
        case BEF_HEAL_MP_BY_MAGIC:
            base_damage = pCaster->GetMagicPoint(static_cast<ElementalType>(stateInfo->elemental_type), false, true);
            break;
        default:
            break;
        }
    }

    bool bNotErasable = stateInfo->state_time_type & AF_AF_NOT_ERASABLE;
    std::vector<uint16_t> vDeleteStateUID{};
    bool bAlreadyExist{false};

    for (auto &it : m_vStateList)
    {
        bool bIsDuplicatedGroup{false};
        if (code == it->GetCode())
        {
            bAlreadyExist = true;
            bIsDuplicatedGroup = true;
        }
        else
        {
            for (int i = 0; i < 3; ++i)
            {
                if (it->IsDuplicatedGroup(stateInfo->duplicate_group[i]))
                {
                    bIsDuplicatedGroup = true;
                    break;
                }
            }
        }

        if (bIsDuplicatedGroup)
        {
            bool bNotErasableCur = it->GetTimeType() & AF_AF_NOT_ERASABLE;
            if (bNotErasable == bNotErasableCur)
            {
                if (it->GetLevel() > level || (it->GetLevel() == level && it->GetEndTime() > end_time))
                    return TS_RESULT_ALREADY_EXIST;

                if (code == it->GetCode())
                    continue;

                vDeleteStateUID.emplace_back(it->GetUID());
            }
            else if (bNotErasable)
            {
                vDeleteStateUID.emplace_back(it->GetUID());
                continue;
            }
            else
            {
                return TS_RESULT_ALREADY_EXIST;
            }
        }
    }

    for (auto dit = vDeleteStateUID.begin(); dit != vDeleteStateUID.end(); ++dit)
    {
        for (auto it = m_vStateList.begin(); it != m_vStateList.end(); ++it)
        {
            if ((*it)->GetUID() == (*dit))
            {
                auto state = (*it);

                onUpdateState((*it), true);
                m_vStateList.erase(it);
                CalculateStat();

                onAfterRemoveState(state);
                state->DeleteThis();
                break;
            }
        }
    }

    if (bAlreadyExist)
    {
        for (auto &it : m_vStateList)
        {
            if (code == it->GetCode())
            {
                it->AddState(type, caster, level, start_time, end_time, base_damage, bIsAura);
                CalculateStat();
                onUpdateState(it, false);

                onAfterAddState(it);
                break;
            }
        }

        return TS_RESULT_SUCCESS;
    }

    {
        auto ns = new State(type, code, ++m_nCurrentStateUID, caster, level, start_time, end_time, base_damage, bIsAura, nStateValue, szStateValue);
        sMemoryPool.AllocMiscHandle(ns);
        m_vStateList.emplace_back(ns);
    }

    CalculateStat();
    onUpdateState(m_vStateList.back(), false);

    if (IsMonster() && !IsMovable())
    {
        if (m_Attribute.nAttackRange < GameRule::MAX_ATTACK_RANGE * GameRule::ATTACK_RANGE_UNIT)
            m_Attribute.nAttackRange = GameRule::MAX_ATTACK_RANGE * GameRule::ATTACK_RANGE_UNIT;
    }

    onAfterAddState(m_vStateList.back());

    return TS_RESULT_SUCCESS;
}

void Unit::onAfterAddState(State *)
{
    procMoveSpeedChange();
}

void Unit::procMoveSpeedChange()
{
    if (bIsMoving)
        return;

    auto pos = GetCurrentPosition(sWorld.GetArTime());
    if (!IsMovable())
        sWorld.SetMove(this, pos, pos, 0, true, sWorld.GetArTime(), true);

    else if (GetMoveSpeed() != GetRealMoveSpeed())
    {
        const std::vector<ArMoveVector::MoveInfo> &vMoveVector(ends);
        std::vector<Position> vMovePos{};

        for (auto it = vMoveVector.begin(); it != vMoveVector.end(); ++it)
            vMovePos.emplace_back((*it).end);

        sWorld.SetMultipleMove(this, pos, vMovePos, GetMoveSpeed(), true, sWorld.GetArTime(), true);
    }
}

void Unit::onUpdateState(State *state, bool bIsExpire)
{
    Messages::BroadcastStateMessage(this, state, bIsExpire);
}

uint16_t Unit::onItemUseEffect(Unit *pCaster, Item *pItem, int type, float var1, float var2, const std::string &szParameter)
{
    uint16_t result{TS_RESULT_SUCCESS};

    switch (static_cast<ITEM_EFFECT_INSTANT>(type))
    {
    case ITEM_EFFECT_INSTANT::INC_HP:
    {
        int prev_hp = GetHealth();
        HealByItem((int)var1);
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, 0, false);
        break;
    }
    case ITEM_EFFECT_INSTANT::INC_MP:
    {
        int prev_mp = GetMana();
        MPHealByItem(static_cast<int32_t>(var1));
        Messages::BroadcastHPMPMessage(this, 0, GetMana() - prev_mp, false);
        break;
    }
    case ITEM_EFFECT_INSTANT::INC_HP_PERCENT:
    {
        int prev_hp = GetHealth();
        HealByItem(static_cast<int32_t>(var1 * GetMaxHealth()));
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, 0, false);
        break;
    }
    case ITEM_EFFECT_INSTANT::INC_MP_PERCENT:
    {
        int prev_mp = GetMana();
        MPHealByItem(static_cast<int32_t>(var1 * GetMaxMana()));
        Messages::BroadcastHPMPMessage(this, 0, GetMana() - prev_mp, false);
        break;
    }
    case ITEM_EFFECT_INSTANT::INC_STAMINA:
    {
        if (!IsPlayer())
        {
            result = TS_RESULT_ACCESS_DENIED;
            break;
        }
        this->As<Player>()->AddStamina(static_cast<int32_t>(var1));
        break;
    }
    case ITEM_EFFECT_INSTANT::RESURECTION:
    {
        result = TS_RESULT_ACCESS_DENIED;
        if (pItem->GetOwnerHandle() == GetHandle())
            break;

        if (IsAlive())
            break;

        if (!pCaster->IsAlly(this))
            break;

        int prev_hp = GetHealth();
        AddHealth(GetMaxHealth() * 0.2f);
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, 0, true);
        ClearRemovedStateByDeath();

        result = TS_RESULT_SUCCESS;
        break;
    }
    case ITEM_EFFECT_INSTANT::WARP:
    {
        // Nothing to do apparentely
        break;
    }
    case ITEM_EFFECT_INSTANT::SKILL:
    {
        auto target_handle = GetHandle();
        if (var1 == SKILL_RETURN_FEATHER || var1 == SKILL_RETURN_BACK_FEATHER)
            target_handle = pItem->GetHandle();
        result = pCaster->CastSkill((int)var1, (int)var2, target_handle, Position(), GetLayer(), true);
        break;
    }
    case ITEM_EFFECT_INSTANT::ADD_STATE:
    case ITEM_EFFECT_INSTANT::ADD_STATE_EX:
    {
        auto eCode = static_cast<StateCode>((static_cast<ITEM_EFFECT_INSTANT>(type) == ITEM_EFFECT_INSTANT::ADD_STATE) ? pItem->m_pItemBase->state_id : var1);

        if (IsPlayer())
        {
            auto pPlayer = this->As<Player>();
            const StateTemplate *pStateInfo = sObjectMgr.GetStateInfo(static_cast<int32_t>(eCode));

            if (pStateInfo == nullptr)
            {
                result = TS_RESULT_NOT_ACTABLE;
                break;
            }

            if (pStateInfo->effect_type == SEF_RIDING)
            {
                if (pPlayer->IsRiding() || pPlayer->HasRidingState() || pPlayer->IsInDungeon())
                {
                    result = TS_RESULT_ACCESS_DENIED;
                    break;
                }
            }

            if (static_cast<int32_t>(eCode) == SC_STAMINA_SAVE)
            {
                if (pPlayer->GetState(SC_STAMINA_SAVE))
                {
                    result = TS_RESULT_ALREADY_STAMINA_SAVED;
                    break;
                }
            }
        }

        auto t = sWorld.GetArTime();
        int nLevel = (static_cast<ITEM_EFFECT_INSTANT>(type) == ITEM_EFFECT_INSTANT::ADD_STATE) ? pItem->m_pItemBase->state_level : static_cast<int32_t>(var2);
        AddState(SG_NORMAL, eCode, pItem->m_Instance.OwnerHandle, nLevel, t, t + pItem->m_pItemBase->state_time * 100);
        break;
    }
    case ITEM_EFFECT_INSTANT::REMOVE_STATE:
    {
        RemoveState((StateCode)pItem->m_pItemBase->state_id, pItem->m_pItemBase->state_level);
        break;
    }
    case ITEM_EFFECT_INSTANT::TOGGLE_STATE:
    {
        auto pWornItem = GetWornItem(ItemWearType::WEAR_RIDE_ITEM);
        if (GetState((StateCode)pItem->m_pItemBase->state_id) != nullptr && pItem == pWornItem)
        {
            RemoveState((StateCode)pItem->m_pItemBase->state_id, pItem->m_pItemBase->state_level);
            return TS_RESULT_SUCCESS;
        }
        else
        {
            if (IsPlayer())
            {
                auto pPlayer = this->As<Player>();
                if (pPlayer->IsRiding() || pPlayer->HasRidingState() || pPlayer->IsInDungeon())
                {
                    const StateTemplate *pStateInfo = sObjectMgr.GetStateInfo(pItem->m_pItemBase->state_id);
                    if (pStateInfo->effect_type == SEF_RIDING)
                    {
                        result = TS_RESULT_ACCESS_DENIED;
                        break;
                    }
                }

                if (pWornItem != nullptr)
                {
                    if (pWornItem != pItem && (pPlayer->Putoff(WEAR_RIDE_ITEM) != TS_RESULT_SUCCESS || pPlayer->Puton(WEAR_RIDE_ITEM, pItem) != TS_RESULT_SUCCESS))
                    {
                        result = TS_RESULT_ACCESS_DENIED;
                        break;
                    }
                }
                else if (pPlayer->Puton(WEAR_RIDE_ITEM, pItem) != TS_RESULT_SUCCESS)
                {
                    result = TS_RESULT_ACCESS_DENIED;
                    break;
                }
            }

            auto t = sWorld.GetArTime();
            AddState(SG_NORMAL, static_cast<StateCode>(pItem->m_pItemBase->state_id), pItem->m_Instance.OwnerHandle, pItem->m_pItemBase->state_level, t, -1, true);
            break;
        }
        break;
    }
    case ITEM_EFFECT_INSTANT::GENERATE_ITEM:
    {
        if (!IsPlayer())
        {
            result = TS_RESULT_ACCESS_DENIED;
            break;
        }

        if (var1 == 0)
        {
            auto pPlayer = this->As<Player>();
            if (pPlayer->ChangeGold(pPlayer->GetGold() + static_cast<int64_t>(var2)) != TS_RESULT_SUCCESS)
            {
                result = TS_RESULT_TOO_MUCH_MONEY;
                break;
            }
        }
        else
        {
            auto pPlayer = this->As<Player>();
            int nGenCount = (var1 >= 0) ? 1 : var2;

            for (int i = 0; i < nGenCount; ++i)
            {
                int32_t nItemID = var1;
                auto nItemCount = static_cast<int64_t>(var2);

                while (nItemID < 0)
                {
                    GameContent::SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
                }

                if (nItemID != 0)
                {
                    auto pItem = Item::AllocItem(0, nItemID, nItemCount, GenerateCode::BY_ITEM);
                    auto pNewItem = this->As<Player>()->PushItem(pItem, pItem->GetCount(), false);

                    if (pNewItem != nullptr && IsPlayer())
                        Messages::SendResult(this->As<Player>(), NGemity::Packets::TS_CS_TAKE_ITEM, pNewItem->GetHandle(), 0);

                    if (pNewItem != pItem)
                        Item::PendFreeItem(pItem);
                }
            }
        }

        break;
    }
    default:
    {
        std::string error = NGemity::StringFormat("Unit::onItemUseEffect [{}]: Unknown type {} !", pItem->m_Instance.Code, type);
        NG_LOG_ERROR("entites.unit", "%s", error.c_str());
        Messages::SendChatMessage(30, "@SYSTEM", dynamic_cast<Player *>(pCaster), error);
        result = TS_RESULT_UNKNOWN;
        break;
    }
    }
    return result;
}

State *Unit::GetStateByEffectType(int effectType) const
{
    auto pos = std::find_if(m_vStateList.begin(),
                            m_vStateList.end(),
                            [effectType](const State *state) { return state->GetEffectType() == effectType; });

    return pos != m_vStateList.end() ? *pos : nullptr;
}

std::pair<float, int> Unit::GetHateMod(int nHateModType, bool bIsHarmful)
{
    float fAmpValue = 1.0f;
    int nIncValue = 0;

    for (auto &hm : m_vHateMod)
    {
        if (bIsHarmful)
        {
            if (!hm.bIsApplyToHarmful)
                continue;
        }
        else
        {
            if (!hm.bIsApplyToHelpful)
                continue;
        }

        if ((nHateModType == 1 && hm.bIsApplyToPhysicalSkill) || (nHateModType == 2 && hm.bIsApplyToMagicalSkill) || (nHateModType == 3 && hm.bIsApplyToPhysicalAttack))
        {
            fAmpValue += hm.fAmpValue;
            nIncValue += hm.nIncValue;
        }
    }
    return {fAmpValue, nIncValue};
}

bool Unit::ClearExpiredState(uint t)
{
    bool bRtn{false};
    bool bErase{false};
    bool bModified{false};

    if (t == 0)
        t = sWorld.GetArTime();

    for (auto it = m_vStateList.begin(); it != m_vStateList.end();)
    {
        bErase = false;
        bModified = false;

        if ((*it)->GetCode() == SC_HAVOC_BURST && GetInt32Value(UNIT_FIELD_HAVOC) < 1)
            bErase = true;

        if ((*it)->ClearExpiredState(t))
        {
            bRtn = true;
            if (!(*it)->IsValid(t))
                bErase = true;
            else
                onUpdateState((*it), false);
        }

        if (bErase)
        {
            if ((*it)->GetCode() == SC_ADD_ENERGY)
            {
                AddEnergy();
            }
            else if ((*it)->GetCode() == SC_HAVOC_BURST)
            {
                bRtn = true;
            }
            else if ((*it)->GetEffectType() == SEF_PROVOKE && IsMonster())
            {
                //
            }

            auto state = (*it);
            onUpdateState((*it), true);
            it = m_vStateList.erase(it);

            CalculateStat();
            onAfterRemoveState(state);

            continue;
        }
        ++it;
    }

    return bRtn;
}

int Unit::GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad)
{
    float v4{1};
    float v5 = m_Attribute.nAttackPointRight;

    // TODO: ElementalStateMod

    return (int)(v5 * v4);
}

DamageInfo Unit::DealMagicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealMagicalDamage(pFrom, (float)nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    ProcessAdditionalDamage(result, DT_ADDITIONAL_MAGICAL_DAMAGE, pFrom->m_vMagicalSkillAdditionalDamage, pFrom, nDamage, elemental_type);
    result.target_hp = GetHealth();
    return result;
}

DamageInfo Unit::DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealPhysicalDamage(pFrom, (float)nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);

    ProcessAdditionalDamage(result, DT_ADDITIONAL_DAMAGE, pFrom->m_vPhysicalSkillAdditionalDamage, pFrom, nDamage, elemental_type);

    result.target_hp = GetHealth();
    return result;
}

void Unit::ProcessAdditionalDamage(DamageInfo &damage_info, DamageType additionalDamage, std::vector<AdditionalDamageInfo> &vAdditionalDamage, Unit *pFrom, float nDamage, ElementalType elemental_type)
{
    if (damage_info.bMiss || damage_info.bPerfectBlock)
        return;

    for (auto &additional : vAdditionalDamage)
    {
        if ((additional.require_type == 99 || additional.require_type == elemental_type) && additional.ratio > urand(1, 100))
        {
            int damage{0};
            if (additional.nDamage != 0)
                damage = additional.nDamage;
            else
                damage = additional.fDamage * damage_info.nDamage;

            damage = DealDamage(pFrom, damage, additional.type, additionalDamage).nDamage;
            if (additional.type >= 0 && additional.type < ElementalType::TYPE_COUNT)
                damage_info.elemental_damage[additional.type] += damage;
            damage_info.nDamage += nDamage;
        }
    }
}

uint Unit::GetRemainCoolTime(int skill_id) const
{
    uint ct = sWorld.GetArTime();
    auto sk = GetSkill(skill_id);
    if (sk == nullptr || sk->m_nNextCoolTime < ct)
        return 0;
    return sk->m_nNextCoolTime - ct;
}

uint Unit::GetTotalCoolTime(int skill_id) const
{
    auto sk = GetSkill(skill_id);
    if (sk == nullptr)
        return 0;
    else
        return sk->GetSkillCoolTime();
}

float Unit::GetCoolTimeMod(ElementalType type, bool bPhysical, bool bBad) const
{
    if (bPhysical)
    {
        if (bBad)
            return m_BadPhysicalElementalSkillStateMod[type].fCooltime;

        return m_GoodPhysicalElementalSkillStateMod[type].fCooltime;
    }

    if (bBad)
        return m_BadMagicalElementalSkillStateMod[type].fCooltime;

    return m_GoodMagicalElementalSkillStateMod[type].fCooltime;
}

void Unit::SetJP(int jp)
{
    if (jp < 0)
        jp = 0;
    SetInt32Value(UNIT_FIELD_JOBPOINT, jp);
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

void Unit::SetEXP(int64_t exp)
{
    SetUInt64Value(UNIT_FIELD_EXP, exp);
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

bool Unit::IsWornByCode(int code) const
{
    for (auto &i : m_anWear)
    {
        if (i != nullptr && i->m_pItemBase != nullptr && i->m_pItemBase->id == code)
            return true;
    }
    return false;
}

void Unit::SetCurrentJLv(int jlv)
{
    SetInt32Value(UNIT_FIELD_JLV, jlv);
    onJobLevelUp();
}

int Unit::Heal(int hp)
{
    if (/*IsMagicImmune()*/ false)
    {
        return 0;
    }
    int heal = (int)GetFloatValue(UNIT_FIELD_HEAL_RATIO) * hp;
    AddHealth(heal);
    return heal;
}

int64_t Unit::GetBulletCount() const
{
    auto item = m_anWear[WEAR_SHIELD];
    if (item != nullptr && item->m_pItemBase->group == GROUP_BULLET)
    {
        return item->m_Instance.nCount;
    }
    else
    {
        return 0;
    }
}

int Unit::GetArmorClass() const
{
    return m_anWear[WEAR_ARMOR] != nullptr ? m_anWear[WEAR_ARMOR]->m_pItemBase->iclass : 0;
}

void Unit::procState(uint32_t t)
{
    struct ADD_STATE_INFO
    {
        StateCode code;
        int nLevel;
        uint32_t nEndTime;
        int nHate;
        Unit *pTarget;
    } add_state_info;

    std::vector<ADD_STATE_INFO> vAddStateInfo{};
    std::vector<State *> vGoodStateRemover{};
    std::vector<int32_t> vGoodStates{};
    bool bHasSequentialStateRemover{false};

    for (auto &pState : m_vStateList)
    {
        if (pState->GetEffectType() == SEF_REMOVE_GOOD_STATE)
        {
            uint32_t nFireTime = pState->GetLastProcessedTime() + pState->GetFireInterval();
            if (nFireTime >= t || nFireTime > pState->GetEndTime(SG_NORMAL))
                continue;

            if (pState->GetValue(5) == 0)
                bHasSequentialStateRemover = true;

            vGoodStateRemover.emplace_back(pState);
        }
        else if (!pState->IsHarmful() && pState->GetTimeType() & AF_ERASE_ON_DEAD)
        {
            vGoodStates.emplace_back(pState->GetUID());
        }

        if (pState->GetEffectType() == SEF_ADD_REGION_STATE)
        {
            auto curr = sWorld.GetArTime();
            if (pState->GetLastProcessedTime() + pState->GetValue(1) > curr)
                continue;

            auto code = static_cast<StateCode>(pState->GetValue(0));
            float fEffectLength = pState->GetValue(4) * GameRule::DEFAULT_UNIT_SIZE;
            auto nTargetType = static_cast<int32_t>(pState->GetValue(7));
            int32_t nLevel = pState->GetLevel();
            auto nHitRate = static_cast<int32_t>(pState->GetValue(8) + nLevel * pState->GetValue(9));
            uint32_t end_time = curr + (pState->GetValue(2) + nLevel * pState->GetValue(3)) * 100;

            auto pCaster = sMemoryPool.GetObjectInWorld<Unit>(pState->GetCaster(SG_NORMAL));
            if (pCaster == nullptr)
                continue;

            std::vector<Unit *> vTargetList{};
            Skill::EnumSkillTargetsAndCalcDamage(GetCurrentPosition(curr), GetLayer(), GetCurrentPosition(curr), true,
                                                 fEffectLength, -1, 0, 0, true, pCaster, static_cast<int32_t>(pState->GetValue(5)), static_cast<int32_t>(pState->GetValue(6)), vTargetList);

            for (auto &pTarget : vTargetList)
            {
                if (pTarget == nullptr || pTarget->IsDead())
                    continue;

                if (nTargetType == 3 && !IsEnemy(pTarget))
                    continue;
                else if (nTargetType == 2 && !IsAlly(pTarget))
                    continue;
                else if (nTargetType == 1 && (!IsAlly(pTarget) && !pTarget->IsNPC()))
                    continue;

                if (nHitRate < irand(0, 99))
                    continue;

                add_state_info.code = code;
                add_state_info.nLevel = nLevel;
                add_state_info.nEndTime = end_time;
                add_state_info.nHate = pState->GetValue(10) + nLevel * pState->GetValue(11);
                add_state_info.pTarget = pTarget;

                vAddStateInfo.emplace_back(add_state_info);
            }
        }
    }

    if (!vGoodStateRemover.empty())
    {
        if (bHasSequentialStateRemover)
        {
            std::sort(vGoodStateRemover.begin(), vGoodStateRemover.end(), [](State *lh, State *rh) -> bool { return lh->GetValue(5) == 0 && rh->GetValue(5) != 0; });
            std::reverse(vGoodStates.begin(), vGoodStates.end());
        }

        for (auto it = vGoodStateRemover.begin(); it != vGoodStateRemover.end(); ++it)
        {
            if (vGoodStates.empty() || ((*it)->GetValue(0) + (*it)->GetValue(1) * (*it)->GetLevel()) <= irand(0, 99))
                break;
            if ((*it)->GetValue(5) != 0)
                std::random_shuffle(vGoodStates.begin(), vGoodStates.end());

            int32_t nRemoveCount = (*it)->GetValue(3) + (*it)->GetValue(4) * (*it)->GetLevel();
            for (int i = 0; i < nRemoveCount; i++)
            {
                if (vGoodStates.empty())
                    break;
                RemoveState(vGoodStates.back());
                vGoodStates.pop_back();
            }
        }
    }

    if (!vAddStateInfo.empty())
    {
        for (auto &stateInfo : vAddStateInfo)
        {
            if (stateInfo.pTarget->IsDead() || !stateInfo.pTarget->IsInWorld())
                continue;

            stateInfo.pTarget->AddState(SG_NORMAL, stateInfo.code, GetHandle(), stateInfo.nLevel, t, stateInfo.nEndTime);
            if (stateInfo.pTarget->IsMonster())
                stateInfo.pTarget->As<Monster>()->AddHate(GetHandle(), stateInfo.nHate);
        }
    }
}

void Unit::procStateDamage(uint t)
{
    std::vector<StateDamage> vDamageList{};
    for (auto &st : m_vStateList)
    {
        if (IsPlayer() || IsSummon())
        {
            auto caster = sMemoryPool.GetObjectInWorld<Unit>(st->m_hCaster[0]);
            if (caster == nullptr)
            {
                if (st->m_nCode != StateCode::SC_GAIA_MEMBER_SHIP && st->m_nCode != StateCode::SC_NEMESIS && st->m_nCode != StateCode::SC_NEMESIS_FOR_AUTO && st->m_nCode != StateCode::SC_FALL_FROM_SUMMON && st->IsHarmful())
                {
                    st->AddState(StateType::SG_NORMAL, st->m_hCaster[0], (uint16_t)st->m_nLevel[0], st->m_nStartTime[0], (uint)(t - 1), st->m_nBaseDamage[0], false);
                    onUpdateState(st, false);
                    continue;
                }
            }
        }

        auto stateBase = sObjectMgr.GetStateInfo((int)st->m_nCode);
        if (stateBase == nullptr)
            continue;
        int nBaseEffectID = 0;
        auto nThisFireTime = (uint)(st->m_nLastProcessedTime + 100 * stateBase->fire_interval);
        if (nThisFireTime < t && nThisFireTime <= st->m_nEndTime[0])
        {
            nBaseEffectID = stateBase->base_effect_id;
            if (nBaseEffectID <= 0)
                continue;

            int nDamageHP = 0;
            int nDamageMP = 0;
            auto elem = (ElementalType)stateBase->elemental_type;

            switch (nBaseEffectID)
            {
            case 1:
            case 2:
            case 3:
            case 4:
            case 11:
                nDamageHP = (int)((stateBase->add_damage_per_skl * st->GetLevel()) + (st->m_nBaseDamage[0] * (stateBase->amplify_base + (stateBase->amplify_per_skl * st->GetLevel()))) + stateBase->add_damage_base);
                break;
            case 6:
                nDamageHP = (int)((st->GetValue(0) + (st->GetValue(1) * st->GetLevel())) * GetMaxHealth());
                nDamageMP = (int)((st->GetValue(2) + (st->GetValue(3) * st->GetLevel())) * GetMaxMana());
                break;
            case 12:
                nDamageMP = (int)((stateBase->add_damage_per_skl * st->GetLevel()) + (st->m_nBaseDamage[0] * (stateBase->amplify_base + (stateBase->amplify_per_skl * st->GetLevel()))) + stateBase->add_damage_base);
                break;
            case 21:
                nDamageHP = (stateBase->add_damage_base + (stateBase->add_damage_per_skl * st->GetLevel()));
                break;
            case 22:
                nDamageMP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st->GetLevel());
                break;
            case 24:
                nDamageHP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st->GetLevel());
                nDamageMP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st->GetLevel());
                break;
            case 25:
                nDamageHP = (int)((st->GetValue(0) + (st->GetValue(1) * st->GetLevel())) * GetMaxHealth());
                nDamageMP = (int)((st->GetValue(3) + (st->GetValue(4) * st->GetLevel())) * GetMaxMana());
                break;
            default:
                break;
            }

            if (nDamageHP != 0 || nDamageMP != 0)
            {
                st->m_nLastProcessedTime = nThisFireTime;
                StateDamage sd{st->m_hCaster[0], elem, nBaseEffectID, (int)st->m_nCode, st->GetLevel(), nDamageHP, nDamageMP,
                               nThisFireTime + (100 * stateBase->fire_interval) > st->m_nEndTime[0], st->m_nUID};
                vDamageList.emplace_back(sd);
            }
        }
    }

    for (auto &sd : vDamageList)
    {
        auto caster = sMemoryPool.GetObjectInWorld<Unit>(sd.caster);
        int nFlag = 0;
        Damage dmg{};
        if (sd.base_effect_id < 11)
        {
            if (caster == nullptr)
            {
                RemoveState(sd.uid);
                continue;
            }

            switch (sd.base_effect_id)
            {
            case 1:
                nFlag |= 10;
                dmg = DealPhysicalStateDamage(caster, sd.damage_hp, sd.elementalType, 0, 0, nFlag, nullptr, nullptr);
                break;
            case 2:
            case 6:
                nFlag |= 14;
                dmg = DealPhysicalStateDamage(caster, sd.damage_hp, sd.elementalType, 0, 0, nFlag, nullptr, nullptr);
                break;
            case 3:
                nFlag |= 8;
                dmg = DealMagicalStateDamage(caster, sd.damage_hp, sd.elementalType, 0, 0, nFlag, nullptr, nullptr);
                break;
            case 4:
                nFlag |= 12;
                dmg = DealMagicalStateDamage(caster, sd.damage_hp, sd.elementalType, 0, 0, nFlag, nullptr, nullptr);
                break;
            default:
                continue;
            }

            int total_amount = 0;
            for (auto &st : m_vStateList)
            {
                if (st->m_nUID == sd.uid)
                {
                    auto stateBase = sObjectMgr.GetStateInfo((int)st->m_nCode);
                    if (stateBase == nullptr)
                        continue;
                    st->m_nTotalDamage += dmg.nDamage;
                    total_amount = stateBase->state_id;
                    break;
                }
            }

            TS_SC_STATE_RESULT statePct{};
            statePct.caster_handle = sd.caster;
            statePct.target_handle = GetHandle();
            statePct.code = sd.code;
            statePct.level = sd.level;
            statePct.result_type = SRT_Damage;
            statePct.value = dmg.nDamage;
            statePct.target_value = GetHealth();
            statePct.final = (sd.final ? 1 : 0);
            statePct.total_amount = total_amount;

            sWorld.Broadcast((uint)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                             (uint)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                             GetLayer(),
                             statePct);
        }
        else
        {
            int nHealHP = 0;
            int nHealMP = 0;

            switch (sd.base_effect_id)
            {
            case 11:
                nHealHP = Heal(sd.damage_hp);
                break;
            case 12:
                nHealMP = MPHeal(sd.damage_mp);
                break;
            case 21:
                nHealHP = HealByItem(sd.damage_hp);
                break;
            case 22:
                nHealMP = MPHealByItem(sd.damage_mp);
                break;
            case 24:
            case 25:
                nHealHP = HealByItem(sd.damage_hp);
                nHealMP = MPHealByItem(sd.damage_mp);
                break;
            default:
                continue;
            }

            int total_amount = 0;
            for (auto &st : m_vStateList)
            {
                if (st->m_nUID == sd.uid)
                {
                    int ad = nHealHP;
                    if (ad == 0)
                        ad = nHealMP;
                    if (ad != 0)
                    {
                        st->m_nTotalDamage += ad;
                        total_amount = st->m_nTotalDamage;
                    }
                    break;
                }
            }

            int df = 0;
            if (nHealHP != 0)
            {

                TS_SC_STATE_RESULT statePct{};
                statePct.caster_handle = sd.caster;
                statePct.target_handle = GetHandle();
                statePct.code = sd.code;
                statePct.level = sd.level;
                statePct.result_type = SRT_Heal;
                statePct.value = nHealHP;
                statePct.target_value = GetHealth();
                statePct.final = (sd.final ? 1 : 0);
                statePct.total_amount = total_amount;

                sWorld.Broadcast((uint)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                 (uint)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                 GetLayer(),
                                 statePct);
            }

            if (nHealMP != 0)
            {
                df = df != 0 ? -1 : 0;
                df = total_amount & df;

                TS_SC_STATE_RESULT statePct{};
                statePct.caster_handle = sd.caster;
                statePct.target_handle = GetHandle();
                statePct.code = sd.code;
                statePct.level = sd.level;
                statePct.result_type = SRT_MagicHeal;
                statePct.value = nHealMP;
                statePct.target_value = GetMana();
                statePct.final = (sd.final ? 1 : 0);
                statePct.total_amount = df;

                sWorld.Broadcast((uint)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                 (uint)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                                 GetLayer(),
                                 statePct);
            }
        }
    }
}

Damage Unit::DealPhysicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_STATE_PHYSICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealMagicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_STATE_MAGICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

void Unit::RemoveState(StateCode code, int state_level)
{
    auto state = std::find_if(m_vStateList.begin(), m_vStateList.end(), [code, state_level](State *s) { return s->m_nCode == code && s->GetLevel() <= state_level; });
    if (state != m_vStateList.end())
    {
        onUpdateState(*state, true);
        m_vStateList.erase(state);
        CalculateStat();
        onAfterAddState(*state); // @todo: onafterremovestate
        (*state)->DeleteThis();
    }
}

void Unit::RemoveState(int uid)
{
    auto state = std::find_if(m_vStateList.begin(),
                              m_vStateList.end(),
                              [uid](const State *s) { return s->m_nUID == uid; });

    if (state != m_vStateList.end())
    {
        onUpdateState(*state, true);
        m_vStateList.erase(state);
        CalculateStat();
        onAfterAddState(*state);
        (*state)->DeleteThis();
    }
}

void Unit::RemoveGoodState(int state_level)
{
    for (auto &s : m_vStateList)
    {
        if (!s->IsHarmful() && s->GetTimeType() != 0)
            RemoveState(s->m_nCode, state_level);
    }
}

int Unit::MPHeal(int mp)
{
    return MPHealByItem(mp);
}

int Unit::HealByItem(int hp)
{
    auto result = (int)((GetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_ITEM) * hp) + GetFloatValue(UNIT_FIELD_ADDITIONAL_HEAL));
    AddHealth(result);
    return result;
}

int Unit::MPHealByItem(int mp)
{
    auto result = (int)(GetFloatValue(UNIT_FIELD_MP_HEAL_RATIO) * mp + GetFloatValue(UNIT_FIELD_ADDITIONAL_MP_HEAL));
    AddMana(result);
    return result;
}

bool Unit::OnCompleteSkill()
{
    if (m_castingSkill != nullptr)
    {
        m_castingSkill = nullptr;
        return true;
    }
    return false;
}

float Unit::GetManaCostRatio(ElementalType type, bool bPhysical, bool bBad)
{
    return 1.0f;
}

void Unit::BindSkillCard(Item *pItem)
{
    Skill *pSkill = GetSkill(pItem->m_pItemBase->skill_id);
    if (pSkill != nullptr)
    {
        pSkill->m_nEnhance = (uint)pItem->m_Instance.nEnhance;
        pItem->SetBindTarget(this);
        Messages::SendSkillCardInfo(dynamic_cast<Player *>(this), pItem);
    }
}

void Unit::UnBindSkillCard(Item *pItem)
{
    Skill *pSkill = GetSkill(pItem->m_pItemBase->skill_id);
    if (pSkill != nullptr)
    {
        pSkill->m_nEnhance = 0;
        pItem->SetBindTarget(nullptr);
        Messages::SendSkillCardInfo(dynamic_cast<Player *>(this), pItem);
    }
}

bool Unit::IsEnemy(const Unit *pTarget, bool bIncludeHiding)
{
    return pTarget != nullptr && pTarget->IsInWorld() && (bIncludeHiding || IsVisible(pTarget)) && !IsAlly(pTarget);
}

bool Unit::IsAlly(const Unit * /*pTarget*/)
{
    return false;
}

bool Unit::IsVisible(const Unit *pTarget)
{
    return !pTarget->HasFlag(UNIT_FIELD_STATUS, STATUS_HIDING);
}

bool Unit::IsActiveAura(Skill *pSkill) const
{
    for (const auto &aura : m_vAura)
    {
        if (aura.first == pSkill)
            return true;
    }
    return false;
}

bool Unit::TurnOnAura(Skill *pSkill)
{
    if (pSkill == nullptr)
        return false;

    int nToggleGroup = pSkill->GetSkillBase()->GetToggleGroup();
    if (nToggleGroup > Skill::MAX_TOGGLE_GROUP || nToggleGroup < 0)
    {
        ASSERT(0, "toggle group invalid!");
        return false;
    }

    TS_SC_AURA msg{};
    bool bExistSameGroup{false};
    for (auto &it : m_vAura)
    {
        if (it.first->GetSkillBase()->GetToggleGroup() != 0 && it.first->GetSkillBase()->GetToggleGroup() == nToggleGroup)
        {
            msg.caster = GetHandle();
            msg.skill_id = it.first->GetSkillId();

            msg.status = false;

            if (IsPlayer())
                this->As<Player>()->SendPacket(msg);
            else if (IsSummon())
                this->As<Summon>()->GetMaster()->SendPacket(msg);

            it.first = pSkill;
            it.second = pSkill->GetRequestedSkillLevel();
            bExistSameGroup = true;
            break;
        }
    }

    if (!bExistSameGroup)
        m_vAura.emplace_back(std::pair<Skill *, int32_t>(pSkill, static_cast<int32_t>(pSkill->GetRequestedSkillLevel())));

    msg.caster = GetHandle();
    msg.skill_id = pSkill->GetSkillId();

    msg.status = true;

    if (IsPlayer())
        this->As<Player>()->SendPacket(msg);
    else if (IsSummon())
        this->As<Summon>()->GetMaster()->SendPacket(msg);

    return true;
}

bool Unit::TurnOffAura(Skill *pSkill)
{
    if (pSkill == nullptr)
        return false;

    for (auto it = m_vAura.begin(); it != m_vAura.end(); ++it)
    {
        if ((*it).first == pSkill)
        {
            TS_SC_AURA msg{};
            msg.caster = GetHandle();
            msg.skill_id = (*it).first->GetSkillId();

            msg.status = false;

            if (IsPlayer())
                this->As<Player>()->SendPacket(msg);
            else if (IsSummon())
                this->As<Summon>()->GetMaster()->SendPacket(msg);

            m_vAura.erase(it);
            return true;
        }
    }
    return false;
}

void Unit::ToggleAura(Skill *pSkill)
{
    if (!TurnOffAura(pSkill))
        TurnOnAura(pSkill);
}

bool Unit::IsActable()
{
    return (!IsDead() && !IsFeared() && (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE) || HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE) || HasFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE) || HasFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE) || HasFlag(UNIT_FIELD_STATUS, STATUS_ITEM_USABLE)));
}

bool Unit::IsAttackable()
{
    if (!IsActable() || IsSitDown() || IsUsingSkill())
        return false;
    return HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);
}

bool Unit::IsMovable()
{
    if (IsDead() || IsSitDown())
        return false;
    if (GetNextMovableTime() > sWorld.GetArTime())
        return false;
    if (IsUsingSkill())
        return false;

    return HasFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
}

bool Unit::IsSkillCastable()
{
    if (!IsActable() || GetState(SC_MUTE))
        return false;
    if (IsSitDown())
        return false;
    if (IsUsingSkill())
        return false;
    if (IsAttacking())
        return false;
    return HasFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);
}

bool Unit::IsMagicCastable()
{
    if (!IsActable() || GetState(SC_MUTE))
        return false;
    if (IsSitDown())
        return false;
    if (IsUsingSkill())
        return false;
    if (IsAttacking())
        return false;
    return HasFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);
}

int Unit::GetMoveSpeed()
{
    return (int)m_Attribute.nMoveSpeed;
}

State *Unit::GetState(StateCode code)
{
    auto var = std::find_if(m_vStateList.begin(), m_vStateList.end(), [&code](State *s) { return s->m_nCode == code; });
    if (var != m_vStateList.end())
        return *var;
    return nullptr;
}

void Unit::AddEnergy()
{
    if (GetInt32Value(UNIT_FIELD_ENERGY) >= GetInt32Value(UNIT_FIELD_MAX_ENERGY))
        return;

    //@todo: Energy Upkeep time?
    auto tmp = GetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS) + GetInt32Value(UNIT_FIELD_ENERGY);
    if (tmp >= 10)
        tmp -= 10;
    SetInt32Value(UNIT_FIELD_ARRAY_ENERGY + tmp, sWorld.GetArTime());
    SetInt32Value(UNIT_FIELD_ENERGY, GetInt32Value(UNIT_FIELD_ENERGY) + 1);
    onEnergyChange();
}

void Unit::RemoveEnergy(int nEnergy)
{
    auto energy = (GetInt32Value(UNIT_FIELD_ENERGY) < nEnergy) ? GetInt32Value(UNIT_FIELD_ENERGY) : nEnergy;

    if (energy >= 1)
    {
        SetInt32Value(UNIT_FIELD_ENERGY, GetInt32Value(UNIT_FIELD_ENERGY) - energy);
        SetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS, GetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS) + energy);

        if (GetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS) >= 10)
            SetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS, GetInt32Value(UNIT_FIELD_ENERGY_STARTING_POS) - 10);
        onEnergyChange();
    }
}

int Unit::GetMagicPoint(ElementalType type, bool bPhysical, bool bBad)
{
    if (bPhysical)
    {
        if (bBad)
            return m_Attribute.nMagicPoint * m_BadPhysicalElementalSkillStateMod[type].fMagicalDamage;

        return m_Attribute.nMagicPoint * m_GoodPhysicalElementalSkillStateMod[type].fMagicalDamage;
    }

    if (bBad)
        return m_Attribute.nMagicPoint * m_BadMagicalElementalSkillStateMod[type].fMagicalDamage;

    return m_Attribute.nMagicPoint * m_GoodMagicalElementalSkillStateMod[type].fMagicalDamage;
}

bool Unit::onProcAura(Skill *pSkill, int nRequestedLevel)
{
    pSkill->SetRequestedSkillLevel(nRequestedLevel);
    bool res = pSkill->ProcAura();
    pSkill->SetRequestedSkillLevel(0);
    return res;
}

float Unit::GetMagicalHateMod(ElementalType type, bool bPhysical, bool bBad)
{
    float result = 0;

    if (bPhysical)
    {
        if (bBad)
            result = m_BadPhysicalElementalSkillStateMod[(int)type].fHate;
        else
            result = m_GoodPhysicalElementalSkillStateMod[(int)type].fHate;
    }
    else
    {
        if (bBad)
            result = m_BadMagicalElementalSkillStateMod[(int)type].fHate;
        else
            result = m_GoodMagicalElementalSkillStateMod[(int)type].fHate;
    }
    return result;
}

void Unit::ProcessAddHPMPOnCritical()
{
    if (m_vAddHPMPOnCritical.empty())
        return;

    for (const auto &it : m_vAddHPMPOnCritical)
    {
        if (irand(0, 99) < it.nActivationRate)
        {
            AddHealth(it.nAddHP);
            AddMana(it.nAddMP);
        }
    }
}

void Unit::AddStateByAttack(Unit *pTarget, bool bIsAttacking, bool bIsSkill, bool bIsPhysicalSkill, bool bIsHarmful)
{
    std::vector<_ADD_STATE_TAG> vState{};

    if (bIsAttacking)
    {
        if (!bIsSkill)
            vState = m_vStateByNormalAttack;
        else
        {
            if (bIsPhysicalSkill)
            {
                if (bIsHarmful)
                {
                    vState = m_vStateByHarmfulPhysicalSkill;
                }
                else
                {
                    vState = m_vStateByHelpfulPhysicalSkill;
                }
            }
            else
            {
                if (bIsHarmful)
                {
                    vState = m_vStateByHarmfulMagicalSkill;
                }
                else
                {
                    vState = m_vStateByHelpfulMagicalSkill;
                }
            }
        }
    }
    else
    {
        if (!bIsSkill)
            vState = m_vStateByBeingNormalAttacked;
        else
        {
            if (bIsPhysicalSkill)
            {
                if (bIsHarmful)
                {
                    vState = m_vStateByBeingHarmfulPhysicalSkilled;
                }
                else
                {
                    vState = m_vStateByBeingHelpfulPhysicalSkilled;
                }
            }
            else
            {
                if (bIsHarmful)
                {
                    vState = m_vStateByBeingHarmfulMagicalSkilled;
                }
                else
                {
                    vState = m_vStateByBeingHelpfulMagicalSkilled;
                }
            }
        }
    }

    for (const auto &itState : vState)
    {
        Unit *pStateTarget{nullptr};

        if (itState.target == ATTACKER)
        {
            if (bIsAttacking)
            {
                pStateTarget = this;
            }
            else
            {
                pStateTarget = pTarget;
            }
        }
        else
        {
            if (bIsAttacking)
            {
                pStateTarget = pTarget;
            }
            else
            {
                pStateTarget = this;
            }
        }

        if (irand(1, 100) % 100 >= itState.percentage)
            continue;
        if (itState.min_hp != 0 && itState.min_hp > (float)this->GetHealth() / this->GetMaxHealth() * 100)
            continue;
        if (itState.max_hp != 0 && itState.max_hp < (float)this->GetHealth() / this->GetMaxHealth() * 100)
            continue;
        if (itState.target_min_hp != 0 && itState.target_min_hp > (float)pStateTarget->GetHealth() / pStateTarget->GetMaxHealth() * 100)
            continue;
        if (itState.target_max_hp != 0 && itState.target_max_hp < (float)pStateTarget->GetHealth() / pStateTarget->GetMaxHealth() * 100)
            continue;
        if (itState.cost_mp > this->GetMana())
            continue;
        AddMana(-itState.cost_mp);

        uint32_t t = sWorld.GetArTime();

        pStateTarget->AddState(SG_NORMAL, itState.code, this->GetHandle(), itState.level, t, t + itState.duration);
    }
}

void Unit::RemoveStatesOnDamage()
{
    for (auto it = m_vStateList.begin(); it != m_vStateList.end();)
    {
        if (!((*it)->GetTimeType() & AF_ERASE_ON_DAMAGED) /*|| (*it).IsByEvent()*/)
        {
            ++it;
            continue;
        }
        auto state = (*it);
        onUpdateState((*it), true);
        it = m_vStateList.erase(it);
        CalculateStat();

        //onAfterRemoveState(state);
    }
}

int Unit::GetCriticalDamage(int damage, float critical_amp, int critical_bonus)
{
    if (irand(0, 99) <= (critical_amp * GetCritical() + critical_bonus))
        return (damage * (GetCriticalPower() / 100.0f));
    return 0;
}

void Unit::onAfterRemoveState(State *pState)
{
    procMoveSpeedChange();
}

bool Unit::AddToEnemyList(uint32_t handle)
{
    if (std::find(m_vEnemyList.begin(), m_vEnemyList.end(), handle) != m_vEnemyList.end())
    {
        m_vEnemyList.emplace_back(handle);
        return true;
    }
    return false;
}

bool Unit::RemoveFromEnemyList(uint32_t handle)
{
    for (auto it = m_vEnemyList.begin(); it != m_vEnemyList.end(); ++it)
    {
        if ((*it) == handle)
        {
            m_vEnemyList.erase(it);
            return true;
        }
    }
    return false;
}

void Unit::EnumPassiveSkill(SkillFunctor &fn)
{
    for (const auto &pSkill : m_vPassiveSkillList)
        fn.onSkill(pSkill);
}

uint8_t Unit::GetRealRidingSpeed()
{
    int nMoveSpeed = GetMoveSpeed() / 7;
    if (nMoveSpeed > UINT8_MAX)
        nMoveSpeed = UINT8_MAX;
    return static_cast<uint8_t>(nMoveSpeed);
}

void Unit::removeStateByDead()
{
    ClearRemovedStateByDeath();

    std::vector<State *> removedStates{};
    RemoveStateIf([](const State *p) { return p->GetTimeType() & AF_ERASE_ON_DEAD; }, &removedStates, true);

    if (IsPlayer())
    {
        for (auto &pState : removedStates)
        {
            if (!pState->IsHarmful())
            {
                State::DB_InsertState(this, pState);
                m_vStateListRemovedByDeath.emplace_back(pState);
            }
            else
            {
                //pState->Expire(this);
                pState->DeleteThis();
            }
        }
    }
}

template <typename COMPARER>
void Unit::RemoveStateIf(COMPARER comparer, std::vector<State *> *result, bool bByDead)
{
    std::vector<State *> removedStates{};
    std::vector<State *>::iterator it, trail;
    if (result != nullptr)
        removedStates.swap(*result);

    for (it = m_vStateList.begin(), trail = it; it != m_vStateList.end(); ++it)
    {
        if (comparer(*it))
        {
            removedStates.emplace_back(*it);
        }
        else
        {
            if (trail != it)
            {
                *trail = *it;
            }
            ++trail;
        }
    }

    m_vStateList.resize(trail - m_vStateList.begin());

    for (it = removedStates.begin(); it != removedStates.end(); ++it)
    {
        onUpdateState((*it), true);
        onAfterRemoveState(*it);
    }

    if (!removedStates.empty())
        CalculateStat();

    if (result != nullptr)
        removedStates.swap(*result);
}

void Unit::RestoreRemovedStateByDeath()
{
    auto t = sWorld.GetArTime();
    for (auto it = m_vStateListRemovedByDeath.begin(); it != m_vStateListRemovedByDeath.end();)
    {
        const State *state = (*it);
        if (GetState(state->GetCode()) || (state->GetTimeType() & AF_ERASE_ON_DEAD))
        {
            ++it;
            continue;
        }

        if (state->GetEndTime() == UINT32_MAX)
        {
            AddState(StateType::SG_NORMAL, state->GetCode(), state->m_hCaster[0], state->GetLevel(), state->GetStartTime(), UINT32_MAX, state->IsAura(), state->m_nStateValue, state->m_szStateValue);
        }
        else
        {
            auto time_difference = sWorld.GetArTime() - GetUInt32Value(UNIT_FIELD_DEAD_TIME);
            AddState(StateType::SG_NORMAL, state->GetCode(), state->m_hCaster[0], state->GetLevel(), state->GetStartTime(), time_difference, state->IsAura(), state->m_nStateValue, state->m_szStateValue);
        }
        it = m_vStateListRemovedByDeath.erase(it);
    }
    ClearRemovedStateByDeath();
}

void Unit::ClearRemovedStateByDeath()
{
    for (auto &it : m_vStateListRemovedByDeath)
        it->DeleteThis();
    m_vStateListRemovedByDeath.clear();
}

bool Unit::Resurrect(_CHARACTER_RESURRECTION_TYPE eResurrectType, int nIncHP, int nIncMP, int64_t nRecoveryEXP, bool bIsRestoreRemovedStateByDead)
{
    if (!IsDead() || (!IsSummon() && !IsPlayer()))
        return false;

    AddHealth(std::max(nIncHP, 1));
    AddMana(nIncMP);
    SetEXP(GetEXP() + nRecoveryEXP);

    if (bIsRestoreRemovedStateByDead)
        RestoreRemovedStateByDeath();
    else
        ClearRemovedStateByDeath();

    if (IsPlayer())
        this->As<Player>()->Save(true);
    else if (IsSummon())
        Summon::DB_UpdateSummon(this->As<Summon>()->GetMaster(), this->As<Summon>());

    Messages::BroadcastHPMPMessage(this, nIncHP, nIncMP, true);
    return true;
}

void Unit::RemoveAllAura()
{
    if (m_vAura.empty())
        return;

    for (auto &it : m_vAura)
    {
        TS_SC_AURA msg{};

        msg.caster = GetHandle();
        msg.skill_id = it.first->GetSkillId();
        msg.status = false;

        if (IsPlayer())
            this->As<Player>()->SendPacket(msg);
        else if (IsSummon())
            this->As<Summon>()->GetMaster()->SendPacket(msg);
    }
    m_vAura.clear();
}

void Unit::RemoveAllHate()
{
    for (auto &it : m_vEnemyList)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Monster>(it);
        if (pUnit != nullptr && pUnit->IsMonster())
        {
            auto pMonster = pUnit->As<Monster>();
            pMonster->RemoveHate(GetHandle(), pMonster->GetHate(GetHandle()));
        }
    }

    m_vEnemyList.clear();
}

bool Unit::ResurrectByState()
{
    if (!IsDead() || (!IsSummon() && !IsPlayer()))
        return false;

    State *pResurrectState{nullptr};
    for (auto &state : m_vStateList)
    {
        if (state->GetEffectType() == SEF_RESURRECTION)
        {
            if (pResurrectState == nullptr || pResurrectState->GetLevel() < state->GetLevel())
                pResurrectState = state;
        }
    }

    if (pResurrectState == nullptr)
        return false;

    auto nIncHP = (pResurrectState->GetValue(0) + pResurrectState->GetValue(1) * pResurrectState->GetLevel()) * GetMaxHealth();
    auto nIncMP = (pResurrectState->GetValue(2) + pResurrectState->GetValue(3) * pResurrectState->GetLevel()) * GetMaxMana();
    // @todo: Recovery XP

    AddHealth(nIncHP);
    AddMana(nIncMP);
    ClearRemovedStateByDeath();

    if (IsPlayer())
    {
        auto pPlayer = this->As<Player>();
        pPlayer->Save(true);

        if (pPlayer->HasFlag(UNIT_FIELD_STATUS, STATUS_DEAD))
            pPlayer->RemoveFlag64(UNIT_FIELD_STATUS, STATUS_DEAD);
    }

    RemoveState(pResurrectState->GetCode(), GameRule::MAX_STATE_LEVEL);

    Messages::BroadcastHPMPMessage(this, nIncHP, nIncMP);
    return true;
}

void Unit::PrepareRemoveExhaustiveSkillStateMod(bool bPhysical, bool bHarmful, int nElementalType, uint32_t nOriginalCastingDelay)
{
    if (bPhysical)
    {
        if (bHarmful)
        {
            m_BadPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCodeForDelete = m_BadPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode;
        }
        else
        {
            m_GoodPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCodeForDelete = m_GoodPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode;
        }
    }
    else
    {
        if (bHarmful)
        {
            m_BadMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCodeForDelete = m_BadMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode;
        }
        else
        {
            m_GoodMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCodeForDelete = m_GoodMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode;
        }
    }
}

void Unit::RemoveExhaustiveSkillStateMod(bool bPhysical, bool bHarmful, int nElementalType, uint32_t nOriginalCastingDelay)
{
    if (bPhysical)
    {
        if (bHarmful)
        {
            removeExhaustiveSkillStateMod(m_BadPhysicalElementalSkillStateMod[nElementalType], nOriginalCastingDelay);
        }
        else
        {
            removeExhaustiveSkillStateMod(m_GoodPhysicalElementalSkillStateMod[nElementalType], nOriginalCastingDelay);
        }
    }
    else
    {
        if (bHarmful)
        {
            removeExhaustiveSkillStateMod(m_BadMagicalElementalSkillStateMod[nElementalType], nOriginalCastingDelay);
        }
        else
        {
            removeExhaustiveSkillStateMod(m_GoodMagicalElementalSkillStateMod[nElementalType], nOriginalCastingDelay);
        }
    }

    CalculateStat();
}

void Unit::removeExhaustiveSkillStateMod(ElementalSkillStateMod &skillStateMod, uint32_t nOriginalCastingDelay)
{
    std::vector<StateCode> vEraseList = skillStateMod.vExhaustiveStateCodeForDelete;

    for (auto it = vEraseList.begin(); it != vEraseList.end(); ++it)
    {
        auto pState = GetState(*it);
        if (pState == nullptr)
            continue;

        if (pState->GetEffectType() == SEF_ADD_PARAMETER_ON_SKILL && pState->GetValue(11) && pState->GetValue(11) * 100 < nOriginalCastingDelay)
            continue;

        RemoveState(*it, pState->GetLevel());
    }
}
