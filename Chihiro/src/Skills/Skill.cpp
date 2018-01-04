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

Skill::Skill(Unit *pOwner, uint64 _uid, int _id) : m_nErrorCode(0)
{
    m_nSkillUID = _uid;
    m_nSkillID = _id;
    m_pOwner = pOwner;
    cool_time = 0;
    m_nSummonID = 0;
    m_nSkillLevel = 0;
    m_SkillBase = sObjectMgr->GetSkillBase(m_nSkillID);
}

void Skill::DB_InsertSkill(Unit *pUnit, uint skillUID, uint owner_uid, uint summon_uid, uint skill_id, uint skill_level)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SKILL);
    stmt->setInt32(0, skillUID);
    stmt->setInt32(1, owner_uid);
    stmt->setInt32(2, summon_uid);
    stmt->setInt32(3, skill_id);
    stmt->setInt32(4, skill_level);
    stmt->setInt32(5, 0); // cool_time
    CharacterDatabase.Execute(stmt);
}

void Skill::DB_UpdateSkill(Unit *pUnit, uint skill_uid, uint skill_level)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_SKILL);
    stmt->setInt32(0, skill_level);
    stmt->setInt32(1, 0); // cool_time
    auto uid = pUnit->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(2, uid);
    stmt->setInt32(3, skill_uid);
    CharacterDatabase.Execute(stmt);
}

int Skill::Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    auto current_time = sWorld->GetArTime();
    uint delay = 0xffffffff;

    if(current_time + this->cool_time > current_time)
        return TS_RESULT_COOL_TIME;

    if(m_SkillBase->effect_type == EffectType::ET_Summon || m_nSkillID == 4001) {
        m_nErrorCode = PrepareSummon(nSkillLevel, handle, pos, current_time);
    }
    auto m_nOriginalDelay = delay;
    if(delay == 0xffffffff) {
        delay = m_SkillBase->GetCastDelay(nSkillLevel,0);
        if(m_nSkillID > 0 || m_nSkillID < -5) {
            if(delay < 0)
                delay = (uint)(delay + 4294967296);
            delay = (uint)(delay / (m_pOwner->m_Attribute.nCastingSpeed / 100.0f));
            delay = (uint)((float)delay * (m_pOwner->GetCastingMod((ElementalType)m_SkillBase->elemental,
                    m_SkillBase->is_physical_act == 1, m_SkillBase->is_harmful != 0,
                    m_nOriginalDelay)));
        }
    }
    m_nCastingDelay = m_nOriginalDelay;
    m_hTarget = handle;
    m_nCastTime = current_time;
    m_nFireTime = current_time + delay;

    if(m_nErrorCode == TS_RESULT_SUCCESS) {
        m_nRequestedSkillLevel = (uint8)nSkillLevel;
        m_pOwner->m_castingSkill = this;
        broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                              (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                              0, 0, 1);
    } else {
        broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                              (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                              0, 0, 5);
    }

    return TS_RESULT_SUCCESS;
}

int Skill::PrepareSummon(int nSkillLevel, uint handle, Position pos, uint current_time)
{
    auto item = dynamic_cast<Item*>(sMemoryPool->getItemPtrFromId(handle));
    if(item == nullptr ||  item->m_pItemBase == nullptr
       || item->m_pItemBase->group != ItemGroup::SummonCard
       || item->m_Instance.OwnerHandle != m_pOwner->GetHandle()) {
        return TS_RESULT_NOT_ACTABLE;
    }
    auto player = dynamic_cast<Player*>(m_pOwner);
    if(player == nullptr)
        return TS_RESULT_NOT_ACTABLE;
    int i = 0;
    while(item->m_nHandle != player->m_aBindSummonCard[i]->m_nHandle) {
        ++i;
        if(i >= 6)
            return TS_RESULT_NOT_ACTABLE;
    }
    auto summon = item->m_pSummon;
    if(summon == nullptr)
        return TS_RESULT_NOT_EXIST;
    if(summon->IsInWorld())
        return TS_RESULT_NOT_ACTABLE;
    Position tmpPos = player->GetCurrentPosition(current_time);
    summon->SetCurrentXY(tmpPos.GetPositionX(), tmpPos.GetPositionY());
    summon->SetLayer(player->GetLayer());
    summon->StopMove();
    do {
        do {
            summon->AddNoise(rand32(), rand32(), 70);
            m_targetPosition = summon->GetCurrentPosition(current_time);
        } while(pos.GetPositionX() == m_targetPosition.GetPositionX() && pos.GetPositionY() == m_targetPosition.GetPositionY());
    } while(tmpPos.GetExactDist2d(&m_targetPosition) < 24.0f);
    return TS_RESULT_SUCCESS;
}

void Skill::assembleMessage(XPacket &pct, int nType, int cost_hp, int cost_mp)
{
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

    if(nType != SkillState::ST_Fire) {
        if(nType <= SkillState::ST_Fire)
            return;
        if(nType <= SkillState::ST_CastingUpdate || nType == SkillState::ST_Complete) {
            pct << (uint32)(m_nFireTime - m_nCastTime);
            pct << (uint16)m_nErrorCode;
            pct << (uint8)0;
            pct << (uint8)0;
            pct << (uint8)0;
            return;
        }
        if(nType == SkillState::ST_Cancel) {
            pct << (uint8)0;
            return;
        }
        if(nType != SkillState::ST_RegionFire)
            return;
    }

    pct << (uint8)0;
    pct << (float)0;
    pct << (uint8)0;
    pct << (uint8)0;
    pct << (uint16)0;
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

    if(m_SkillBase->id == 4001)
        DoSummon();
    else if(m_SkillBase->id == 4002)
        DoUnsummon();


    broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                          (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                          0, 0, 0);
    broadcastSkillMessage((int)(m_pOwner->GetPositionX() /g_nRegionSize),
                          (int)(m_pOwner->GetPositionY() / g_nRegionSize),m_pOwner->GetLayer(),
                          0, 0, 5);

}

void Skill::DoSummon()
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

void Skill::DoUnsummon()
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
    return true;
}
