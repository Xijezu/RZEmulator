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
#include <limits>
#include "Unit.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Messages.h"
#include "ClientPackets.h"
#include "Skill.h"
#include "MemPool.h"
#include "RegionContainer.h"
// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

Unit::Unit(bool isWorldObject) : WorldObject(isWorldObject), m_unitTypeMask(0)
{
#ifdef _MSC_VER
#pragma warning(default:4355)
#endif
    _mainType   = MT_StaticObject;
    _objType    = OBJ_STATIC;
    _subType    = ST_Object;
    _bIsInWorld = false;
}

Unit::~Unit()
{
    uint ct = sWorld->GetArTime();

    for (auto &skill : m_vSkillList)
    {
        Skill::DB_InsertSkill(this, skill->m_nSkillUID, skill->m_nSkillID, skill->m_nSkillLevel, skill->m_nNextCoolTime < ct ? 0 : skill->m_nNextCoolTime - ct);
        delete skill;
        skill = nullptr;
    }
    m_vSkillList.clear();
}

void Unit::_InitTimerFieldsAndStatus()
{
    auto ct = sWorld->GetArTime();
    SetUInt32Value(UNIT_LAST_STATE_PROC_TIME, ct);
    SetUInt32Value(UNIT_LAST_UPDATE_TIME, ct);
    SetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME, ct);
    SetUInt32Value(UNIT_LAST_SAVE_TIME, ct);
    SetUInt32Value(UNIT_LAST_HATE_UPDATE_TIME, ct);
    SetFlag(UNIT_FIELD_STATUS, (STATUS_NEED_TO_CALCULATE_STAT | STATUS_ATTACKABLE | STATUS_SKILL_CASTABLE | STATUS_MOVABLE | STATUS_MAGIC_CASTABLE | STATUS_ITEM_USABLE | STATUS_MORTAL));
}


