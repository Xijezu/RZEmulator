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

#include "State.h"
#include "World.h"
#include "ObjectMgr.h"
#include <algorithm>
#include "DatabaseEnv.h"

State::State(StateType type, StateCode code, int uid, uint caster, uint16 level, uint start_time, uint end_time, int base_damage, bool bIsAura, int nStateValue, std::string szStateValue)
{
    init(uid, (int)code);
    m_nCode = code;
    m_nLevel[(int)type] = level;
    m_hCaster[(int)type] = caster;
    m_nBaseDamage[(int)type] = base_damage;
    m_nStartTime[(int)type] = start_time;
    m_nEndTime[(int)type] = end_time;
    m_bAura = bIsAura;
    m_nLastProcessedTime = sWorld.GetArTime();
    m_nStateValue = nStateValue;
    m_szStateValue = std::move(szStateValue);
}

bool State::IsHolded()
{
    auto v1 = m_nRemainDuration[0];
    auto v2 = m_nRemainDuration[1];
    if(v1 <= v2)
        v1 = v2;
    return v1 != 0;
}

void State::ReleaseRemainDuration()
{

}

bool State::AddState(StateType type, uint caster, uint16 level, uint start_time, uint end_time, int base_damage, bool bIsAura)
{
    if(m_nLevel[(int)type] <= level) {
        m_nLevel[(int)type] = level;
        m_nEndTime[(int)type] = end_time;
        m_nBaseDamage[(int)type] = base_damage;
        m_hCaster[(int)type] = caster;
        m_nStartTime[(int)type] = start_time;
        m_nEndTime[(int)type] = end_time;
        m_bAura = bIsAura;
        m_nLastProcessedTime = start_time;
        return true;
    }
    return false;
}

uint16 State::GetLevel() const
{
    uint16 result = 0;
    if(m_nLevel[0] != 0 || m_nLevel[1] != 0)
        result = m_nLevel[0] + m_nLevel[1] + m_nLevel[2];
    return result;
}

int State::GetEffectType() const
{
    return m_pTemplate != nullptr ? m_pTemplate->effect_type : 0;
}

float State::GetValue(int idx) const
{
    return m_pTemplate != nullptr ? m_pTemplate->value[idx] : 0;
}

bool State::IsHarmful()
{
    return m_pTemplate != nullptr ? m_pTemplate->is_harmful != 0 : false;
}

bool State::IsDuplicatedGroup(int nGroupID)
{
    if (m_pTemplate == nullptr)
        return false;
    return (m_pTemplate->duplicate_group[0] != 0 && m_pTemplate->duplicate_group[0] == nGroupID)
           || (m_pTemplate->duplicate_group[1] != 0 && m_pTemplate->duplicate_group[1] == nGroupID)
           || (m_pTemplate->duplicate_group[2] != 0 && m_pTemplate->duplicate_group[2] == nGroupID);
}

void State::SetState(int code, int uid, uint caster, const uint16 *levels, const uint *durations, const int *remain_times, uint last_fire_time, const int *base_damage, int state_value, std::string szStateValue)
{
    uint t = sWorld.GetArTime();
    init(uid, code);

    for(int i = 0; i < 3; i++) {
        m_nLevel[i] = levels[i];
        m_hCaster[i] = caster;
        m_nBaseDamage[i] = base_damage[i];

        if(durations[i] != 0) {
            uint v = 0;
            if (t <= durations[i] - remain_times[i])
                v = 0;
            else
                v = (t + remain_times[i] - durations[i]);
            m_nStartTime[i] = v;
            m_nEndTime[i] = (uint)remain_times[i];
            if(m_nEndTime[i] != 0xffffffff)
                m_nEndTime[i] += t;
        } else {
            m_nEndTime[i] = 0;
            m_nStartTime[i] = 0;
        }
    }
    m_nLastProcessedTime = last_fire_time;
    m_nStateValue = state_value;
    m_szStateValue = std::move(szStateValue);
    m_nCode = (StateCode)code;
}

int State::GetTimeType() const
{
    return m_pTemplate != nullptr ? m_pTemplate->state_time_type : 0;
}

void State::init(int uid, int code)
{
    m_pTemplate = sObjectMgr.GetStateInfo(code);
    m_nUID = (uint16)uid;
}

void State::DB_InsertState(Unit *pOwner, State &pState)
{
    if(pState.m_bAura)
        return;
    uint ct = sWorld.GetArTime();
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_REP_STATE);
    stmt->setUInt64(0, sWorld.GetStateIndex());
    auto uid = pOwner->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(1, pOwner->IsPlayer() ? uid : 0);
    stmt->setInt32(2, pOwner->IsSummon() ? uid : 0);
    stmt->setInt32(3, pState.m_nCode);
    stmt->setInt16(4, pState.m_nLevel[0]);
    stmt->setInt16(5, pState.m_nLevel[1]);
    stmt->setInt16(6, pState.m_nLevel[2]);
    if(pState.m_nEndTime[0] <= ct)
        stmt->setInt32(7, 0);
    else
        stmt->setInt32(7, (int)(pState.m_nEndTime[0] - pState.m_nStartTime[0]));
    stmt->setInt32(8, pState.m_nStartTime[1]);
    stmt->setInt32(9, pState.m_nStartTime[2]);
    if(pState.m_nEndTime[0] == 0xffffffff)
        stmt->setInt32(10, -1);
    else
        stmt->setInt32(10, (int)std::max((int)(pState.m_nEndTime[0] - ct), 0));
    stmt->setInt32(11, pState.m_nEndTime[1]);
    stmt->setInt32(12, pState.m_nEndTime[2]);
    stmt->setInt32(13, pState.m_nBaseDamage[0]);
    stmt->setInt32(14, pState.m_nBaseDamage[1]);
    stmt->setInt32(15, pState.m_nBaseDamage[2]);
    auto si = sObjectMgr.GetStateInfo(pState.m_nCode);
    if(si == nullptr)
        return;
    stmt->setInt32(16, (int)(pState.m_nLastProcessedTime + (100 * (uint)si->fire_interval - ct)));
    stmt->setInt32(17, pState.m_nStateValue);
    stmt->setString(18, pState.m_szStateValue);
    stmt->setInt32(19, 0);
    CharacterDatabase.Execute(stmt);
}

void State::DB_ClearState(Unit *pOwner)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_STATE);
    auto uid = pOwner->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(0, pOwner->IsPlayer() ? uid : 0);
    stmt->setInt32(1, pOwner->IsSummon() ? uid : 0);
    CharacterDatabase.Execute(stmt);
}
