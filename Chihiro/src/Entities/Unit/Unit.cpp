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
#include <limits>
#include "Unit.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Messages.h"
#include "ClientPackets.h"
#include "Skill.h"
#include "MemPool.h"
#include "RegionContainer.h"
#include "GameContent.h"
#include "Log.h"
#include "Player.h"
#include "NPC.h"
#include "WorldSession.h"
#include "GameRule.h"

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
        Skill::DB_InsertSkill(this, skill->m_nSkillUID, skill->m_nSkillID, skill->m_nSkillLevel, skill->m_nNextCoolTime < ct ? 0 : skill->m_nNextCoolTime - ct);
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

void Unit::Update(uint32 p_time)
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

void Unit::SetMove(Position _to, uint8 _speed, uint _start_time)
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
            auto pSkill = GetSkill((*it).second);
            if (pSkill != nullptr && pSkill->GetAuraRefreshTime() + 500 <= ct)
            {
                if (!onProcAura(pSkill, (*it).first))
                {
                    TS_SC_AURA auraMsg{};
                    auraMsg.caster = GetHandle();
                    auraMsg.skill_id = pSkill->GetSkillId();
                    auraMsg.status = false;

                    if (IsPlayer())
                        this->As<Player>()->SendPacket(auraMsg);
                    else if (IsSummon())
                        this->As<Summon>()->GetMaster()->SendPacket(auraMsg);

                    it = m_vAura.erase(it);
                    continue;
                }
                pSkill->SetAuraRefreshTime(ct);
            }
            ++it;
        }
    }

    if (IsInWorld())
    {
        if (!m_vStateList.empty() && GetUInt32Value(UNIT_LAST_STATE_PROC_TIME) + 100 < ct)
        {
            procStateDamage(ct);
            //procState(ct);
            if (ClearExpiredState(ct))
            {
                CalculateStat();
                SetUInt32Value(UNIT_LAST_STATE_PROC_TIME, ct);
            }
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
    for (auto s : m_vSkillList)
    {
        if (s->m_nSkillID == skill_id)
            return s;
    }
    return nullptr;
}

Skill *Unit::RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID)
{
    Skill *pSkill = nullptr;
    int nNeedJP = sObjectMgr.GetNeedJpForSkillLevelUp(skill_id, skill_level, nJobID);
    if (GetJP() >= nNeedJP)
    {
        SetJP(GetJP() - nNeedJP);
        if (GetJP() < 0)
            SetJP(0);

        onExpChange();

        int64 nSkillUID = 0;
        int nPrevLevel = GetBaseSkillLevel(skill_id);
        if (nPrevLevel == 0)
        {
            nSkillUID = sWorld.GetSkillIndex();
            pSkill = new Skill{this, nSkillUID, skill_id};
            m_vSkillList.emplace_back(pSkill);
        }
        else
        {
            pSkill = GetSkill(skill_id);
            nSkillUID = pSkill == nullptr ? 0 : pSkill->m_nSkillUID;
        }
        if (pSkill != nullptr)
        {
            pSkill->m_nSkillLevel = skill_level;

            onRegisterSkill(nSkillUID, skill_id, nPrevLevel, skill_level);
        }
    }
    return pSkill;
}

int Unit::GetCurrentSkillLevel(int skill_id) const
{
    auto s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel + 0;
}

void Unit::SetSkill(int skill_uid, int skill_id, int skill_level, int remain_cool_time)
{
    if (!sObjectMgr.GetSkillBase(skill_id))
        return;

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

    pSkill->m_nSkillID = skill_id;
    pSkill->m_nSkillUID = skill_uid;
    pSkill->m_nSkillLevel = skill_level;
    if (remain_cool_time != 0)
        pSkill->m_nNextCoolTime = remain_cool_time + sWorld.GetArTime();
}

