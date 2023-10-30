#pragma once
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

#include "Common.h"
#include "SkillBase.h"
#include "StateBase.h"
#include "Unit.h"

struct StateDamage {
    StateDamage(uint32_t _caster, ElementalType _type, int32_t _base_effect_id, int32_t _code, uint16_t _level, int32_t _damage_hp, int32_t _damage_mp, bool _final, uint16_t _uid)
    {
        caster = _caster;
        elementalType = _type;
        base_effect_id = _base_effect_id;
        code = _code;
        level = _level;
        damage_hp = _damage_hp;
        damage_mp = _damage_mp;
        final = _final;
        uid = _uid;
    }

    uint32_t caster;
    ElementalType elementalType;
    int32_t base_effect_id;
    int32_t code;
    uint16_t level;
    int32_t damage_hp;
    int32_t damage_mp;
    bool final;
    uint16_t uid;
};

class Unit;
class State : public Object {
public:
    static void DB_ClearState(Unit *pOwner);
    static void DB_InsertState(Unit *pOwner, State *pState);

    State() = default;
    ~State() = default;

    // Deleting the copy & assignment operators
    // Better safe than sorry
    State(const State &) = delete;
    State &operator=(const State &) = delete;

    State(StateType type, StateCode code, int32_t uid, uint32_t caster, uint16_t level, uint32_t start_time, uint32_t end_time, int32_t base_damage, bool bIsAura, int32_t nStateValue,
        std::string szStateValue);

    bool IsValid(uint32_t t) const;
    bool IsHolded();
    void ReleaseRemainDuration();
    void HoldRemainDuration();
    bool AddState(StateType type, uint32_t caster, uint16_t level, uint32_t start_time, uint32_t end_time, int32_t base_damage, bool bIsAura);
    void SetLevel(int32_t idx, uint16_t level) { m_nLevel[idx] = level; }
    int32_t GetEffectType() const;
    float GetValue(int32_t idx) const;
    bool IsHarmful();
    bool IsAura() const { return m_bAura; }
    bool IsDuplicatedGroup(int32_t nGroupID);
    void SetState(int32_t code, int32_t uid, uint32_t caster, const uint16_t levels[], const uint32_t durations[], const int32_t remain_times[], uint32_t last_fire_time, const int32_t base_damage[],
        int32_t state_value, std::string szStateValue);
    int32_t GetTimeType() const;

    bool ClearExpiredState(uint32_t t);

    inline StateCode GetCode() const { return m_nCode; }
    inline uint16_t GetUID() const { return m_nUID; }
    inline uint16_t GetLevel() const { return (m_nLevel[0] || m_nLevel[1]) ? m_nLevel[0] + m_nLevel[1] + m_nLevel[2] : 0; }
    inline uint16_t GetLevel(int32_t idx) const { return m_nLevel[idx]; }
    inline uint32_t GetStartTime() const { return (m_nStartTime[0] > m_nStartTime[1]) ? m_nStartTime[0] : m_nStartTime[1]; }
    inline uint32_t GetStartTime(int32_t type) const { return m_nStartTime[type]; }
    inline uint32_t GetEndTime() const { return (m_nEndTime[0] > m_nEndTime[1]) ? m_nEndTime[0] : m_nEndTime[1]; }
    inline uint32_t GetEndTime(int32_t type) const { return m_nEndTime[type]; }
    inline uint32_t GetRemainDuration() const { return (m_nRemainDuration[0] > m_nRemainDuration[1]) ? m_nRemainDuration[0] : m_nRemainDuration[1]; }
    inline uint32_t GetRemainDuration(int32_t type) const { return m_nRemainDuration[type]; }
    inline uint32_t GetCaster(int32_t type) const { return m_hCaster[type]; }
    inline uint32_t GetLastProcessedTime() const { return m_nLastProcessedTime; }
    inline uint32_t GetFireInterval() const { return m_pTemplate->fire_interval * 100; }

    uint16_t m_nUID;
    StateCode m_nCode;
    uint16_t m_nLevel[3]{};
    int32_t m_nBaseDamage[3]{};
    uint32_t m_hCaster[3]{};
    uint32_t m_nStartTime[3]{};
    uint32_t m_nEndTime[3]{};
    uint32_t m_nRemainDuration[3]{};
    uint32_t m_nLastProcessedTime;
    StateTemplate *m_pTemplate{nullptr};
    int32_t m_nTotalDamage;
    bool m_bAura;
    int32_t m_nStateValue;
    std::string m_szStateValue;
    bool m_bByEvent;

protected:
    void init(int32_t uid, int32_t code);
};