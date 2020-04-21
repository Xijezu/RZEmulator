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
#include "ItemFields.h"
#include "StateBase.h"

enum Target
{
    ATTACKER = 0,
    ATTACKEE = 1,
};

enum DamageFlag
{
    IGNORE_AVOID = 0x2,
    IGNORE_DEFENCE = 0x4,
    IGNORE_BLOCK = 0x8,
    IGNORE_CRITICAL = 0x10,
};

struct HateModifier
{
    HateModifier(int32_t nHateModType, int32_t nHarmfulType, float _fAmpValue, int32_t _nIncValue)
        : fAmpValue(_fAmpValue), nIncValue(_nIncValue)
    {
        bIsApplyToPhysicalSkill = false;
        bIsApplyToMagicalSkill = false;
        bIsApplyToPhysicalAttack = false;

        bIsApplyToHarmful = false;
        bIsApplyToHelpful = false;

        switch (nHateModType)
        {
        case 1:
            bIsApplyToPhysicalSkill = true;
            break;
        case 2:
            bIsApplyToMagicalSkill = true;
            break;
        case 3:
            bIsApplyToPhysicalAttack = true;
            break;
        case 99:
            bIsApplyToPhysicalSkill = true;
            bIsApplyToMagicalSkill = true;
            bIsApplyToPhysicalAttack = true;
            break;
        }

        bIsApplyToHarmful = false;
        bIsApplyToHelpful = false;

        switch (nHarmfulType)
        {
        case 0:
            bIsApplyToHelpful = true;
            break;
        case 1:
            bIsApplyToHarmful = true;
            break;
        case 99:
            bIsApplyToHelpful = true;
            bIsApplyToHarmful = true;
            break;
        }
    }

    bool bIsApplyToPhysicalSkill;
    bool bIsApplyToMagicalSkill;
    bool bIsApplyToPhysicalAttack;

    bool bIsApplyToHarmful;
    bool bIsApplyToHelpful;

    float fAmpValue;
    int32_t nIncValue;
};

struct Damage
{
    // UserDefinedType:   _DAMAGE
    // Function       :     public void _DAMAGE()
    int32_t nDamage;
    int32_t nResistedDamage;
    bool bCritical;
    bool bMiss;
    bool bBlock;
    bool bPerfectBlock;
    int32_t target_hp;
};

struct DamageInfo : public Damage
{
    // Function       :     public void StructCreature::_DAMAGE_INFO::_DAMAGE_INFO()
    void SetDamage(const Damage &damage)
    {
        nDamage = damage.nDamage;
        bCritical = damage.bCritical;
        bMiss = damage.bMiss;
        bBlock = damage.bBlock;
        bPerfectBlock = damage.bPerfectBlock;
        target_hp = damage.target_hp;
    }

    uint16_t elemental_damage[7];
};

struct AttackInfo : public DamageInfo
{
    // Function       :     public void StructCreature::_ATTACK_INFO::_ATTACK_INFO()
    void SetDamageInfo(const DamageInfo &damage_info)
    {
        SetDamage(damage_info);
        elemental_damage[0] = damage_info.elemental_damage[0];
        elemental_damage[1] = damage_info.elemental_damage[1];
        elemental_damage[2] = damage_info.elemental_damage[2];
        elemental_damage[3] = damage_info.elemental_damage[3];
        elemental_damage[4] = damage_info.elemental_damage[4];
        elemental_damage[5] = damage_info.elemental_damage[5];
        elemental_damage[6] = damage_info.elemental_damage[6];
    }

    int16_t mp_damage;
    int16_t attacker_damage;
    int16_t attacker_mp_damage;
    uint16_t target_mp;
    uint32_t attacker_hp;
    uint16_t attacker_mp;
};

struct AdditionalDamageInfo
{
    AdditionalDamageInfo(uint8_t _ratio, ElementalType _require_type, ElementalType _type, uint16_t _nDamage, float _fDamage)
    {
        ratio = _ratio;
        require_type = _require_type;
        type = _type;
        nDamage = _nDamage;
        fDamage = _fDamage;
    }