void Unit::AddToWorld()
{
    if (!IsInWorld()) {
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

void Unit::EnterPacket(XPacket &pEnterPct, Unit *pUnit, Player* pPlayer)
{
    pEnterPct << (uint32_t) Messages::GetStatusCode(pUnit, pPlayer);
    pEnterPct << pUnit->GetOrientation();
    pEnterPct << (int32_t) pUnit->GetHealth();
    pEnterPct << (int32_t) pUnit->GetMaxHealth();
    pEnterPct << (int32_t) pUnit->GetMana();
    pEnterPct << (int32_t) pUnit->GetMaxMana();
    pEnterPct << (int32_t) pUnit->GetLevel();
    pEnterPct << (uint8_t) pUnit->GetUInt32Value(UNIT_FIELD_RACE);
    pEnterPct << (uint32_t) pUnit->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    pEnterPct << (uint8_t) (pUnit->HasFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ENTER) ? 1 : 0);
    pEnterPct << (int32_t) 0;
}

Item *Unit::GetWornItem(ItemWearType idx)
{
    if ((uint) idx >= MAX_ITEM_WEAR || idx < 0)
        return nullptr;
    return m_anWear[idx];
}

void Unit::SetHealth(int hp)
{
    int old_hp = GetHealth();
    SetInt32Value(UNIT_FIELD_HEALTH, hp);
    if (hp > GetMaxHealth())
        SetInt32Value(UNIT_FIELD_HEALTH, GetMaxHealth());
    //if (old_hp != GetHealth())
    //this.onHPChange(old_hp);
}

void Unit::SetMana(int mp)
{
    int old_np = GetMana();
    SetInt32Value(UNIT_FIELD_MANA, mp);
    if (mp > GetMaxMana())
        SetInt32Value(UNIT_FIELD_MANA, GetMaxMana());
}

void Unit::CleanupBeforeRemoveFromMap(bool finalCleanup)
{

}

void Unit::SetMultipleMove(std::vector<Position>& _to, uint8_t _speed, uint _start_time)
{
    ArMoveVector::SetMultipleMove(_to, _speed, _start_time, sWorld->GetArTime());
    lastStepTime = start_time;
}

void Unit::SetMove(Position _to, uint8 _speed, uint _start_time)
{
    ArMoveVector::SetMove(_to, _speed, _start_time, sWorld->GetArTime());
    lastStepTime = start_time;
}

void Unit::processPendingMove()
{
    uint     ct = sWorld->GetArTime();
    Position pos{ };//             = ArPosition ptr -10h


    if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
    {
        if (m_nMovableTime < ct)
        {
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED);
            if (IsActable() && IsMovable())
            {
                pos = GetCurrentPosition(ct);
                sWorld->SetMultipleMove(this, pos, m_PendingMovePos, m_nPendingMoveSpeed, true, ct, true);
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
    uint ct = sWorld->GetArTime();
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_CALCULATE_STAT))
    {
        CalculateStat();
        RemoveFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_CALCULATE_STAT);
    }
    this->regenHPMP(ct);

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
    int   prev_hp;
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
                pt     = GetMaxHealth() * m_Attribute.nHPRegenPercentage;
                pt     = pt * 0.01f * etf;// + 0.0;
                pt     = pt + m_Attribute.nHPRegenPoint * etf;
                if(IsSitdown())
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
                pt = pt * 0.01f * etf;// +0.0;
                pt = pt + m_Attribute.nMPRegenPoint * etf;

                if(IsSitdown())
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
                    XPacket hpmpPct(TS_SC_REGEN_HPMP);
                    hpmpPct << (uint)GetHandle();
                    hpmpPct << (int16)m_nRegenHP;
                    hpmpPct << (int16)m_fRegenMP;
                    hpmpPct << (int32)GetHealth();
                    hpmpPct << (int16)GetMana();

                    this->m_nRegenHP = 0;
                    this->m_fRegenMP = 0;
                    if (IsInWorld())
                    {
                        sWorld->Broadcast((uint)(GetPositionX() / g_nRegionSize), (uint)(GetPositionY() / g_nRegionSize), GetLayer(), hpmpPct);
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

int Unit::GetBaseSkillLevel(int skill_id)  const
{
    Skill *s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel;
}

Skill *Unit::GetSkill(int skill_id)  const
{
    for (auto s : m_vSkillList) {
        if (s->m_nSkillID == skill_id)
            return s;
    }
    return nullptr;
}

Skill *Unit::RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID)
{
    Skill *pSkill = nullptr;
    int   nNeedJP = sObjectMgr->GetNeedJpForSkillLevelUp(skill_id, skill_level, nJobID);
    if (GetJP() >= nNeedJP) {
        SetJP(GetJP() - nNeedJP);
        if (GetJP() < 0)
            SetJP(0);

        onExpChange();

        int64 nSkillUID  = 0;
        int      nPrevLevel = GetBaseSkillLevel(skill_id);
        if (nPrevLevel == 0) {
            nSkillUID = sWorld->GetSkillIndex();
            pSkill    = new Skill{this, nSkillUID, skill_id};
            m_vSkillList.emplace_back(pSkill);
        } else {
            pSkill    = GetSkill(skill_id);
            nSkillUID = pSkill == nullptr ? 0 : pSkill->m_nSkillUID;
        }
        if (pSkill != nullptr) {
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
    auto skill = new Skill{this, skill_uid, skill_id};
    skill->m_nSkillID      = skill_id;
    skill->m_nSkillLevel   = skill_level;
    skill->m_nSkillUID     = skill_uid;
    skill->m_nNextCoolTime = remain_cool_time + sWorld->GetArTime();
    m_vSkillList.emplace_back(skill);
}

int Unit::CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    auto   player = dynamic_cast<Player *>(this);
    Summon *summon{nullptr};
    Unit *pSkillTarget{nullptr};
    auto tpos = pos.GetPosition();

    auto pSkill = GetSkill(nSkillID);
    if (pSkill == nullptr || pSkill->m_SkillBase == nullptr || m_castingSkill != nullptr /*|| using storage */)
        return TS_RESULT_NOT_ACTABLE;

    //auto pSkillTarget = sMemoryPool->getPtrFromId(target_handle);
    auto obj = sMemoryPool->GetObjectInWorld<WorldObject>(target_handle);
    if (obj != nullptr && (!obj->IsItem() && !obj->IsFieldProp()))
    {
        pSkillTarget = dynamic_cast<Unit *>(obj);
    }

    switch (pSkill->m_SkillBase->target)
    {
        case TARGET_MASTER:
            if (!IsSummon())
                return TS_RESULT_NOT_ACTABLE;
            summon = dynamic_cast<Summon *>(this);
            if (summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        case TARGET_SELF_WITH_MASTER:
            if (!IsSummon())
                return TS_RESULT_NOT_ACTABLE;
            summon = this->As<Summon>();
            if (pSkillTarget->GetHandle() != GetHandle() && summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        case TARGET_TARGET_EXCEPT_CASTER:
            if (pSkillTarget->GetHandle() == GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        default:
            break;
    }

    // Return feather
    if (pSkillTarget == nullptr)
    {
        if(nSkillID == 6020 && IsPlayer() /* && IsInSiegeOrRaidDungeon*/)
            return TS_RESULT_NOT_ACTABLE;
    }
    else
    {
        if (!pSkillTarget->IsInWorld())
            return TS_RESULT_NOT_ACTABLE;

        uint ct = sWorld->GetArTime();

        Position t               = (pSkillTarget->GetCurrentPosition(ct));
        float    target_distance = (GetCurrentPosition(ct).GetExactDist2d(&t) - GetUnitSize() * 0.5f);
        float    enemy_distance  = target_distance - (pSkillTarget->GetUnitSize() * 0.5f);
        float    range_mod       = 1.2f;
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
            isInRange       = enemy_distance < target_distance;
        }
        if (!isInRange)
            return TS_RESULT_TOO_FAR;

        if (pSkill->m_SkillBase->is_corpse != 0 && pSkillTarget->GetHealth() != 0)
            return TS_RESULT_NOT_ACTABLE;
        if(pSkill->m_SkillBase->is_corpse == 0 && pSkillTarget->GetHealth() == 0)
            return TS_RESULT_NOT_ACTABLE;

        if (pSkillTarget->GetHandle() == GetHandle()
            || (pSkillTarget->IsSummon()
               && pSkillTarget->As<Summon>()->GetMaster()->GetHandle() == GetHandle()))
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

        if ((pSkillTarget->IsPlayer() && pSkill->m_SkillBase->tf_avatar == 0)
            || (pSkillTarget->IsMonster() && pSkill->m_SkillBase->tf_monster == 0)
            || (pSkillTarget->IsSummon() && pSkill->m_SkillBase->tf_summon == 0))
            return TS_RESULT_NOT_ACTABLE;

        tpos = pSkillTarget->GetCurrentPosition(ct);
    }

    SetDirection(tpos);
    m_castingSkill = pSkill;
    int res = pSkill->Cast(nSkillLevel, target_handle, tpos, layer, bIsCastedByItem);
    if(res != TS_RESULT_SUCCESS)
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
        if((IsUsingBow() || IsUsingCrossBow()) && IsPlayer())
            m_nNextAttackMode = 1;
        if (bNeedFastReaction)
            onAttackAndSkillProcess();
        return true;
    }
}

void Unit::processAttack()
{
    uint t = sWorld->GetArTime();
    if (GetHealth() == 0 /*IsAttackable()*/)
        return;

    Player *player{nullptr};

    if (GetNextAttackableTime() <= t)
    {
        if((IsUsingCrossBow() || IsUsingBow()) && IsPlayer())
        {
            auto bullets = GetBulletCount();
            if(bullets < 1)
            {
                EndAttack();
                return;
            }
        }

        //auto enemy = dynamic_cast<Unit *>(sMemoryPool->getPtrFromId(GetTargetHandle()));
        auto enemy = sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle());
        if (GetHealth() == 0)
        {
            CancelAttack();
            return;
        }
        if (IsMoving(t) || GetTargetHandle() == 0)
            return;

        if (enemy == nullptr || !IsEnemy(enemy, false) || enemy->GetHealth() == 0 || sRegion->IsVisibleRegion(this, enemy) == 0)
        {
            if (IsPlayer())
            {
                player = dynamic_cast<Player *>(this);
            }
            else if (IsSummon())
            {
                auto summon = this->As<Summon>();
                if (summon != nullptr)
                    player = summon->GetMaster();
            }

            if (player != nullptr)
            {
                //Messages::SendHPMPMessage(player, enemy, 0, 0, true);
                Messages::SendCantAttackMessage(player, player->GetHandle(), player->GetTargetHandle(), TS_RESULT_NOT_EXIST);
            }
            EndAttack();
            return;
        }

        if (HasFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK))
        {
            enemy->OnUpdate();
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
        }

        if (enemy->GetHealth() == 0)
        {
            if (IsPlayer())
            {
                player = dynamic_cast<Player *>(this);
            }
            else if (IsSummon())
            {
                auto summon = this->As<Summon>();
                if (summon != nullptr)
                    player = summon->GetMaster();
            }

            if (player != nullptr)
                Messages::SendCantAttackMessage(player, player->GetHandle(), player->GetTargetHandle(), TS_RESULT_NOT_EXIST);
            EndAttack();
            return;
        }

        auto enemyPosition = enemy->GetCurrentPosition(t);
        auto myPosition    = GetCurrentPosition(t);

        auto real_distance = myPosition.GetExactDist2d(&enemyPosition) - ((enemy->GetUnitSize() * 0.5f) + (GetUnitSize() * 0.5f));
        auto attack_range  = GetRealAttackRange();
        SetDirection(enemyPosition);
        if (enemy->bIsMoving && enemy->IsInWorld())
            attack_range *= 1.5f;
        else
            attack_range *= 1.200000047683716f;

        AttackInfo Damages[4]{ };

        bool _bDoubleAttack{false};

        uint attack_interval = GetAttackInterval();
        auto attInt          = GetAttackInterval();
        if (attack_range < real_distance)
        {
            onCantAttack(enemy->GetHandle(), t);
            return;
        }
        int next_mode = m_nNextAttackMode;
        // If Bow/Crossbow
        if ((IsUsingBow() || IsUsingCrossBow()) && IsPlayer())
        {
            if(m_nNextAttackMode == 1)
            {
                attInt = (uint)(GetBowAttackInterval()  * 0.8f);
                SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, attInt + t);
                m_nNextAttackMode = 0;
                SetFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED);

                bool bFormChanged = HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED);
                if(bFormChanged)
                {
                    // @todo: form changed
                }
                if(!bFormChanged || next_mode != 1)
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
            if((IsUsingCrossBow() || IsUsingBow()) && IsPlayer())
            {
                player = dynamic_cast<Player*>(this);
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

void Unit::Attack(Unit *pTarget, uint t, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack)
{
    uint ct = t;
    if (ct == 0)
        ct = sWorld->GetArTime();

    DamageInfo di{ };

    SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, attack_interval + ct);
    bIsDoubleAttack = false;

    int nHate        = 0;
    int nAttackCount = 1;
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        nAttackCount = 2;
    if (((uint)rand32() % 100) < m_Attribute.nDoubleAttackRatio)
    {
        bIsDoubleAttack = true;
        nAttackCount *= 2;
    }

    if (nAttackCount <= 0)
        return;

    int i = 0;
    do
    {
        attack_interval = 0;
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        {
            if (((uint)i & 1) != 0)
                attack_interval = 1;
        }

        auto prev_target_hp = pTarget->GetHealth();
        auto prev_target_mp = pTarget->GetMana();
        auto prev_hp        = GetHealth();
        auto prev_mp        = GetMana();
        int  crit           = 0;
        // TODO Crit

        if (attack_interval != 0)
            di = pTarget->DealPhysicalNormalLeftHandDamage(this, m_Attribute.nAttackPointLeft, TYPE_NONE, 0, crit, 0);
        else
            di = pTarget->DealPhysicalNormalDamage(this, m_Attribute.nAttackPointRight, TYPE_NONE, 0, crit, 0);
        arDamage[i].SetDamageInfo(di);

        if (arDamage[i].bCritical)
        {
            // Do Crit calc
        }

        if (!arDamage[i].bMiss)
        {
            /// Do more calc TODO
        }

        arDamage[i].nDamage            = prev_target_hp - pTarget->GetHealth();
        arDamage[i].mp_damage          = (uint16)(prev_target_mp - pTarget->GetMana());
        arDamage[i].attacker_damage    = (short)(prev_hp - GetHealth());
        arDamage[i].attacker_mp_damage = (short)(prev_mp - GetMana());
        arDamage[i].target_hp          = pTarget->GetHealth();
        arDamage[i].target_mp          = (uint16)pTarget->GetMana();
        arDamage[i].attacker_hp        = GetHealth();
        arDamage[i].attacker_mp        = (uint16)GetMana();

        nHate += arDamage[i].nDamage;

        if (!arDamage[i].bMiss)
        {
            // Set Havoc TODO
        }
        ++i;
    } while (i < nAttackCount);
    if (pTarget->IsMonster())
    {
        auto mob = dynamic_cast<Monster *>(pTarget);
        auto hm  = GetHateMod(3, true);
        bool usingBow = (IsUsingBow() || IsUsingCrossBow()) && IsPlayer();
        nHate = (int)((float)(nHate + hm.second) * hm.first);
        // @todo hate calc
        mob->AddHate(GetHandle(), nHate, true, true);
    }
}

DamageInfo Unit::DealPhysicalNormalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    int nTargetGroup = pFrom->GetCreatureGroup();
    int damage{};
    StateMod damageReduceByState{};

    // Do damage reduce

    if(nDamage < 0)
        nDamage = 0;

    Damage d = DealPhysicalLeftHandDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);

    result.target_hp = GetHealth();
    return result;
}


