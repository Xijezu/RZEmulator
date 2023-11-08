/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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

#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "World.h"

State::State(StateType type, StateCode code, int32_t uid, uint32_t caster, uint16_t level, uint32_t start_time, uint32_t end_time, int32_t base_damage, bool bIsAura, int32_t nStateValue,
    std::string szStateValue)
{
    init(uid, (int32_t)code);
    m_nCode = code;

    m_nLevel[0] = m_nLevel[1] = m_nLevel[2] = 0;
    m_hCaster[0] = m_hCaster[1] = m_hCaster[2] = 0;
    m_nStartTime[0] = m_nStartTime[1] = m_nStartTime[2] = 0;
    m_nEndTime[0] = m_nEndTime[1] = m_nEndTime[2] = 0;
    m_nRemainDuration[0] = m_nRemainDuration[1] = m_nRemainDuration[2] = 0;

    m_nLevel[type] = level;
    m_hCaster[type] = caster;
    m_nBaseDamage[type] = base_damage;
    m_nStartTime[type] = start_time;
    m_nEndTime[type] = end_time;
    m_bAura = bIsAura;

    m_nTotalDamage = 0;
    m_nLastProcessedTime = sWorld.GetArTime();

    m_nStateValue = nStateValue;
    m_szStateValue = std::move(szStateValue);
}

bool State::IsHolded()
{
    return GetRemainDuration() != 0;
}

void State::ReleaseRemainDuration()
{
    if (!IsHolded())
        return;

    auto t = sWorld.GetArTime();

    m_nEndTime[0] = t + m_nRemainDuration[0];
    m_nEndTime[1] = t + m_nRemainDuration[1];
    m_nEndTime[2] = t + m_nRemainDuration[2];

    m_nRemainDuration[0] = m_nRemainDuration[1] = m_nRemainDuration[2] = 0;

    m_bAura = false;
}

void State::HoldRemainDuration()
{
    if (IsHolded() || IsAura())
        return;

    auto t = sWorld.GetArTime();

    m_nRemainDuration[0] = m_nEndTime[0] - t;
    m_nRemainDuration[1] = m_nEndTime[1] - t;
    m_nRemainDuration[2] = m_nEndTime[2] - t;

    m_nEndTime[0] = m_nEndTime[1] = m_nEndTime[2] = -1;
    m_bAura = true;
}

bool State::AddState(StateType type, uint32_t caster, uint16_t level, uint32_t start_time, uint32_t end_time, int32_t base_damage, bool bIsAura)
{
    if (m_nLevel[type] > level)
        return false;

    m_nLevel[type] = level;
    m_nBaseDamage[type] = base_damage;
    m_nStartTime[type] = start_time;
    m_nEndTime[type] = end_time;
    m_hCaster[type] = caster;
    m_nLastProcessedTime = start_time;
    m_bAura = bIsAura;

    return true;
}

int32_t State::GetEffectType() const
{
    return m_pTemplate != nullptr ? m_pTemplate->effect_type : 0;
}

float_t State::GetValue(int32_t idx) const
{
    return m_pTemplate != nullptr ? m_pTemplate->value[idx] : 0;
}

bool State::IsHarmful()
{
    return m_pTemplate != nullptr ? m_pTemplate->is_harmful != 0 : false;
}

bool State::IsDuplicatedGroup(int32_t nGroupID)
{
    if (m_pTemplate == nullptr)
        return false;
    return ((m_pTemplate->duplicate_group[0] && m_pTemplate->duplicate_group[0] == nGroupID) || (m_pTemplate->duplicate_group[1] && m_pTemplate->duplicate_group[1] == nGroupID) ||
        (m_pTemplate->duplicate_group[2] && m_pTemplate->duplicate_group[2] == nGroupID));
}

