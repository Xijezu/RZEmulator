#pragma once
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
#include "StateBase.h"
#include "SkillBase.h"

struct StateDamage
{
    StateDamage(uint _caster, ElementalType _type, int _base_effect_id, int _code, uint16 _level, int _damage_hp, int _damage_mp, bool _final, uint16 _uid)
    {
        caster         = _caster;
        elementalType  = _type;
        base_effect_id = _base_effect_id;
        code           = _code;
        level          = _level;
        damage_hp      = _damage_hp;
        damage_mp      = _damage_mp;
        final          = _final;
        uid            = _uid;
    }

    uint          caster;
    ElementalType elementalType;
    int           base_effect_id;
    int           code;
    uint16        level;
    int           damage_hp;
    int           damage_mp;
    bool          final;
    uint16        uid;
};

class Unit;
class State
{
    public:
        static void DB_ClearState(Unit *pOwner);
        static void DB_InsertState(Unit *pOwner, State &pState);
        State() = default;
        ~State() = default;
        State(StateType type, StateCode code, int uid, uint caster, uint16 level, uint start_time, uint end_time, int base_damage, bool bIsAura, int nStateValue, std::string szStateValue);

        bool IsHolded();
        void ReleaseRemainDuration();
        bool AddState(StateType type, uint caster, uint16 level, uint start_time, uint end_time, int base_damage, bool bIsAura);
        uint16 GetLevel() const;
        int GetEffectType() const;
        float GetValue(int idx) const;
        bool IsHarmful();
        bool IsDuplicatedGroup(int nGroupID);
        void SetState(int code, int uid, uint caster, const uint16 levels[], const uint durations[], const int remain_times[], uint last_fire_time, const int base_damage[], int state_value, std::string szStateValue);
        int GetTimeType() const;

        uint16        m_nUID;
        StateCode     m_nCode;
        uint16        m_nLevel[3]{ };
        int           m_nBaseDamage[3]{ };
        uint          m_hCaster[3]{ };
        uint          m_nStartTime[3]{ };
        uint          m_nEndTime[3]{ };
        uint          m_nRemainDuration[3]{ };
        uint          m_nLastProcessedTime;
        StateTemplate *m_pTemplate{nullptr};
        int           m_nTotalDamage;
        bool          m_bAura;
        int           m_nStateValue;
        std::string   m_szStateValue;
        bool          m_bByEvent;
    protected:
        void init(int uid, int code);
};