DamageInfo Unit::DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    int nTargetGroup = pFrom->GetCreatureGroup();
    bool bRange;
    int damage{};
    StateMod damageReduceByState{};

    if((pFrom->IsUsingBow() || pFrom->IsUsingCrossBow()) && pFrom->IsPlayer())
    {
        bRange = true;
        // @todo: Damage reduce by state
    }
    else
    {
        bRange = false;
        // @todo: Damage reduce by state
    }

    // Do damage reduce

    if(nDamage < 0)
        nDamage = 0;

    Damage d{};
    if(bRange) {
        d = DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    } else {
        d = DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    }
    result.SetDamage(d);
    if(!result.bMiss && !result.bPerfectBlock)
    {
        std::vector<AdditionalDamageInfo> v_add = bRange ? pFrom->m_vRangeAdditionalDamage : pFrom->m_vNormalAdditionalDamage;
        for(auto& addi : v_add)
        {
            if(addi.ratio > (uint8)((uint)rand32() % 100))
            {
                if(addi.nDamage != 0)
                    damage = addi.nDamage;
                else
                    damage = (int)(addi.fDamage * (float)result.nDamage);
                Damage dd = DealDamage(pFrom, damage, addi.type, DT_ADDITIONAL_DAMAGE, 0, 0, 0, nullptr, nullptr);
                result.nDamage += dd.nDamage;
            }
        }
    }

    result.target_hp = GetHealth();
    return result;
}