    AdditionalDamageInfo(uint8_t _ratio, ElementalType _require_type, ElementalType _type, float _fDamage)
    {
        ratio = _ratio;
        require_type = _require_type;
        type = _type;
        nDamage = static_cast<uint16_t>(_fDamage);
        fDamage = _fDamage;
    }

    uint8_t ratio;
    ElementalType require_type;
    ElementalType type;
    uint16_t nDamage;
    float fDamage;
};

struct AddHPMPOnCriticalInfo
{
    AddHPMPOnCriticalInfo(int32_t _nAddHP, int32_t _nAddMP, int32_t _nActivationRate)
        : nAddHP(_nAddHP), nAddMP(_nAddMP), nActivationRate(_nActivationRate) {}

    int32_t nAddHP;
    int32_t nAddMP;
    int32_t nActivationRate;
};

struct DamageReflectInfo
{
    DamageReflectInfo(uint8_t _fire_ratio, float _range, ElementalType _type, unsigned short _nReflectDamage, float _fPhysicalReflectRatio, float _fPhysicalSkillReflectRatio, float _fMagicalReflectRatio, bool _bIgnoreDefence)
        : fire_ratio(_fire_ratio), range(_range), type(_type), nReflectDamage(_nReflectDamage), fPhysicalReflectRatio(_fPhysicalReflectRatio), fPhysicalSkillReflectRatio(_fPhysicalSkillReflectRatio), fMagicalReflectRatio(_fMagicalReflectRatio), bIgnoreDefence(_bIgnoreDefence)
    {
    }

    unsigned char fire_ratio;
    float range;
    ElementalType type;
    unsigned short nReflectDamage;
    float fPhysicalReflectRatio;
    float fPhysicalSkillReflectRatio;
    float fMagicalReflectRatio;
    bool bIgnoreDefence;
};

struct StateReflectInfo
{
    StateReflectInfo(StateCode _nCode, int32_t _nLevel, uint32_t _nDuration)
        : nCode(_nCode), nLevel(_nLevel), nDuration(_nDuration) {}

    StateCode nCode;
    int32_t nLevel;
    uint32_t nDuration;
};

struct _ADD_STATE_TAG
{
    _ADD_STATE_TAG(StateCode c, int32_t lv, int32_t p, uint32_t d, Target t, int32_t cm, int32_t min, int32_t max, int32_t tmin, int32_t tmax) : code(c), level(lv), percentage(p), duration(d), target(t), cost_mp(cm), min_hp(min), max_hp(max), target_min_hp(tmin), target_max_hp(tmax) {}
    StateCode code;
    int32_t level;
    int32_t percentage;
    uint32_t duration;
    Target target;
    int32_t cost_mp;
    int32_t min_hp;
    int32_t max_hp;
    int32_t target_min_hp;
    int32_t target_max_hp;
};

struct _HEAL_ON_ATTACK_TAG
{
    _HEAL_ON_ATTACK_TAG(int32_t _ratio, int32_t _hp_inc, int32_t _mp_inc) : ratio(_ratio), hp_inc(_hp_inc), mp_inc(_mp_inc) {}

    int32_t ratio;
    int32_t hp_inc;
    int32_t mp_inc;
};

struct _DAMAGE_ABSORB_TAG
{
    _DAMAGE_ABSORB_TAG(int32_t _ratio, float _hp_absorb_ratio, float _mp_absorb_ratio) : ratio(_ratio), hp_absorb_ratio(_hp_absorb_ratio), mp_absorb_ratio(_mp_absorb_ratio) {}

    int32_t ratio;
    float hp_absorb_ratio;
    float mp_absorb_ratio;
};

struct _STEAL_ON_ATTACK_TAG
{
    _STEAL_ON_ATTACK_TAG(int32_t _ratio, int32_t _hp_steal, int32_t _mp_steal) : ratio(_ratio), hp_steal(_hp_steal), mp_steal(_mp_steal) {}

    int32_t ratio;
    int32_t hp_steal;
    int32_t mp_steal;
};
