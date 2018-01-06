#ifndef PROJECT_STATE_H
#define PROJECT_STATE_H

#include "StateBase.h"
#include "SkillBase.h"

struct StateDamage {
    StateDamage(uint _caster, ElementalType _type, int _base_effect_id, int _code, uint16 _level, int _damage_hp, int _damage_mp, bool _final, uint16 _uid)
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

    uint caster;
    ElementalType elementalType;
    int base_effect_id;
    int code;
    uint16 level;
    int damage_hp;
    int damage_mp;
    bool final;
    uint16 uid;
};

class State {
public:
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
    void SetState(StateCode code, int uid, uint caster, const uint16 levels[], const uint durations[], const int remain_times[], uint last_fire_time, const int base_damage[], int state_value, std::string szStateValue);
    int GetTimeType() const;

    uint16 m_nUID;
    StateCode m_nCode;
    uint16 m_nLevel[3]{};
    int m_nBaseDamage[3]{};
    uint m_hCaster[3]{};
    uint m_nStartTime[3]{};
    uint m_nEndTime[3]{};
    uint m_nRemainDuration[3]{};
    uint m_nLastProcessedTime;
    StateTemplate* m_pTemplate{nullptr};
    int m_nTotalDamage;
    bool m_bAura;
    int m_nStateValue;
    std::string m_szStateValue;
    bool m_bByEvent;
protected:
    void init(int uid, int code);
};


#endif // PROJECT_STATE_H