Damage Unit::DealDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod* damage_penalty, StateMod* damage_advantage)
{
    int   nCritical{0};
    float fCritical{0};

    if (damage_penalty != nullptr)
    {
        nCritical = damage_penalty->nDamage;
        fCritical = damage_penalty->fCritical;
    }
    else
    {
        nCritical = 0;
        fCritical = 0.0f;
    }

    if (damage_advantage != nullptr)
    {
        nCritical += damage_advantage->nCritical;
        fCritical += damage_advantage->fCritical - 1.0f;
    }

    auto result        = pFrom->CalcDamage(this, damageType, nDamage, elemental_type, accuracy_bonus, fCritical + 1.0f, critical_bonus + nCritical, nFlag);

    if (!result.bMiss)
    {
        if (damage_penalty != nullptr)
        {
            result.nDamage += damage_penalty->nDamage;
            result.nDamage = (int)((float)result.nDamage * damage_penalty->fDamage);
        }
        if (damage_advantage != nullptr)
        {
            result.nDamage += damage_advantage->nDamage;
            result.nDamage = (int)((float)result.nDamage * damage_advantage->fDamage);
        }
    }
    if (result.nDamage < 0)
        result.nDamage = 0;

    //             if (damageType == DamageType.DT_NORMAL_PHYSICAL_DAMAGE)
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio; // goto LABEL_26;
//             else if ( damageType == DamageType.DT_NORMAL_MAGICAL_DAMAGE)
//                 fDamageFlag = this.m_fMagicalDamageManaShieldAbsorbRatio; //goto LABEL_29;
// //             if ( damageType <= DamageType.DT_NORMAL_MAGICAL_DAMAGE)
// //                 goto LABEL_36;
//             else if ( damageType <= DamageType.AdditionalLeftHand)
//             {
//         LABEL_26:
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio;
//                 goto LABEL_27;
//             }
//             if ( damageType > DamageType.StateMagical)
//             {
//                 if ( damageType != DamageType.StatePhysical)
//                     goto LABEL_36;
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio;
//                 goto LABEL_27;
//             }
//         LABEL_29:
//             fDamageFlag = this.m_fMagicalDamageManaShieldAbsorbRatio;
//         LABEL_27:
//             fDamageFlag = v20;

    float fDamageFlag = 0; /*m_fPhysicalDamageManaShieldAbsorbRatio*/
    int   nDamageFlag{0};
    if (fDamageFlag < 0.0f)
        fDamageFlag = 0.0f;
    if (fDamageFlag > 0.0f)
    {
        if (fDamageFlag > 1.0f)
            fDamageFlag = 1.0f;
        nDamageFlag     = (int)(result.nDamage * fDamageFlag);
        if (GetMana() < nDamageFlag)
            nDamageFlag = GetMana();
        result.nDamage -= nDamageFlag;
        AddMana(-nDamageFlag);
        Messages::BroadcastHPMPMessage(this, 0, nDamageFlag, false);
    }

    int real_damage = onDamage(pFrom, elemental_type, damageType, result.nDamage, result.bCritical);
    damage(pFrom, real_damage, true);

    if (!result.bMiss)
    {
        auto nPrevHP = pFrom->GetHealth();
        auto nPrevMP = pFrom->GetMana();

        Messages::BroadcastHPMPMessage(pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, false);

        if (IsPlayer())
        {
            Messages::SendHPMPMessage(dynamic_cast<Player *>(this), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }
        else if (IsSummon())
        {
            auto s = dynamic_cast<Summon *>(this);
            if (s != nullptr && s->GetMaster() != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }

        if (pFrom->IsPlayer())
        {
            Messages::SendHPMPMessage(dynamic_cast<Player *>(pFrom), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }
        else if (pFrom->IsSummon())
        {
            auto s = dynamic_cast<Summon *>(pFrom);
            if (s != nullptr && s->GetMaster() != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }

    }
    result.nDamage = real_damage;
    return result;
}

Damage Unit::DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod* damage_penalty, StateMod* damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_NORMAL_PHYSICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealPhysicalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod* damage_advantage)
{
    return DealDamage(pFrom, nDamage, elemental_type, DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, type, DT_NORMAL_MAGICAL_DAMAGE, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}


Damage Unit::CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag)
{
    Damage result{ };
    if (damage_type == DT_NORMAL_MAGICAL_DAMAGE || damage_type == DT_STATE_MAGICAL_DAMAGE)
    {
        // TODO how about no?
    }

    auto nDamagec             = int(m_Expert[GetCreatureGroup()].fDamage * nDamage + nDamage);
    auto nDamagea             = int(pTarget->m_Expert[GetCreatureGroup()].fAvoid * nDamagec + nDamagec);

    float fDefAdjustb{0}, fDefAdjustc{0}, fDefAdjust{0};
    bool  bIsPhysicalDamage   = damage_type == DT_NORMAL_PHYSICAL_DAMAGE || damage_type == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE || damage_type == DT_STATE_PHYSICAL_DAMAGE || damage_type == DT_NORMAL_PHYSICAL_SKILL_DAMAGE;
    bool  bIsMagicalDamage    = damage_type == DT_NORMAL_MAGICAL_DAMAGE || damage_type == DT_STATE_MAGICAL_DAMAGE;
    bool  bIsAdditionalDamage = damage_type == DT_ADDITIONAL_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE || damage_type == DT_ADDITIONAL_MAGICAL_DAMAGE;
    bool  bIsLeftHandDamage   = damage_type == DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE;
    bool  bDefAdjust          = damage_type == DT_STATE_PHYSICAL_DAMAGE || damage_type == DT_STATE_MAGICAL_DAMAGE;

    int nAccuracy{0};
    int nPercentage{0};

    if ((nFlag & 2) == 0 && bIsPhysicalDamage && !bDefAdjust)
    {
        if (bIsLeftHandDamage)
            nAccuracy = (int)m_Attribute.nAccuracyLeft;
        else
            nAccuracy = (int)m_Attribute.nAccuracyRight;

        nPercentage     = 2 * ((44 - pTarget->GetLevel()) + GetLevel());
        if (nPercentage < 10)
            nPercentage = 10;

        nPercentage = (int)(nAccuracy / pTarget->m_Attribute.nAvoid * (float)nPercentage + 7.0f + accuracy_bonus);
        //nAccuracy   = (int)pTarget->m_Attribute.nAvoid;
        if ((uint)rand32() % 100 > nPercentage)
        {
            result.bMiss   = true;
            result.nDamage = 0;
            return result;
        }
    }

    if ((nFlag & 2) == 0 && bIsMagicalDamage && !bDefAdjust)
    {
        nAccuracy     = 2 * ((44 - pTarget->GetLevel()) + GetLevel());
        nPercentage   = (int)(m_Attribute.nMagicAccuracy / pTarget->m_Attribute.nMagicAvoid * nAccuracy + 7.0f + accuracy_bonus);
        if (nAccuracy < 10)
            nAccuracy = 10;
        if ((uint)rand32() % 100 > nPercentage)
        {
            result.bMiss   = true;
            result.nDamage = 0;
            return result;
        }
    }

    int   nRandomDamage = 0;
    float nDefence      = 0;
    if (bIsAdditionalDamage && bDefAdjust)
    {
        float fDefAdjusta = 1.0f;
        if (bIsMagicalDamage && (m_Attribute.nMagicPoint < pTarget->m_Attribute.nMagicDefence))
        {
            if (pTarget->m_Attribute.nMagicDefence <= 0)
                fDefAdjusta = 1.0f;
            else
                fDefAdjusta = pTarget->m_Attribute.nMagicDefence;
            fDefAdjusta     = 1.0f - ((pTarget->m_Attribute.nMagicDefence - m_Attribute.nMagicPoint) / (2 * fDefAdjusta));
        }
        else if (bIsPhysicalDamage && (m_Attribute.nAttackPointRight < pTarget->m_Attribute.nDefence))
        {
            if (pTarget->m_Attribute.nDefence <= 0)
                fDefAdjusta = 1;
            else
                fDefAdjusta = pTarget->m_Attribute.nDefence;
            fDefAdjusta     = 1.0f - ((pTarget->m_Attribute.nDefence - m_Attribute.nAttackPointRight) / (2 * fDefAdjusta));
        }
        fDefAdjust = (int)((float)nDamagea * fDefAdjusta);
    }
    else
    {
        fDefAdjust = nDamagea;
        if (!IsFieldProp())
        {
            if ((nFlag & 4) == 0)
            {
                if (bIsPhysicalDamage)
                {
                    nDefence = pTarget->m_Attribute.nDefence;
                    // TODO Shield
                }
                else if (bIsMagicalDamage)
                {
                    nDefence = pTarget->m_Attribute.nMagicDefence;
                }
            }
            float nDamageb = 1.0f - 0.4f * nDefence / nDamagea;
            if (nDamageb < 0.3f)
                nDamageb = 0.3f;

            fDefAdjustc     = 1.0f - nDefence * 0.5f / nDamagea;
            if (fDefAdjustc < 0.05f)
                fDefAdjustc = 0.05f;

            int nDefencea = GetLevel();
            fDefAdjust     = (int)((float)nDefencea * 1.7f * nDamageb + nDamagea * fDefAdjustc);
            if (fDefAdjust < 1)
                fDefAdjust = 1;
        }
        // TODO IgnoreRandomDamage
        if (true)
        {
            nRandomDamage = irand((int)(-(fDefAdjust * 0.05f)), (int)(fDefAdjust * 0.05f));
        }
    }

    fDefAdjustb = (fDefAdjust + nRandomDamage);
    if (!bIsAdditionalDamage && (nFlag & 0x10) == 0)
    {
        int cd = 0; // GetCriticalDamage
        if (cd != 0)
        {
            fDefAdjustb += cd;
            result.bCritical = true;
        }
    }
    if ((damage_type == DT_ADDITIONAL_DAMAGE || damage_type == DT_ADDITIONAL_LEFT_HAND_DAMAGE) && HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
    {
        if (bIsLeftHandDamage)
            fDefAdjustb = (int)(fDefAdjustb * (float)1/*m_nDoubleWeaponMasteryLevel*/ * 0.02f + 0.44f);
        else
            fDefAdjustb *= (int)(fDefAdjustb * (float)1/*m_nDoubleWeaponMasteryLevel*/ * 0.001f + 0.9f);
    }

    float nDamaged = 1.0f - (pTarget->m_Resist.nResist[(int)elemental_type] / 300);
    result.nDamage         = (int)(nDamaged * fDefAdjustb);
    result.nResistedDamage = fDefAdjustb - result.nDamage;

    if (bIsPhysicalDamage && pTarget->GetStateByEffectType(SEF_FORCE_CHIP) != nullptr)
        result.nDamage *= 2;
    else if (bIsMagicalDamage && pTarget->GetStateByEffectType(SEF_SOUL_CHIP) != nullptr)
        result.nDamage *= 2;
    else if (pTarget->GetStateByEffectType(SEF_LUNAR_CHIP) != nullptr)
        result.nDamage *= 2;

    if ((pTarget->IsPlayer() || pTarget->IsSummon()) && GetHandle() != pTarget->GetHandle())
    {
        if (IsPlayer())
            result.nDamage = (int)((float)result.nDamage * 1 /*PVPRateForPlayer*/);
        else if (IsSummon())
            result.nDamage = (int)((float)result.nDamage * 1 /*PVPRateForSummon*/);
    }
    if (bIsMagicalDamage && !bIsAdditionalDamage && fDefAdjustb < 1)
        result.nDamage     = 1;
    return result;
}

uint Unit::GetCreatureGroup() const
{
    return 0;
}

int Unit::damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead)
{
    int result{0};
    if (GetHealth() != 0)
    {
        //if(HasFlag(UNIT_FIELD_STATUS, STATUS_HIDING))
        //1RemoveState(StateCode::Hide, 65535);

        if (GetHealth() <= nDamage)
            SetHealth(0);
        else
            AddHealth(-nDamage);

        if (GetHealth() == 0)
        {
            SetUInt32Value(UNIT_FIELD_DEAD_TIME, sWorld->GetArTime());
            onDead(pFrom, decreaseEXPOnDead);
        }
        result = nDamage;
    }
    return result;
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

    XPacket pct(TS_SC_ATTACK_EVENT);
    pct << GetHandle();
    if (pTarget != nullptr)
        pct << pTarget->GetHandle();
    else
        pct << (uint)0;
    pct << (uint16)tm; // attack_speed
    pct << (uint16)delay;

    uint8 attack_flag = 0;
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED))
    {
        if (bIsDoubleAttack)
            attack_flag = ATTACK_FLAG_DOUBLE_ATTACK;
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            attack_flag |= ATTACK_FLAG_DOUBLE_WEAPON;
        if (IsUsingBow() && IsPlayer())
            attack_flag |= ATTACK_FLAG_BOW;
        if (IsUsingCrossBow() && IsPlayer())
            attack_flag |= ATTACK_FLAG_CROSS_BOW;
    }

    uint8 attack_action = ATTACK_ATTACK;
    if (bIsAiming)
        attack_action = ATTACK_AIMING;
    else if (bEndAttack)
        attack_action = ATTACK_END;
    else if (bCancelAttack)
        attack_action = ATTACK_CANCEL;

    pct << attack_action;
    pct << attack_flag;

    pct << attack_count;
    for (int i = 0; i < attack_count; ++i)
    {
        pct << (uint16)arDamage[i].nDamage;
        pct << (uint16)arDamage[i].mp_damage;
        uint8 flag = 0;
        if (arDamage[i].bPerfectBlock)
            flag = FLAG_PERFECT_BLOCK;
        if (arDamage[i].bBlock)
            flag |= FLAG_BLOCK;
        if (arDamage[i].bMiss)
            flag |= FLAG_MISS;
        if (arDamage[i].bCritical)
            flag |= FLAG_CRITICAL;
        pct << flag;
        for (auto &ed : arDamage[i].elemental_damage)
        {
            pct << (uint16)ed;
        }
        pct << arDamage[i].target_hp;
        pct << (uint16)arDamage[i].target_mp;
        pct << (uint16)arDamage[i].attacker_damage;
        pct << (uint16)arDamage[i].attacker_mp_damage;
        pct << (int)arDamage[i].attacker_hp;
        pct << (uint16)arDamage[i].attacker_mp;
    }
    sWorld->Broadcast((uint)GetPositionX() / g_nRegionSize, (uint)GetPositionY() / g_nRegionSize, GetLayer(), pct);
}

void Unit::EndAttack()
{
    AttackInfo info[4]{};
    if((IsUsingBow() || IsUsingCrossBow()) && IsPlayer() && m_nNextAttackMode == 0)
    {
        m_nNextAttackMode = 1;
        SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, sWorld->GetArTime());
    }
    if(HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED)) {
        //auto target = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(GetTargetHandle()));
        auto target = sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle());
        if(IsPlayer() || IsSummon()) {
            if(target != nullptr)
                broadcastAttackMessage(target, info, 0, 0, false, false, true, false);
        }
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
}