int Unit::CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    auto player = dynamic_cast<Player *>(this);
    Summon *summon{nullptr};
    Unit *pSkillTarget{nullptr};
    auto tpos = pos.GetPosition();

    auto pSkill = GetSkill(nSkillID);
    if (pSkill == nullptr || pSkill->m_SkillBase == nullptr || m_castingSkill != nullptr /*|| using storage*/)
        return TS_RESULT_NOT_ACTABLE;

    //auto pSkillTarget = sMemoryPool.getPtrFromId(target_handle);
    auto obj = sMemoryPool.GetObjectInWorld<WorldObject>(target_handle);
    if (obj != nullptr && (!obj->IsItem() && !obj->IsFieldProp()))
    {
        pSkillTarget = dynamic_cast<Unit *>(obj);
    }

    switch (pSkill->m_SkillBase->target)
    {
    case TARGET_TYPE::TARGET_MASTER:
        if (!IsSummon())
            return TS_RESULT_NOT_ACTABLE;
        summon = dynamic_cast<Summon *>(this);
        if (summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
            return TS_RESULT_NOT_ACTABLE;
        break;
    case TARGET_TYPE::TARGET_SELF_WITH_MASTER:
        if (!IsSummon())
            return TS_RESULT_NOT_ACTABLE;
        summon = this->As<Summon>();
        if (pSkillTarget->GetHandle() != GetHandle() && summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
            return TS_RESULT_NOT_ACTABLE;
        break;
    case TARGET_TYPE::TARGET_TARGET_EXCEPT_CASTER:
        if (pSkillTarget->GetHandle() == GetHandle())
            return TS_RESULT_NOT_ACTABLE;
        break;
    default:
        break;
    }

    // Return feather
    if (pSkillTarget == nullptr)
    {
        if (nSkillID == 6020 && IsPlayer() /* && IsInSiegeOrRaidDungeon*/)
            return TS_RESULT_NOT_ACTABLE;
    }
    else
    {
        if (!pSkillTarget->IsInWorld())
            return TS_RESULT_NOT_ACTABLE;

        uint ct = sWorld.GetArTime();

        Position t = (pSkillTarget->GetCurrentPosition(ct));
        float target_distance = (GetCurrentPosition(ct).GetExactDist2d(&t) - GetUnitSize() * 0.5f);
        float enemy_distance = target_distance - (pSkillTarget->GetUnitSize() * 0.5f);
        float range_mod = 1.2f;
        if (pSkillTarget->bIsMoving)
        {
            if (pSkillTarget->IsInWorld())
                range_mod = 1.5f;
        }
        bool isInRange{false};
        if (pSkill->m_SkillBase->cast_range == -1)
        {
            isInRange = enemy_distance < GetRealAttackRange() * range_mod;
        }
        else
        {
            target_distance = 12 * pSkill->m_SkillBase->cast_range * range_mod;
            isInRange = enemy_distance < target_distance;
        }
        if (!isInRange)
            return TS_RESULT_TOO_FAR;

        if (pSkill->m_SkillBase->is_corpse != 0 && pSkillTarget->GetHealth() != 0)
            return TS_RESULT_NOT_ACTABLE;
        if (pSkill->m_SkillBase->is_corpse == 0 && pSkillTarget->GetHealth() == 0)
            return TS_RESULT_NOT_ACTABLE;

        if (pSkillTarget->GetHandle() == GetHandle() || (pSkillTarget->IsSummon() && pSkillTarget->As<Summon>()->GetMaster()->GetHandle() == GetHandle()))
        {
            if (!pSkill->m_SkillBase->IsUsable(0))
                return TS_RESULT_NOT_ACTABLE;
        }
        else
        {
            if (IsAlly(pSkillTarget))
            {
                if (!pSkill->m_SkillBase->IsUsable(1))
                    return TS_RESULT_NOT_ACTABLE;
            }
            else if (IsEnemy(pSkillTarget, false))
            {
                if (!pSkill->m_SkillBase->IsUsable(5))
                    return TS_RESULT_NOT_ACTABLE;
            }
            else if (!pSkill->m_SkillBase->IsUsable(3))
            {
                return TS_RESULT_NOT_ACTABLE;
            }
        }

        if ((pSkillTarget->IsPlayer() && pSkill->m_SkillBase->tf_avatar == 0) || (pSkillTarget->IsMonster() && pSkill->m_SkillBase->tf_monster == 0) || (pSkillTarget->IsSummon() && pSkill->m_SkillBase->tf_summon == 0))
            return TS_RESULT_NOT_ACTABLE;

        tpos = pSkillTarget->GetCurrentPosition(ct);
    }

    SetDirection(tpos);
    m_castingSkill = pSkill;
    int res = pSkill->Cast(nSkillLevel, target_handle, tpos, layer, bIsCastedByItem);
    if (res != TS_RESULT_SUCCESS)
    {
        m_castingSkill = nullptr;
    }

    return res;
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
    uint8 attack_count = 1;
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
    Position pos{};

    if (m_castingSkill != nullptr)
    {
        CancelSkill();
    }
    if (bIsMoving && IsInWorld())
    {
        pos = GetCurrentPosition(GetUInt32Value(UNIT_FIELD_DEAD_TIME));
        sWorld.SetMove(this, pos, pos, 0, true, sWorld.GetArTime(), true);
        if (IsPlayer())
        {
            // Ride handle
        }
    }
    if (GetTargetHandle() != 0)
        EndAttack();

    for (auto &state : m_vStateList)
    {
        Messages::BroadcastStateMessage(this, state, true);
    }
    m_vStateList.clear();
}

void Unit::AddEXP(int64 exp, uint jp, bool bApplyStamina)
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

bool Unit::TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList)
{
    bool result;
    if (item->GetWearType() != WEAR_CANTWEAR && item->IsWearable())
    {
        int elevel = m_nUnitExpertLevel;
        int level = GetLevel();
        if (m_nUnitExpertLevel <= level)
            elevel = level;
        result = (item->GetLevelLimit() <= elevel) && ((item->m_pItemBase->use_min_level == 0 || level >= item->m_pItemBase->use_min_level) && (item->m_pItemBase->use_max_level == 0 || level <= item->m_pItemBase->use_max_level));
    }
    else
    {
        result = false;
    }
    return result;
}

