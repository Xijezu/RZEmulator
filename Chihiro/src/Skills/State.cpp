/*
  *  Copyright (C) 2018 Xijezu <http://xijezu.com/>
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
    m_nLastProcessedTime = sWorld->GetArTime();
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
    return m_pTemplate->duplicate_group[0] != 0 && m_pTemplate->duplicate_group[0] == nGroupID
           || m_pTemplate->duplicate_group[1] != 0 && m_pTemplate->duplicate_group[1] == nGroupID
           || m_pTemplate->duplicate_group[2] != 0 && m_pTemplate->duplicate_group[2] == nGroupID;
}

void State::SetState(StateCode code, int uid, uint caster, const uint16 *levels, const uint *durations, const int *remain_times, uint last_fire_time, const int *base_damage, int state_value, std::string szStateValue)
{
    uint t = sWorld->GetArTime();
    init(uid, (int)code);

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
}

int State::GetTimeType() const
{
    return m_pTemplate != nullptr ? m_pTemplate->state_time_type : 0;
}

void State::init(int uid, int code)
{
    m_pTemplate = sObjectMgr->GetStateInfo(code);
    m_nUID = (uint16)uid;
}