void Unit::onDead(Unit *pFrom, bool decreaseEXPOnDead)
{
    Position pos{ };

    if (m_castingSkill != nullptr)
    {
        CancelSkill();
    }
    if (bIsMoving && IsInWorld())
    {
        pos = GetCurrentPosition(GetUInt32Value(UNIT_FIELD_DEAD_TIME));
        sWorld->SetMove(this, pos, pos, 0, true, sWorld->GetArTime(), true);
        if (IsPlayer())
        {
            // Ride handle
        }
    }
    if (GetTargetHandle() != 0)
        EndAttack();

    for (auto& state : m_vStateList)
    {
        Messages::BroadcastStateMessage(this, state, true);
    }
    m_vStateList.clear();
}

void Unit::AddEXP(int64 exp, uint jp, bool bApplyStanima)
{
    SetUInt64Value(UNIT_FIELD_EXP, GetEXP() + exp);
    SetUInt32Value(UNIT_FIELD_JOBPOINT, GetJP() + jp);
    // SetTotalJP
    if(HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

void Unit::CancelSkill()
{
    if(m_castingSkill != nullptr && m_castingSkill->Cancel()) {
        m_castingSkill = nullptr;
    }
}

void Unit::CancelAttack()
{
    AttackInfo info[4]{};
    if((IsUsingCrossBow() || IsUsingBow()) && (IsPlayer() && m_nNextAttackMode == 0))
    {
        m_nNextAttackMode = 1;
        SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, sWorld->GetArTime());
    }
    if(HasFlag(UNIT_FIELD_STATUS, STATUS_ATTACK_STARTED)) {
        this->broadcastAttackMessage(sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle()), info, 0, 0, false, false, false, true);
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
}

bool Unit::TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList)
{
    bool result;
    if(item->GetWearType() != WEAR_CANTWEAR && item->IsWearable()) {
        int elevel = m_nUnitExpertLevel;
        int level = GetLevel();
        if(m_nUnitExpertLevel <= level)
            elevel = level;
        result = (item->GetLevelLimit() <= elevel) && ((item->m_pItemBase->use_min_level == 0 || level >= item->m_pItemBase->use_min_level)
                                                      && (item->m_pItemBase->use_max_level == 0 || level <= item->m_pItemBase->use_max_level));
    } else {
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
    if(IsPlayer()) {
        auto p = dynamic_cast<Player*>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    else if(IsSummon())
    {
        auto p = dynamic_cast<Summon*>(this);
        Messages::SendItemWearInfoMessage(p->GetMaster(), this, item);
    }
    return 0;
}

ushort Unit::Puton(ItemWearType pos, Item *item)
{
    if(item->m_Instance.nWearInfo != WEAR_CANTWEAR)
        return 0;

    std::vector<int> vOverlapItemList{ };
    if(!TranslateWearPosition(pos, item, vOverlapItemList))
        return 0;

    for(int& s : vOverlapItemList) {
        putoffItem((ItemWearType)s);
        if(m_anWear[s] != nullptr)
            return 0;
    }
    return putonItem(pos, item);
}

uint16_t Unit::putoffItem(ItemWearType pos)
{
    auto item = m_anWear[pos];
    if(item == nullptr)
        return TS_RESULT_ACCESS_DENIED;

    item->m_Instance.nWearInfo = WEAR_NONE;
    item->m_bIsNeedUpdateToDB = true;
    // Binded Target
    m_anWear[pos] = nullptr;
    if(IsPlayer()) {
        auto p = dynamic_cast<Player*>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    else if(IsSummon())
    {
        auto p = dynamic_cast<Summon*>(this);
        Messages::SendItemWearInfoMessage(p->GetMaster(), this, item);
    }
    return 0;
}

ushort Unit::Putoff(ItemWearType pos)
{
    if(pos == WEAR_TWOHAND)
        pos = WEAR_WEAPON;
    if(pos == WEAR_TWOFINGER_RING)
        pos = WEAR_RING;
    if(pos >= MAX_ITEM_WEAR || pos < 0)
        return TS_RESULT_NOT_ACTABLE;
    ItemWearType abspos = GetAbsoluteWearPos(pos);
    if(abspos == WEAR_CANTWEAR)
        return TS_RESULT_NOT_ACTABLE;
    if(pos != WEAR_BAG_SLOT)
        return putoffItem(abspos);

    // Todo: Bag

    return TS_RESULT_NOT_ACTABLE;
}

ItemWearType Unit::GetAbsoluteWearPos(ItemWearType pos)
{
    ItemWearType result = pos;
    if(m_anWear[pos] == nullptr)
        result = WEAR_CANTWEAR;
    return result;
}

ItemClass Unit::GetWeaponClass()
{
    ItemClass result = CLASS_ETC;
    auto itemRight = GetWornItem(WEAR_RIGHTHAND);

    if (itemRight != nullptr)
    {
        if(HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        {
            Item* itemLeft = GetWornItem(WEAR_LEFTHAND);
            if (itemRight->m_pItemBase->iclass == CLASS_ONEHAND_SWORD && itemLeft->m_pItemBase->iclass == CLASS_ONEHAND_SWORD)
                return CLASS_DOUBLE_SWORD;
            if (itemRight->m_pItemBase->iclass == CLASS_DAGGER && itemLeft->m_pItemBase->iclass == CLASS_DAGGER)
                return CLASS_DOUBLE_DAGGER;
            if (itemRight->m_pItemBase->iclass== CLASS_ONEHAND_AXE && itemLeft->m_pItemBase->iclass == CLASS_ONEHAND_AXE)
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
    if(item != nullptr)
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
    auto stateInfo = sObjectMgr->GetStateInfo(code);
    if (stateInfo == nullptr)
    {
        return TS_RESULT_NOT_EXIST;
    }
    if (GetHealth() == 0 && (stateInfo->state_time_type & 0x81) != 0)
        return TS_RESULT_NOT_ACTABLE;

    if ((stateInfo->state_time_type & 8) != 0 && IsMonster() && false /* Connector/AutoTrap */)
    {
        return TS_RESULT_LIMIT_TARGET;
    } /*else if (code != StateCode::SC_SLEEP && code != StateCode::SC_NIGHTMARE && code != StateCode::SC_SEAL
               && code != StateCode::SC_SHINE_WALL && code != StateCode::SC_STUN && stateInfo->effect_type != 104) {
        if (stateInfo->effect_type != 82 || stateInfo->value[0] == 0.0f && stateInfo->value[1] == 0.0f
                                            && stateInfo->value[2] == 0.0f && stateInfo->value[3] == 0.0f)

    }*/

    if (code == StateCode::SC_FEAR)
        ToggleFlag(UNIT_FIELD_STATUS, STATUS_MOVING_BY_FEAR);

    //auto pCaster = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(caster));
    auto pCaster     = sMemoryPool->GetObjectInWorld<Unit>(caster);
    int  base_damage = 0;
    if (pCaster != nullptr && stateInfo->base_effect_id > 0)
    {
        // DAMAGES WOHOOOOOOOO
    }

    bool                bNotErasable = ((stateInfo->state_time_type >> 4) & 1) != 0;
    std::vector<uint16> vDeleteStateUID{ };
    bool                bAlreadyExist{false};

    for (auto &s : m_vStateList)
    {
        if (code == s.m_nCode)
        {
            bAlreadyExist = true;
        }
        else
        {
            bool     bf = false;
            for (int i : stateInfo->duplicate_group)
            {
                if (s.IsDuplicatedGroup(i))
                {
                    bf = true;
                    break;
                }
            }
            if (!bf)
                continue;
        }
        if (bNotErasable != (((s.GetTimeType() >> 4) & 1) != 0))
        {
            if (bNotErasable)
                return TS_RESULT_ALREADY_EXIST;
            vDeleteStateUID.emplace_back(s.m_nUID);
        }
        else
        {
            if (s.GetLevel() > level)
                return TS_RESULT_ALREADY_EXIST;

            if (s.GetLevel() == level)
            {
                uint et = s.m_nEndTime[1];
                if (s.m_nEndTime[0] > et)
                    et = s.m_nEndTime[0];
                if (et > end_time)
                    return TS_RESULT_ALREADY_EXIST;
            }
            if (code != s.m_nCode)
                vDeleteStateUID.emplace_back(s.m_nUID);
        }
    }

    for (auto &id : vDeleteStateUID)
    {
        for (int i = (int)m_vStateList.size() - 1; i >= 0; --i)
        {
            State s = m_vStateList[i];
            if (id == s.m_nUID)
            {
                m_vStateList.erase(m_vStateList.begin() + i);
                CalculateStat();
                break;
            }
        }
    }
    if (bAlreadyExist)
    {
        for (auto &s : m_vStateList)
        {
            if (code == s.m_nCode)
            {
                s.AddState(type, caster, (uint16)level, start_time, end_time, base_damage, bIsAura);
                CalculateStat();
                onUpdateState(s, false);
                onAfterAddState(s);
                break;
            }
        }
    }
    else
    {
        m_nCurrentStateUID++;
        State ns{type, code, (int)m_nCurrentStateUID, caster, (uint16)level, start_time, end_time, base_damage, bIsAura, nStateValue, std::move(szStateValue)};
        m_vStateList.emplace_back(ns);
        CalculateStat();

        onUpdateState(ns, false);
        if (IsMonster() && !HasFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE))
        {
            if (m_Attribute.nAttackRange < 84)
                m_Attribute.nAttackRange = 83;
        }
        onAfterAddState(ns);
    }
    return TS_RESULT_SUCCESS;
}

void Unit::onAfterAddState(State)
{
    procMoveSpeedChange();
}

void Unit::procMoveSpeedChange()
{
    std::vector<Position> vMovePos{ };

    if (bIsMoving && IsInWorld())
    {
        uint ct  = sWorld->GetArTime();
        auto pos = GetCurrentPosition(ct);
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE))
        {
            if (speed != m_Attribute.nMoveSpeed / 7)
            {
                for (const auto& mi : ends)
                    vMovePos.emplace_back(mi.end);
                sWorld->SetMultipleMove(this, pos, vMovePos, (uint8)(m_Attribute.nMoveSpeed / 7), true, ct, true);
            }
        }
        else
        {
            sWorld->SetMove(this, pos, pos, 0, true, ct, true);
        }
    }
}

void Unit::onUpdateState(State state, bool bIsExpire)
{
    Messages::BroadcastStateMessage(this, state, bIsExpire);
}

uint16 Unit::onItemUseEffect(Unit *pCaster, Item* pItem, int type, float var1, float var2, const std::string &szParameter)
{
    uint16      result{TS_RESULT_ACCESS_DENIED};
    uint        target_handle{0};
    Position    pos{ };
    std::string error{ };
    uint        ct = sWorld->GetArTime();
    uint        prev_hp;
    uint        prev_mp;

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
            target_handle     = GetHandle();
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
            auto state = sObjectMgr->GetStateInfo(pItem->m_pItemBase->state_id);
            if (state == nullptr)
                return TS_RESULT_NOT_ACTABLE;
            if (state->effect_type != 200)
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
                    auto si = sObjectMgr->GetStateInfo(pItem->m_pItemBase->state_id);
                    if (si != nullptr && si->effect_type == 200)
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
                    auto nItemID    = (int)var1;
                    auto nItemCount = (int64)var2;
                    while (nItemID < 0)
                        sObjectMgr->SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
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
            error = string_format("Unit::onItemUseEffect [%d]: Unknown type %d !", pItem->m_Instance.Code, type);
            NG_LOG_ERROR("entites.unit", "%s", error.c_str());
            Messages::SendChatMessage(30, "@SYSTEM", dynamic_cast<Player *>(pCaster), error);
            result = TS_RESULT_UNKNOWN;
            break;
    }
    return result;
}

State *Unit::GetStateByEffectType(int effectType) const
{
    for (int i = 0; i < m_vStateList.size(); i++) {
        if (m_vStateList[i].GetEffectType() == effectType)
            return (State*)&m_vStateList[i];
    }
    return nullptr;
}

std::pair<float, int> Unit::GetHateMod(int nHateModType, bool bIsHarmful)
{
    float fAmpValue = 1.0f;
    int nIncValue = 0;

    for(auto& hm : m_vHateMod) {
        if(bIsHarmful) {
            if(!hm.bIsApplyToHarmful)
                continue;
        } else {
            if(!hm.bIsApplyToHelpful)
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
    bool bDeleted{false};
    for(int i = 0; i < m_vStateList.size(); i++) {
        uint et = m_vStateList[i].m_nEndTime[1];
        if(m_vStateList[i].m_nEndTime[0] > et)
            et = m_vStateList[i].m_nEndTime[0];
        if(et < t && !m_vStateList[i].m_bAura) {
            //RemoveState(it);
            Messages::BroadcastStateMessage(this, m_vStateList[i], true);
            m_vStateList.erase(m_vStateList.begin() + i);
            bDeleted = true;
        }
    }
    return bDeleted;
}

int Unit::GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad) const
{
    float v4{1};
    float v5 = m_Attribute.nAttackPointRight;

    // TODO: ElementalStateMod

    return (int)(v5 * v4);
}

DamageInfo Unit::DealMagicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealMagicalDamage(pFrom, (float)nDamage, elemental_type, 0, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    result.target_hp = GetHealth();
    return result;
}

DamageInfo Unit::DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealPhysicalDamage(pFrom, (float)nDamage, elemental_type, 0, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    result.target_hp = GetHealth();
    return result;
}

uint Unit::GetRemainCoolTime(int skill_id) const
{
    uint ct = sWorld->GetArTime();
    auto sk = GetSkill(skill_id);
    if(sk == nullptr || sk->m_nNextCoolTime < ct)
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
    if(jp < 0)
        jp = 0;
    SetInt32Value(UNIT_FIELD_JOBPOINT, jp);
    if(HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

void Unit::SetEXP(int64 exp)
{
    SetUInt64Value(UNIT_FIELD_EXP, exp);
    if(HasFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE))
        onExpChange();
}

bool Unit::IsWornByCode(int code) const
{
    for(auto& i : m_anWear)
    {
        if(i != nullptr && i->m_pItemBase != nullptr && i->m_pItemBase->id == code)
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
    if(/*IsMagicImmune()*/ false)
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

void Unit::applyPassiveSkillAmplifyEffect()
{

}

int Unit::GetArmorClass() const
{
    return m_anWear[WEAR_ARMOR] != nullptr ? m_anWear[WEAR_ARMOR]->m_pItemBase->iclass : 0;
}

void Unit::procStateDamage(uint t)
{
    std::vector<StateDamage> vDamageList{};
    for(auto& st : m_vStateList)
    {
        if(IsPlayer() || IsSummon())
        {
            auto caster = sMemoryPool->GetObjectInWorld<Unit>(st.m_hCaster[0]);
            if (caster == nullptr)
            {
                if (st.m_nCode != StateCode::SC_GAIA_MEMBER_SHIP
                    && st.m_nCode != StateCode::SC_NEMESIS
                    && st.m_nCode != StateCode::SC_NEMESIS_FOR_AUTO
                    && st.m_nCode != StateCode::SC_FALL_FROM_SUMMON
                    && st.IsHarmful())
                {
                    st.AddState(StateType::SG_NORMAL, st.m_hCaster[0], (uint16)st.m_nLevel[0], st.m_nStartTime[0], (uint)(t - 1), st.m_nBaseDamage[0], false);
                    onUpdateState(st, false);
                    continue;
                }
            }
        }

        bool bNeedToProcLightningForceCongestion = false;
        auto stateBase = sObjectMgr->GetStateInfo((int)st.m_nCode);
        if(stateBase == nullptr)
            continue;
        int nBaseEffectID = 0;
        auto nThisFireTime = (uint)(st.m_nLastProcessedTime + 100 * stateBase->fire_interval);
        if(nThisFireTime < t && nThisFireTime <= st.m_nEndTime[0])
        {
            if(st.m_nCode == StateCode::SC_LIGHTNING_FORCE_CONGESTION)
                bNeedToProcLightningForceCongestion = true;
            nBaseEffectID = stateBase->base_effect_id;
            if(nBaseEffectID <= 0)
                continue;

            int nDamageHP = 0;
            int nDamageMP = 0;
            auto elem = (ElementalType)stateBase->elemental_type;

            switch(nBaseEffectID)
            {
                case 1:
                case 2:
                case 3:
                case 4:
                case 11:
                    nDamageHP = (int)((stateBase->add_damage_per_skl * st.GetLevel())
                                      + (st.m_nBaseDamage[0] * (stateBase->amplify_base + (stateBase->amplify_per_skl * st.GetLevel())))
                                      + stateBase->add_damage_base);
                    break;
                case 6:
                    nDamageHP = (int)((st.GetValue(0) + (st.GetValue(1) * st.GetLevel())) * GetMaxHealth());
                    nDamageMP = (int)((st.GetValue(2) + (st.GetValue(3) * st.GetLevel())) * GetMaxMana());
                    break;
                case 12:
                    nDamageMP = (int)((stateBase->add_damage_per_skl * st.GetLevel())
                                      + (st.m_nBaseDamage[0] * (stateBase->amplify_base + (stateBase->amplify_per_skl * st.GetLevel())))
                                      + stateBase->add_damage_base);
                    break;
                case 21:
                    nDamageHP = (stateBase->add_damage_base + (stateBase->add_damage_per_skl * st.GetLevel()));
                    break;
                case 22:
                    nDamageMP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st.GetLevel());
                    break;
                case 24:
                    nDamageHP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st.GetLevel());
                    nDamageMP = stateBase->add_damage_base + (stateBase->add_damage_per_skl * st.GetLevel());
                    break;
                case 25:
                    nDamageHP = (int)((st.GetValue(0) + (st.GetValue(1) * st.GetLevel())) * GetMaxHealth());
                    nDamageMP = (int)((st.GetValue(3) + (st.GetValue(4) * st.GetLevel())) * GetMaxMana());
                    break;
                default:
                    break;
            }

            if(nDamageHP != 0 || nDamageMP != 0)
            {
                st.m_nLastProcessedTime = nThisFireTime;
                StateDamage sd{st.m_hCaster[0], elem, nBaseEffectID, (int)st.m_nCode, st.GetLevel(), nDamageHP, nDamageMP,
                               nThisFireTime + (100 * stateBase->fire_interval) > st.m_nEndTime[0], st.m_nUID};
                vDamageList.emplace_back(sd);
            }
        }
    }

    for(auto& sd : vDamageList)
    {
        auto caster = sMemoryPool->GetObjectInWorld<Unit>(sd.caster);
        int nFlag = 0;
        Damage dmg{};
        if(sd.base_effect_id < 11)
        {
            if(caster == nullptr)
            {
                RemoveState(sd.uid);
                continue;
            }

            switch(sd.base_effect_id)
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
            for(auto& st : m_vStateList)
            {
                if(st.m_nUID == sd.uid)
                {
                    auto stateBase = sObjectMgr->GetStateInfo((int)st.m_nCode);
                    if(stateBase == nullptr)
                        continue;
                    st.m_nTotalDamage += dmg.nDamage;
                    total_amount = stateBase->state_id;
                    break;
                }
            }

            XPacket statePct(TS_SC_STATE_RESULT);
            statePct << (uint)sd.caster;
            statePct << GetHandle();
            statePct << sd.code;
            statePct << sd.level;
            statePct << (uint16)1; // STATE_DAMAGE_HP
            statePct << dmg.nDamage;
            statePct << GetHealth();
            statePct << (uint8)(sd.final ? 1 : 0);
            statePct << total_amount;

            sWorld->Broadcast((uint)(GetPositionX() / g_nRegionSize),
                              (uint)(GetPositionY() / g_nRegionSize),
                              GetLayer(),
                              statePct);
        }
        else
        {
            int nHealHP = 0;
            int nHealMP = 0;

            switch(sd.base_effect_id)
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
            for(auto& st : m_vStateList)
            {
                if(st.m_nUID == sd.uid)
                {
                    int ad = nHealHP;
                    if(ad == 0)
                        ad = nHealMP;
                    if(ad != 0)
                    {
                        st.m_nTotalDamage += ad;
                        total_amount = st.m_nTotalDamage;
                    }
                    break;
                }
            }

            int df = 0;
            if(nHealHP != 0)
            {
                XPacket statePct(TS_SC_STATE_RESULT);
                statePct << (uint)sd.caster;
                statePct << GetHandle();
                statePct << sd.code;
                statePct << sd.level;
                statePct << (uint16)4; // STATE_HEAL
                statePct << nHealHP;
                statePct << GetHealth();
                statePct << (uint8)(sd.final ? 1 : 0);
                statePct << total_amount;

                sWorld->Broadcast((uint)(GetPositionX() / g_nRegionSize),
                                  (uint)(GetPositionY() / g_nRegionSize),
                                  GetLayer(),
                                  statePct);
            }

            if(nHealMP != 0)
            {
                df = df != 0 ? -1 : 0;
                df = total_amount & df;

                XPacket statePct(TS_SC_STATE_RESULT);
                statePct << (uint)sd.caster;
                statePct << GetHandle();
                statePct << sd.code;
                statePct << sd.level;
                statePct << (uint16)5; // STATE_HEAL_MP
                statePct << nHealMP;
                statePct << GetMana();
                statePct << (uint8)(sd.final ? 1 : 0);
                statePct << df;

                sWorld->Broadcast((uint)(GetPositionX() / g_nRegionSize),
                                  (uint)(GetPositionY() / g_nRegionSize),
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
    auto state = std::find_if(m_vStateList.begin(), m_vStateList.end(), [code, state_level](State s) { return s.m_nCode == code && s.GetLevel() <= state_level; });
    if (state != m_vStateList.end())
    {
        onUpdateState(*state, true);
        m_vStateList.erase(state);
        CalculateStat();
        onAfterAddState(*state); // @todo: onafterremovestate
    }
}

void Unit::RemoveState(int uid)
{
    for(int i = 0; i < m_vStateList.size(); ++i)
    {
        State s = m_vStateList[i];
        if(s.m_nUID == uid)
        {
            onUpdateState(s, true);
            m_vStateList.erase(m_vStateList.begin() + i);
            CalculateStat();
            onAfterAddState(s);
            return;
        }
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

bool Unit::IsMovable()
{
    if (GetHealth() == 0 || IsSitdown() || m_nMovableTime > sWorld->GetArTime() || m_castingSkill != nullptr)
        return false;
    else
        return HasFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
}

bool Unit::OnCompleteSkill()
{
    if(m_castingSkill != nullptr)
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
    if(pSkill != nullptr)
    {
        pSkill->m_nEnhance = (uint)pItem->m_Instance.nEnhance;
        pItem->SetBindTarget(this);
        Messages::SendSkillCardInfo(dynamic_cast<Player*>(this), pItem);
    }
}

void Unit::UnBindSkillCard(Item *pItem)
{
    Skill *pSkill = GetSkill(pItem->m_pItemBase->skill_id);
    if(pSkill != nullptr)
    {
        pSkill->m_nEnhance = 0;
        pItem->SetBindTarget(nullptr);
        Messages::SendSkillCardInfo(dynamic_cast<Player*>(this), pItem);
    }
}

bool Unit::IsEnemy(const Unit *pTarget, bool bIncludeHiding)
{
    return pTarget != nullptr && pTarget->IsInWorld()
           && (bIncludeHiding || IsVisible(pTarget))
           && !IsAlly(pTarget);
}

bool Unit::IsAlly(const Unit */*pTarget*/)
{
    return false;
}

bool Unit::IsVisible(const Unit *pTarget)
{
    return !pTarget->HasFlag(UNIT_FIELD_STATUS, STATUS_HIDING);
}

bool Unit::IsActiveAura(Skill *pSkill) const
{
    for(const auto& aura : m_vAura)
    {
        if(aura.first == pSkill->m_SkillBase->toggle_group)
        {
            return true;
        }
    }
    return false;
}

bool Unit::TurnOnAura(Skill *pSkill)
{
    if(pSkill == nullptr)
        return false;

    if(m_vAura.count(pSkill->m_SkillBase->toggle_group) != 0)
    {
        return false;
    }

    m_vAura[pSkill->m_SkillBase->toggle_group] = pSkill;
    AddState(SG_NORMAL, (StateCode)pSkill->m_SkillBase->state_id, GetHandle(), pSkill->m_SkillBase->GetStateLevel(pSkill->m_nSkillLevel, pSkill->GetSkillEnhance()), sWorld->GetArTime(), 0, true, 0, "");

    Messages::SendToggleInfo(this, pSkill->m_nSkillID, true);
    return true;
}

bool Unit::TurnOffAura(Skill *pSkill)
{
    if(pSkill == nullptr)
        return false;

    if(m_vAura.count(pSkill->m_SkillBase->toggle_group) == 0)
        return false;

    Messages::SendToggleInfo(this, pSkill->m_nSkillID, false);
    m_vAura.erase(pSkill->m_SkillBase->toggle_group);
    RemoveState((StateCode)pSkill->m_SkillBase->state_id, 255); // fuck this shit
    return true;
}

void Unit::ToggleAura(Skill *pSkill)
{
    bool bNewAura = m_vAura.count(pSkill->m_SkillBase->toggle_group) == 0;
    if(m_vAura.count(pSkill->m_SkillBase->toggle_group) != 0)
    {
        bNewAura = m_vAura[pSkill->m_SkillBase->toggle_group] != pSkill;
        TurnOffAura(m_vAura[pSkill->m_SkillBase->toggle_group]);
    }
    if(bNewAura)
        TurnOnAura(pSkill);
}

bool Unit::IsActable() const
{
    return GetHealth() != 0
           && !HasFlag(UNIT_FIELD_STATUS, STATUS_FEARED)
           && (GetUInt32Value(UNIT_FIELD_STATUS) & (STATUS_MOVABLE | STATUS_ATTACKABLE | STATUS_SKILL_CASTABLE | STATUS_MAGIC_CASTABLE | STATUS_ITEM_USABLE)) != 0;

}

int Unit::GetMoveSpeed()
{
    return (int)m_Attribute.nMoveSpeed;
}

State *Unit::GetState(StateCode code)
{
    auto var = std::find_if(m_vStateList.begin(), m_vStateList.end(), [&code](State s) { return s.m_nCode == code; });
    if (var != m_vStateList.end())
        return &*var; // iterator to State (*var), State to "pointer" (&var)
    return nullptr;
}