uint16_t Unit::putonItem(ItemWearType pos, Item *item)
{
    m_anWear[pos] = item;
    item->m_Instance.nWearInfo = pos;
    item->m_bIsNeedUpdateToDB = true;
    // Binded target
    if (IsPlayer())
    {
        auto p = dynamic_cast<Player *>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    else if (IsSummon())
    {
        auto p = dynamic_cast<Summon *>(this);
        Messages::SendItemWearInfoMessage(p->GetMaster(), this, item);
    }
    return 0;
}

ushort Unit::Puton(ItemWearType pos, Item *item, bool bIsTranslated)
{
    if (item->m_Instance.nWearInfo != WEAR_CANTWEAR)
        return 0;

    if (!bIsTranslated)
    {
        std::vector<int> vOverlapItemList{};
        if (!TranslateWearPosition(pos, item, vOverlapItemList))
            return 0;

        for (int &s : vOverlapItemList)
        {
            putoffItem((ItemWearType)s);
            if (m_anWear[s] != nullptr)
                return 0;
        }
    }
    return putonItem(pos, item);
}

uint16_t Unit::putoffItem(ItemWearType pos)
{
    auto item = m_anWear[pos];
    if (item == nullptr)
        return TS_RESULT_ACCESS_DENIED;

    item->m_Instance.nWearInfo = WEAR_NONE;
    item->m_bIsNeedUpdateToDB = true;
    // Binded Target
    m_anWear[pos] = nullptr;
    if (IsPlayer())
    {
        auto p = dynamic_cast<Player *>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    else if (IsSummon())
    {
        auto p = dynamic_cast<Summon *>(this);
        Messages::SendItemWearInfoMessage(p->GetMaster(), this, item);
    }
    return 0;
}

ushort Unit::Putoff(ItemWearType pos)
{
    if (pos == WEAR_TWOHAND)
        pos = WEAR_WEAPON;
    if (pos == WEAR_TWOFINGER_RING)
        pos = WEAR_RING;
    if (pos >= MAX_ITEM_WEAR || pos < 0)
        return TS_RESULT_NOT_ACTABLE;
    ItemWearType abspos = GetAbsoluteWearPos(pos);
    if (abspos == WEAR_CANTWEAR)
        return TS_RESULT_NOT_ACTABLE;
    if (pos != WEAR_BAG_SLOT)
        return putoffItem(abspos);

    // Todo: Bag

    return TS_RESULT_NOT_ACTABLE;
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

uint16 Unit::AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura, int nStateValue, std::string szStateValue)
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
                if (it->GetLevel() > level || it->GetLevel() == level && it->GetEndTime() > end_time)
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
    {
        sWorld.SetMove(this, pos, pos, 0, true, sWorld.GetArTime(), true);
    }
    else if (GetMoveSpeed() != speed)
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

uint16 Unit::onItemUseEffect(Unit *pCaster, Item *pItem, int type, float var1, float var2, const std::string &szParameter)
{
    uint16 result{TS_RESULT_ACCESS_DENIED};
    uint target_handle{0};
    Position pos{};
    std::string error{};
    uint ct = sWorld.GetArTime();
    uint prev_hp;
    uint prev_mp;

    auto pPlayer = this->As<Player>();

    switch (type)
    {
    case 1:
        prev_hp = GetHealth();
        HealByItem((int)var1);
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, 0, false);
        return TS_RESULT_SUCCESS;
    case 2:
        prev_mp = GetMana();
        MPHealByItem((int)var1);
        Messages::BroadcastHPMPMessage(this, 0, GetMana() - prev_mp, false);
        return TS_RESULT_SUCCESS;
    case 5: // Skillcast (e.g. Force/Soul Chips)
        target_handle = GetHandle();
        if (var1 == 6020.0f || var1 == 6021.0f)
            target_handle = pItem->GetHandle();
        pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        return (uint16)pCaster->CastSkill((int)var1, (int)var2, target_handle, pos, GetLayer(), true);
    case 6:
    {
        if (!IsPlayer())
        {
            AddState(StateType::SG_NORMAL, (StateCode)pItem->m_pItemBase->state_id, pItem->m_Instance.OwnerHandle,
                     pItem->m_pItemBase->state_level, ct, ct + (uint)(100 * pItem->m_pItemBase->state_time), false, 0, "");
            return TS_RESULT_SUCCESS;
        }
        auto state = sObjectMgr.GetStateInfo(pItem->m_pItemBase->state_id);
        if (state == nullptr)
            return TS_RESULT_NOT_ACTABLE;
        if (state->effect_type != StateEffect::SEF_RIDING)
        {
            if (pItem->m_pItemBase->state_id == 4003)
            {
                // @todo: stanima saver
            }
            AddState(StateType::SG_NORMAL, (StateCode)pItem->m_pItemBase->state_id, pItem->m_Instance.OwnerHandle,
                     pItem->m_pItemBase->state_level, ct, ct + (uint)(100 * pItem->m_pItemBase->state_time), false, 0, "");
            return TS_RESULT_SUCCESS;
        }
        result = TS_RESULT_ACCESS_DENIED;
    }
    break;
    case 7:
        RemoveState((StateCode)pItem->m_pItemBase->state_id, pItem->m_pItemBase->state_level);
        break;
    case 8:
    {
        auto pWornItem = GetWornItem(ItemWearType::WEAR_RIDE_ITEM);
        if (GetState((StateCode)pItem->m_pItemBase->state_id) != nullptr && pItem->GetHandle() == pWornItem->GetHandle())
        {
            RemoveState((StateCode)pItem->m_pItemBase->state_id, pItem->m_pItemBase->state_level);
            pPlayer->SetUInt32Value(PLAYER_FIELD_RIDING_UID, 0);
            return TS_RESULT_SUCCESS;
        }
        if (pPlayer != nullptr)
        {
            if (pPlayer->GetUInt32Value(PLAYER_FIELD_RIDING_IDX) != 0 || pPlayer->GetUInt32Value(PLAYER_FIELD_RIDING_UID) != 0 || pPlayer->IsInDungeon())
            {
                auto si = sObjectMgr.GetStateInfo(pItem->m_pItemBase->state_id);
                if (si != nullptr && si->effect_type == StateEffect::SEF_RIDING)
                    return TS_RESULT_ACCESS_DENIED;
            }
            if (pWornItem != nullptr)
            {
                if (pItem->GetHandle() != pWornItem->GetHandle())
                {
                    if (pPlayer->Putoff(ItemWearType::WEAR_RIDE_ITEM) != 0)
                        return TS_RESULT_ACCESS_DENIED;
                    if (pPlayer->Puton(ItemWearType::WEAR_RIDE_ITEM, pItem) != 0)
                        return TS_RESULT_ACCESS_DENIED;
                }
            }
            else
            {
                if (pPlayer->Puton(ItemWearType::WEAR_RIDE_ITEM, pItem) != 0)
                    return TS_RESULT_ACCESS_DENIED;
            }
            uint endTime = std::numeric_limits<unsigned int>::max();
            pPlayer->AddState((StateType)0, (StateCode)pItem->m_pItemBase->state_id, pItem->m_Instance.OwnerHandle,
                              pItem->m_pItemBase->state_level, ct, endTime, true, 0, "");
            return TS_RESULT_SUCCESS;
        }
    }
    break;
    case 80:
        if (IsPlayer())
        {
            dynamic_cast<Player *>(this)->AddStamina((int)var1);
            return TS_RESULT_SUCCESS;
        }
        return TS_RESULT_ACCESS_DENIED;
    case 94:
    {
        if (!pCaster->IsPlayer())
            return TS_RESULT_ACCESS_DENIED;
        if (var1 != 0.0f)
        {
            int total = 1;
            if (var1 < 0.0f)
                total = (int)var2;

            for (int i = 0; i < total; ++i)
            {
                auto nItemID = (int)var1;
                auto nItemCount = (int64)var2;
                while (nItemID < 0)
                    GameContent::SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
                if (nItemID != 0)
                {
                    auto pCItem = Item::AllocItem(0, nItemID, nItemCount, BY_ITEM, -1, -1, -1, -1, 0, 0, 0, 0);
                    if (pCItem == nullptr)
                    {
                        NG_LOG_ERROR("entities.item", "ItemID Invalid! %d", nItemID);
                        return TS_RESULT_NOT_ACTABLE;
                    }
                    Item *pNewItem = pPlayer->PushItem(pCItem, pCItem->m_Instance.nCount, false);
                    if (pNewItem != nullptr)
                        Messages::SendResult(pPlayer, 204, TS_RESULT_SUCCESS, pCItem->GetHandle());
                    if (pNewItem != nullptr && pNewItem->GetHandle() != pCItem->GetHandle())
                        Item::PendFreeItem(pCItem);
                }
            }
        }
        else
        {
            auto gold = pPlayer->GetGold() + (int64)var2;
            if (pPlayer->ChangeGold(gold) != TS_RESULT_SUCCESS)
                return 53;
        }
        return TS_RESULT_SUCCESS;
    }
    break;
    default:
        error = NGemity::StringFormat("Unit::onItemUseEffect [{}]: Unknown type {} !", pItem->m_Instance.Code, type);
        NG_LOG_ERROR("entites.unit", "%s", error.c_str());
        Messages::SendChatMessage(30, "@SYSTEM", dynamic_cast<Player *>(pCaster), error);
        result = TS_RESULT_UNKNOWN;
        break;
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
    result.target_hp = GetHealth();
    return result;
}

DamageInfo Unit::DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealPhysicalDamage(pFrom, (float)nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    result.target_hp = GetHealth();
    return result;
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

void Unit::SetJP(int jp)
{
    if (jp < 0)
        jp = 0;
    SetInt32Value(UNIT_FIELD_JOBPOINT, jp);
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

void Unit::SetEXP(int64 exp)
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

int64 Unit::GetBulletCount() const
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
                    st->AddState(StateType::SG_NORMAL, st->m_hCaster[0], (uint16)st->m_nLevel[0], st->m_nStartTime[0], (uint)(t - 1), st->m_nBaseDamage[0], false);
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
        if (aura.first == pSkill->m_SkillBase->toggle_group)
        {
            return true;
        }
    }
    return false;
}

bool Unit::TurnOnAura(Skill *pSkill)
{
    if (pSkill == nullptr)
        return false;

    if (m_vAura.count(pSkill->m_SkillBase->toggle_group) != 0)
    {
        return false;
    }

    m_vAura.emplace(pSkill->GetSkillBase()->GetToggleGroup(), pSkill->GetSkillId());
    AddState(SG_NORMAL, (StateCode)pSkill->m_SkillBase->state_id, GetHandle(), pSkill->m_SkillBase->GetStateLevel(pSkill->m_nSkillLevel, pSkill->GetSkillEnhance()), sWorld.GetArTime(), 0, true, 0, "");

    Messages::SendToggleInfo(this, pSkill->m_nSkillID, true);
    return true;
}

bool Unit::TurnOffAura(Skill *pSkill)
{
    if (pSkill == nullptr)
        return false;

    if (m_vAura.count(pSkill->m_SkillBase->toggle_group) == 0)
        return false;

    Messages::SendToggleInfo(this, pSkill->m_nSkillID, false);
    m_vAura.erase(pSkill->m_SkillBase->toggle_group);
    RemoveState(static_cast<StateCode>(pSkill->GetSkillBase()->GetStateId()), 255);
    if (pSkill->GetVar(3) != 0)
        RemoveState(static_cast<StateCode>(pSkill->GetVar(3)), 255);
    if (pSkill->GetVar(4) != 0)
        RemoveState(static_cast<StateCode>(pSkill->GetVar(4)), 255);
    return true;
}

void Unit::ToggleAura(Skill *pSkill)
{
    bool bNewAura = m_vAura.count(pSkill->m_SkillBase->toggle_group) == 0;
    if (m_vAura.count(pSkill->m_SkillBase->toggle_group) != 0)
    {
        bNewAura = m_vAura[pSkill->m_SkillBase->toggle_group] != pSkill->GetSkillId();
        TurnOffAura(GetSkill(m_vAura[pSkill->m_SkillBase->toggle_group]));
    }
    if (bNewAura)
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
    /*
    float v4;
    float v5;

    if (bPhysical)
    {
        if (bBad)
        {
            v4 = this.m_BadPhysicalElementalSkillStateMod[(int)type].fMagicalDamage;
            v5 = this.m_Attribute.nMagicPoint;
        }
        else
        {
            v4 = this.m_GoodPhysicalElementalSkillStateMod[(int)type].fMagicalDamage;
            v5 = this.m_Attribute.nMagicPoint;
        }
    }
    else
    {
        if (bBad)
        {
            v4 = this.m_BadMagicalElementalSkillStateMod[(int)type].fMagicalDamage;
            v5 = this.m_Attribute.nMagicPoint;
        }
        else
        {
            v4 = this.m_GoodMagicalElementalSkillStateMod[(int)type].fMagicalDamage;
            v5 = this.m_Attribute.nMagicPoint;
        }
    }*/
    return m_Attribute.nMagicPoint;
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
    /*
        if ( bPhysical )
        {
            if ( bBad )
                result = this.m_BadPhysicalElementalSkillStateMod[(int)type].fHate;
            else
                result = this.m_GoodPhysicalElementalSkillStateMod[(int)type].fHate;
        }
        else
        {
            if ( bBad )
                result = this.m_BadMagicalElementalSkillStateMod[(int)type].fHate;
            else
                result = this.m_GoodMagicalElementalSkillStateMod[(int)type].fHate;
        }*/
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