void State::SetState(int32_t code, int32_t uid, uint32_t caster, const uint16_t *levels, const uint32_t *durations, const int32_t *remain_times, uint32_t last_fire_time, const int32_t *base_damage,
    int32_t state_value, std::string szStateValue)
{
    uint32_t t = sWorld.GetArTime();
    init(uid, code);
    m_nCode = (StateCode)code;

    for (int32_t i = 0; i < 3; i++) {
        m_nLevel[i] = levels[i];
        m_hCaster[i] = caster;
        m_nBaseDamage[i] = base_damage[i];

        if (durations[i]) {
            m_nStartTime[i] = t > (durations[i] - remain_times[i]) ? t - (durations[i] - remain_times[i]) : 0;
            m_nEndTime[i] = remain_times[i] == -1 ? -1 : t + remain_times[i];
        }
        else {
            m_nStartTime[i] = m_nEndTime[i] = 0;
        }
    }

    m_nLastProcessedTime = last_fire_time;
    m_nStateValue = state_value;
    m_szStateValue = std::move(szStateValue);
}

int32_t State::GetTimeType() const
{
    return m_pTemplate != nullptr ? m_pTemplate->state_time_type : 0;
}

bool State::ClearExpiredState(uint32_t t)
{
    bool bFlag = false;

    if (m_nLevel[0] && m_nEndTime[0] < t) {
        m_nLevel[0] = 0;
        bFlag = true;
    }
    if (m_nLevel[1] && m_nEndTime[1] < t) {
        m_nLevel[1] = 0;
        bFlag = true;
    }
    if ((m_nLevel[2] && m_nEndTime[2] < t) || (!m_nLevel[0] && !m_nLevel[1])) {
        m_nLevel[2] = 0;
        bFlag = true;
    }

    return bFlag;
}

bool State::IsValid(uint32_t t) const
{
    if (m_nLevel[0] && m_nEndTime[0] > t)
        return true;
    if (m_nLevel[1] && m_nEndTime[1] > t)
        return true;
    return false;
}

void State::init(int32_t uid, int32_t code)
{
    _mainType = MT_StaticObject;
    _subType = ST_State;
    _objType = OBJ_STATIC;

    _valuesCount = UNIT_FIELD_HANDLE + 1;
    _InitValues();

    m_pTemplate = sObjectMgr.GetStateInfo(code);
    m_nUID = (uint16_t)uid;
}

void State::DB_InsertState(Unit *pOwner, State *pState)
{
    if (pState->m_bAura)
        return;
    uint32_t ct = sWorld.GetArTime();
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_REP_STATE);
    stmt->setUInt64(0, sWorld.GetStateIndex());
    auto uid = pOwner->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(1, pOwner->IsPlayer() ? uid : 0);
    stmt->setInt32(2, pOwner->IsSummon() ? uid : 0);
    stmt->setInt32(3, pState->m_nCode);
    stmt->setInt16(4, pState->m_nLevel[0]);
    stmt->setInt16(5, pState->m_nLevel[1]);
    stmt->setInt16(6, pState->m_nLevel[2]);
    if (pState->m_nEndTime[0] <= ct)
        stmt->setInt32(7, 0);
    else
        stmt->setInt32(7, (int32_t)(pState->m_nEndTime[0] - pState->m_nStartTime[0]));
    stmt->setInt32(8, pState->m_nStartTime[1]);
    stmt->setInt32(9, pState->m_nStartTime[2]);
    if (pState->m_nEndTime[0] == 0xffffffff)
        stmt->setInt32(10, -1);
    else
        stmt->setInt32(10, (int32_t)std::max((int32_t)(pState->m_nEndTime[0] - ct), 0));
    stmt->setInt32(11, pState->m_nEndTime[1]);
    stmt->setInt32(12, pState->m_nEndTime[2]);
    stmt->setInt32(13, pState->m_nBaseDamage[0]);
    stmt->setInt32(14, pState->m_nBaseDamage[1]);
    stmt->setInt32(15, pState->m_nBaseDamage[2]);
    auto si = sObjectMgr.GetStateInfo(pState->m_nCode);
    if (si == nullptr)
        return;
    stmt->setInt32(16, (int32_t)(pState->m_nLastProcessedTime + (100 * (uint32_t)si->fire_interval - ct)));
    stmt->setInt32(17, pState->m_nStateValue);
    stmt->setString(18, pState->m_szStateValue);
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
