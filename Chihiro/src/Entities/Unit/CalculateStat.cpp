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

#include "Unit.h"
#include "ObjectMgr.h"
#include "Item.h"
#include "Messages.h"
#include "Skill.h"
#include "Summon.h"
#include "Player.h"
#include "GameRule.h"
#include "StateBase.h"
#include "Log.h"

void Unit::CalculateStat()
{
    CreatureAtributeServer stateAttr{};
    CreatureStat stateStat{};

    auto prev_max_hp = GetMaxHealth();
    auto prev_max_mp = GetMaxMana();
    auto prev_hp = GetHealth();
    auto prev_mp = GetMana();

    SetInt32Value(UNIT_FIELD_MAX_ENERGY, 5);
    SetFloatValue(UNIT_FIELD_HP_REGEN_MOD, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_REGEN_MOD, 1.0f);

    SetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER, 1.0f);
    SetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER, 1.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO, 1.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_ITEM, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_ITEM, 1.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_REST, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_REST, 2.0f);

    SetInt32Value(UNIT_FIELD_MAX_HEALTH, 1);
    SetInt32Value(UNIT_FIELD_MAX_MANA, 1);

    SetInt32Value(UNIT_FIELD_ADDITIONAL_HEAL, 0);
    SetInt32Value(UNIT_FIELD_ADDITIONAL_MP_HEAL, 0);

    SetFlag(UNIT_FIELD_STATUS, (STATUS_ATTACKABLE | STATUS_SKILL_CASTABLE | STATUS_MOVABLE | STATUS_MAGIC_CASTABLE | STATUS_ITEM_USABLE | STATUS_MORTAL));
    RemoveFlag(UNIT_FIELD_STATUS, (STATUS_HIDING | STATUS_HAVOC_BURST | STATUS_FEARED | STATUS_FORM_CHANGED | STATUS_MOVE_SPEED_FIXED | STATUS_HP_REGEN_STOPPED | STATUS_MP_REGEN_STOPPED));

    m_cStatByState.Reset(0);
    m_StatAmplifier.Reset(0.0f);
    m_AttributeByState.Reset(0);
    m_AttributeAmplifier.Reset(0);
    m_Attribute.Reset(0);
    m_Resist.Reset(0);
    m_ResistAmplifier.Reset(0.0f);
    m_vNormalAdditionalDamage.clear();
    m_vRangeAdditionalDamage.clear();
    m_vMagicalSkillAdditionalDamage.clear();
    m_vPhysicalSkillAdditionalDamage.clear();

    auto statptr = GetBaseStat();
    CreatureStat basestat{};
    if (statptr != nullptr)
        basestat.Copy(*statptr);
    m_cStat.Copy(basestat);
    onBeforeCalculateStat();
    // checkAdditionalItemEffect(); -> Nonexistant in epic 4
    applyStatByItem();
    applyJobLevelBonus();
    stateStat.Copy(m_cStat);
    applyStatByState();
    m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);
    // TODO onApplyStat -> Beltslots, Summonstatamplify?
    // TODO amplifyStatByItem -> Nonexistant atm, used for set effects?
    stateStat.Copy(m_cStat);
    getAmplifiedStatByAmplifier(stateStat);
    amplifyStatByState();
    getAmplifiedStatByAmplifier(m_cStat);
    m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);
    // TODO onAfterApplyStat -> summonstatpenalty
    finalizeStat();
    float b1 = GetLevel();
    float fcm = GetFCM();
    SetMaxHealth((uint32_t)(GetMaxHealth() + (m_cStat.vital * 33.0f) + (b1 * 20.0f)));
    SetMaxMana((uint32_t)(GetMaxMana() + (m_cStat.intelligence * 33.0f) + (b1 * 20.0f)));
    calcAttribute(m_Attribute);
    // TODO onAfterCalcAttrivuteByStat -> Nonexistant, loop through passiveskills <- Beltskills? o0
    applyItemEffect();
    applyPassiveSkillEffect();
    stateAttr.Copy(m_Attribute);
    applyStateEffect();
    m_AttributeByState.nAttackPointRight += (m_Attribute.nAttackPointRight - stateAttr.nAttackPointRight);
    m_AttributeByState.nAttackPointLeft += (m_Attribute.nAttackPointLeft - stateAttr.nAttackPointLeft);
    m_AttributeByState.nCritical += (m_Attribute.nCritical - stateAttr.nCritical);
    m_AttributeByState.nDefence += (m_Attribute.nDefence - stateAttr.nDefence);
    m_AttributeByState.nBlockDefence += (m_Attribute.nBlockDefence - stateAttr.nBlockDefence);
    m_AttributeByState.nMagicPoint += (m_Attribute.nMagicPoint - stateAttr.nMagicPoint);
    m_AttributeByState.nMagicDefence += (m_Attribute.nMagicDefence - stateAttr.nMagicDefence);
    m_AttributeByState.nAccuracyRight += (m_Attribute.nAccuracyRight - stateAttr.nAccuracyRight);
    m_AttributeByState.nAccuracyLeft += (m_Attribute.nAccuracyLeft - stateAttr.nAccuracyLeft);
    m_AttributeByState.nMagicAccuracy += (m_Attribute.nMagicAccuracy - stateAttr.nMagicAccuracy);
    m_AttributeByState.nAvoid += (m_Attribute.nAvoid - stateAttr.nAvoid);
    m_AttributeByState.nMagicAvoid += (m_Attribute.nMagicAvoid - stateAttr.nMagicAvoid);
    m_AttributeByState.nBlockChance += (m_Attribute.nBlockChance - stateAttr.nBlockChance);
    m_AttributeByState.nMoveSpeed += (m_Attribute.nMoveSpeed - stateAttr.nMoveSpeed);
    m_AttributeByState.nAttackSpeedRight += (m_Attribute.nAttackSpeedRight - stateAttr.nAttackSpeedRight);
    m_AttributeByState.nAttackSpeedLeft += (m_Attribute.nAttackSpeedLeft - stateAttr.nAttackSpeedLeft);
    m_AttributeByState.nDoubleAttackRatio += (m_Attribute.nDoubleAttackRatio - stateAttr.nDoubleAttackRatio);
    m_AttributeByState.nAttackRange += (m_Attribute.nAttackRange - stateAttr.nAttackRange);
    m_AttributeByState.nMaxWeight += (m_Attribute.nMaxWeight - stateAttr.nMaxWeight);
    m_AttributeByState.nCastingSpeed += (m_Attribute.nCastingSpeed - stateAttr.nCastingSpeed);
    m_AttributeByState.nCoolTimeSpeed += (m_Attribute.nCoolTimeSpeed - stateAttr.nCoolTimeSpeed);
    m_AttributeByState.nHPRegenPercentage += (m_Attribute.nHPRegenPercentage - stateAttr.nHPRegenPercentage);
    m_AttributeByState.nHPRegenPoint += (m_Attribute.nHPRegenPoint - stateAttr.nHPRegenPoint);
    m_AttributeByState.nMPRegenPercentage += (m_Attribute.nMPRegenPercentage - stateAttr.nMPRegenPercentage);
    m_AttributeByState.nMPRegenPoint += (m_Attribute.nMPRegenPoint - stateAttr.nMPRegenPoint);
    applyPassiveSkillAmplifyEffect();
    onApplyAttributeAdjustment();
    stateAttr.Copy(m_Attribute);
    getAmplifiedAttributeByAmplifier(stateAttr);
    applyStateAmplifyEffect();
    getAmplifiedAttributeByAmplifier(m_Attribute);
    applyDoubeWeaponEffect();
    SetMaxHealth((uint32_t)GetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER) * GetMaxHealth());
    SetMaxMana((uint32_t)GetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER) * GetMaxMana());
    // TODO this.getAmplifiedResistByAmplifier(m_Resist);
    onCompleteCalculateStat();
    SetHealth(GetHealth());
    SetMana(GetMana());
    onModifyStatAndAttribute();
    if (IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana()))
    {
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
    }
    else
    {
        if (IsSummon() && !IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana()))
        {
            auto s = dynamic_cast<Summon *>(this);
            if (s != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
        }
    }
}

void Unit::amplifyStatByState()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{};

    for (auto &s : m_vStateList)
    {
        if (s->GetEffectType() == SEF_DECREASE_STATE_EFFECT)
        {
            auto nDecreaseLevel = static_cast<int32_t>(s->GetValue(0) + (s->GetValue(1) * s->GetLevel()));
            for (int i = 2; i < 11 && s->GetValue(i) != 0; ++i)
            {
                vDecreaseList.emplace_back(std::pair<int, int>((int)s->GetValue(i), (int)nDecreaseLevel));
            }
        }
    }
    for (auto &s : m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s->m_nLevel[i];

        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s->m_nCode)
            {
                int nLevel[3] = {0, 0, 0};
                for (int i = 2; i >= 0; --i)
                {
                    nLevel[i] = nOriginalLevel[i] - rp.second;
                    if (nLevel[i] < 0)
                    {
                        rp.second = -1 * nLevel[i];
                        nLevel[i] = 0;
                    }
                }

                s->SetLevel(SG_NORMAL, nLevel[0]);
                s->SetLevel(SG_DUPLICATE, nLevel[1]);
                s->SetLevel(SG_DEPENDENCE, nLevel[2]);

                break;
            }
        }

        if (s->GetLevel() > 0)
        {
            switch (s->GetEffectType())
            {
            case SEF_PARAMETER_AMP:
                // format is 0 = bitset, 1 = base, 2 = add per level
                ampParameter((uint)s->GetValue(0), (int)(s->GetValue(1) + (s->GetValue(2) * s->GetLevel())), true);
                ampParameter((uint)s->GetValue(3), (int)(s->GetValue(4) + (s->GetValue(5) * s->GetLevel())), true);
                ampParameter((uint)s->GetValue(12), (int)(s->GetValue(13) + (s->GetValue(14) * s->GetLevel())), true);
                ampParameter((uint)s->GetValue(15), (int)(s->GetValue(16) + (s->GetValue(17) * s->GetLevel())), true);
                break;
            default:
                break;
            }
        }

        s->SetLevel(SG_NORMAL, nOriginalLevel[0]);
        s->SetLevel(SG_DUPLICATE, nOriginalLevel[1]);
        s->SetLevel(SG_DEPENDENCE, nOriginalLevel[2]);
    }
}

void Unit::applyState(State &state)
{
    switch (state.GetEffectType())
    {
    case SEF_PARAMETER_INC:
        incParameter(state.GetValue(0), state.GetValue(1) + state.GetValue(2) * state.GetLevel(), false);
        incParameter(state.GetValue(3), state.GetValue(4) + state.GetValue(5) * state.GetLevel(), false);
        incParameter2(state.GetValue(6), state.GetValue(7) + state.GetValue(8) * state.GetLevel());
        incParameter2(state.GetValue(9), state.GetValue(10) + state.GetValue(11) * state.GetLevel());
        incParameter(state.GetValue(12), state.GetValue(13) + state.GetValue(14) * state.GetLevel(), false);
        incParameter(state.GetValue(15), state.GetValue(16) + state.GetValue(17) * state.GetLevel(), false);

        break;
    case SEF_PARAMETER_INC_WHEN_EQUIP_SHIELD:
        if (IsWearShield())
        {
            incParameter(state.GetValue(0), state.GetValue(1) + state.GetValue(2) * state.GetLevel(), false);
            incParameter(state.GetValue(3), state.GetValue(4) + state.GetValue(5) * state.GetLevel(), false);
            incParameter2(state.GetValue(6), state.GetValue(7) + state.GetValue(8) * state.GetLevel());
            incParameter2(state.GetValue(9), state.GetValue(10) + state.GetValue(11) * state.GetLevel());
        }
        break;
    case SEF_PARAMETER_AMP_WHEN_EQUIP_SHIELD:
        if (IsWearShield())
        {
            ampParameter(state.GetValue(0), state.GetValue(1) + state.GetValue(2) * state.GetLevel(), false);
            ampParameter(state.GetValue(3), state.GetValue(4) + state.GetValue(5) * state.GetLevel(), false);
            ampParameter2(state.GetValue(6), state.GetValue(7) + state.GetValue(8) * state.GetLevel());
            ampParameter2(state.GetValue(9), state.GetValue(10) + state.GetValue(11) * state.GetLevel());
        }
        break;
    case SEF_PARAMETER_INC_WHEN_EQUIP:
    case SEF_PARAMETER_AMP_WHEN_EQUIP:
    {
        auto weapon_class = GetWeaponClass();
        uint32_t nWeaponBitFlag{0};

        if (!weapon_class)
            break;

        switch (weapon_class)
        {
        case CLASS_ONEHAND_SWORD:
            nWeaponBitFlag = FLAG_EQUIP_ONEHAND_SWORD;
            break;
        case CLASS_TWOHAND_SWORD:
            nWeaponBitFlag = FLAG_EQUIP_TWOHAND_SWORD;
            break;
        case CLASS_DAGGER:
            nWeaponBitFlag = FLAG_EQUIP_DAGGER;
            break;
        case CLASS_TWOHAND_SPEAR:
            nWeaponBitFlag = FLAG_EQUIP_TWOHAND_SPEAR;
            break;
        case CLASS_TWOHAND_AXE:
            nWeaponBitFlag = FLAG_EQUIP_TWOHAND_AXE;
            break;
        case CLASS_ONEHAND_MACE:
            nWeaponBitFlag = FLAG_EQUIP_ONEHAND_MACE;
            break;
        case CLASS_TWOHAND_MACE:
            nWeaponBitFlag = FLAG_EQUIP_TWOHAND_MACE;
            break;
        case CLASS_HEAVY_BOW:
            nWeaponBitFlag = FLAG_EQUIP_HEAVY_BOW;
            break;
        case CLASS_LIGHT_BOW:
            nWeaponBitFlag = FLAG_EQUIP_LIGHT_BOW;
            break;
        case CLASS_CROSSBOW:
            nWeaponBitFlag = FLAG_EQUIP_CROSSBOW;
            break;
        case CLASS_ONEHAND_STAFF:
            nWeaponBitFlag = FLAG_EQUIP_ONEHAND_STAFF;
            break;
        case CLASS_TWOHAND_STAFF:
            nWeaponBitFlag = FLAG_EQUIP_TWOHAND_STAFF;
            break;
        case CLASS_DOUBLE_SWORD:
            nWeaponBitFlag = FLAG_EQUIP_DOUBLE_SWORD;
            break;
        case CLASS_DOUBLE_DAGGER:
            nWeaponBitFlag = FLAG_EQUIP_DOUBLE_DAGGER;
            break;
        default:
            break;
        }

        if (!(((uint32_t)state.GetValue(0)) & nWeaponBitFlag))
            break;

        if (state.GetEffectType() == SEF_PARAMETER_INC_WHEN_EQUIP)
        {
            incParameter(state.GetValue(1), state.GetValue(2) + state.GetLevel() * state.GetValue(3), (state.GetValue(1) <= FLAG_LUK) ? true : false);
            incParameter(state.GetValue(4), state.GetValue(5) + state.GetLevel() * state.GetValue(6), (state.GetValue(4) <= FLAG_LUK) ? true : false);
            incParameter2(state.GetValue(7), state.GetValue(8) + state.GetLevel() * state.GetValue(9));
        }
        else if (state.GetEffectType() == SEF_PARAMETER_AMP_WHEN_EQUIP)
        {
            ampParameter(state.GetValue(1), state.GetValue(2) + state.GetLevel() * state.GetValue(3), (state.GetValue(1) <= FLAG_LUK) ? true : false);
            ampParameter(state.GetValue(4), state.GetValue(5) + state.GetLevel() * state.GetValue(6), (state.GetValue(4) <= FLAG_LUK) ? true : false);
            ampParameter2(state.GetValue(7), state.GetValue(8) + state.GetLevel() * state.GetValue(9));
        }
    }
    break;

    case SEF_DOUBLE_ATTACK:
    {
        auto weapon_class = GetWeaponClass();
        if (weapon_class == 0)
            break;

        if (state.GetValue(8) != CLASS_EVERY_WEAPON)
        {
            if (weapon_class != state.GetValue(8) && weapon_class != state.GetValue(9) && weapon_class != state.GetValue(10) && weapon_class != state.GetValue(11))
                break;
        }

        m_Attribute.nDoubleAttackRatio += state.GetValue(0) + state.GetLevel() * state.GetValue(1);
    }
    break;

    case SEF_ADDITIONAL_DAMAGE_ON_ATTACK:
    {
        if (state.GetValue(11) == 0 || state.GetValue(11) == 99)
            m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), ElementalType::TYPE_NONE, (ElementalType)(int)state.GetValue(8), state.GetValue(0) + state.GetValue(1) * state.GetLevel(), 0));
        if (state.GetValue(11) == 1 || state.GetValue(11) == 99)
            m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), ElementalType::TYPE_NONE, (ElementalType)(int)state.GetValue(8), state.GetValue(0) + state.GetValue(1) * state.GetLevel(), 0));
    }
    break;

    case SEF_AMP_ADDITIONAL_DAMAGE_ON_ATTACK:
    {
        if (state.GetValue(11) == 0 || state.GetValue(11) == 99)
            m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), ElementalType::TYPE_NONE, (ElementalType)(int)state.GetValue(8), 0, state.GetValue(0) + state.GetValue(1) * state.GetLevel()));
        if (state.GetValue(11) == 1 || state.GetValue(11) == 99)
            m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), ElementalType::TYPE_NONE, (ElementalType)(int)state.GetValue(8), 0, state.GetValue(0) + state.GetValue(1) * state.GetLevel()));
    }
    break;

    case SEF_ADDITIONAL_DAMAGE_ON_SKILL:
    {
        if (state.GetValue(11) == 0 || state.GetValue(11) == 99)
            m_vPhysicalSkillAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), (ElementalType)(int)state.GetValue(5), (ElementalType)(int)state.GetValue(8), state.GetValue(0) + state.GetValue(1) * state.GetLevel(), 0));
        if (state.GetValue(11) == 1 || state.GetValue(11) == 99)
            m_vMagicalSkillAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), (ElementalType)(int)state.GetValue(5), (ElementalType)(int)state.GetValue(8), state.GetValue(0) + state.GetValue(1) * state.GetLevel(), 0));
    }
    break;

    case SEF_AMP_ADDTIONAL_DAMAGE_ON_SKILL:
    {
        if (state.GetValue(11) == 0 || state.GetValue(11) == 99)
            m_vPhysicalSkillAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), (ElementalType)(int)state.GetValue(5), (ElementalType)(int)state.GetValue(8), 0, state.GetValue(0) + state.GetValue(1) * state.GetLevel()));
        if (state.GetValue(11) == 1 || state.GetValue(11) == 99)
            m_vMagicalSkillAdditionalDamage.emplace_back(AdditionalDamageInfo(state.GetValue(6) + state.GetValue(7) * state.GetLevel(), (ElementalType)(int)state.GetValue(5), (ElementalType)(int)state.GetValue(8), 0, state.GetValue(0) + state.GetValue(1) * state.GetLevel()));
    }
    break;

    case SEF_ADD_STATE_ON_ATTACK_OLD:
    case SEF_ADD_STATE_ON_ATTACK:
    case SEF_ADD_STATE_BY_SELF_ON_ATTACK:
    case SEF_ADD_STATE_ON_BEING_ATTACKED:
    case SEF_ADD_STATE_BY_SELF_ON_BEING_ATTACKED:
    {
        if (state.GetValue(8) != CLASS_EVERY_WEAPON)
        {
            auto weapon_class = GetWeaponClass();
            if (weapon_class == 0)
                break;

            if (weapon_class != state.GetValue(8) && weapon_class != state.GetValue(9) && weapon_class != state.GetValue(10) && weapon_class != state.GetValue(11))
                break;
        }

        StateCode code = (StateCode)(int)state.GetValue(0);
        int level = state.GetValue(2) + state.GetLevel() * state.GetValue(3);
        int ratio = state.GetValue(6) + state.GetLevel() * state.GetValue(7);
        uint32_t duration = (state.GetValue(4) + state.GetLevel() * state.GetValue(5)) * 100;
        uint32_t attack_type = (uint32_t)state.GetValue(12);
        int cost_mp = (int)state.GetValue(13);
        int min = (int)state.GetValue(14);
        int max = (int)state.GetValue(15);
        int tmin = (int)state.GetValue(16);
        int tmax = (int)state.GetValue(17);

        if (state.GetEffectType() == SEF_ADD_STATE_ON_ATTACK_OLD)
        {
            m_vStateByNormalAttack.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, ATTACKEE, 0, 0, 0, 0, 0));
        }

        else if (state.GetEffectType() == SEF_ADD_STATE_ON_ATTACK || state.GetEffectType() == SEF_ADD_STATE_BY_SELF_ON_ATTACK)
        {
            auto eTarget = (state.GetEffectType() == SEF_ADD_STATE_ON_ATTACK) ? ATTACKEE : ATTACKER;

            if (attack_type == 0 || attack_type & STT_NORMAL_ATTACK)
            {
                m_vStateByNormalAttack.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
            }

            if (attack_type == 0 || attack_type & STT_PHYSICAL_SKILL)
            {
                if (attack_type == 0 || attack_type & STT_HELPFUL)
                {
                    m_vStateByHelpfulPhysicalSkill.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
                if (attack_type == 0 || attack_type & STT_HARMFUL)
                {
                    m_vStateByHarmfulPhysicalSkill.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
            }

            if (attack_type == 0 || attack_type & STT_MAGICAL_SKILL)
            {
                if (attack_type == 0 || attack_type & STT_HELPFUL)
                {
                    m_vStateByHelpfulMagicalSkill.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
                if (attack_type == 0 || attack_type & STT_HARMFUL)
                {
                    m_vStateByHarmfulMagicalSkill.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
            }
        }

        else if (state.GetEffectType() == SEF_ADD_STATE_ON_BEING_ATTACKED || state.GetEffectType() == SEF_ADD_STATE_BY_SELF_ON_BEING_ATTACKED)
        {
            auto eTarget = (state.GetEffectType() == SEF_ADD_STATE_ON_BEING_ATTACKED) ? ATTACKER : ATTACKEE;

            if (attack_type == 0 || attack_type & STT_NORMAL_ATTACK)
            {
                m_vStateByBeingNormalAttacked.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
            }

            if (attack_type == 0 || attack_type & STT_PHYSICAL_SKILL)
            {
                if (attack_type == 0 || attack_type & STT_HELPFUL)
                {
                    m_vStateByBeingHelpfulPhysicalSkilled.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
                if (attack_type == 0 || attack_type & STT_HARMFUL)
                {
                    m_vStateByBeingHarmfulPhysicalSkilled.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
            }

            if (attack_type == 0 || attack_type & STT_MAGICAL_SKILL)
            {
                if (attack_type == 0 || attack_type & STT_HELPFUL)
                {
                    m_vStateByBeingHelpfulMagicalSkilled.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
                if (attack_type == 0 || attack_type & STT_HARMFUL)
                {
                    m_vStateByBeingHarmfulMagicalSkilled.emplace_back(_ADD_STATE_TAG(code, level, ratio, duration, eTarget, cost_mp, min, max, tmin, tmax));
                }
            }
        }
    }
    break;

    case SEF_ADD_HP_ON_ATTACK:
    {

        if (state.GetValue(8) != CLASS_EVERY_WEAPON)
        {
            auto weapon_class = GetWeaponClass();

            if (weapon_class == 0)
                break;

            if (weapon_class != state.GetValue(8) && weapon_class != state.GetValue(9) && weapon_class != state.GetValue(10) && weapon_class != state.GetValue(11))
                break;
        }

        int nHPInc = state.GetValue(0) + state.GetLevel() * state.GetValue(1);
        int nMPInc = state.GetValue(2) + state.GetLevel() * state.GetValue(3);
        int nRatio = state.GetValue(6) + state.GetLevel() * state.GetValue(7);

        if (nHPInc || nMPInc)
        {
            m_vHealOnAttack.emplace_back(_HEAL_ON_ATTACK_TAG(nRatio, nHPInc, nMPInc));
        }
    }
    break;

    case SEF_ABSORB:
    {
        if (state.GetValue(8) != CLASS_EVERY_WEAPON)
        {
            auto weapon_class = GetWeaponClass();

            if (weapon_class == 0)
                break;

            if (weapon_class != state.GetValue(8) && weapon_class != state.GetValue(9) && weapon_class != state.GetValue(10) && weapon_class != state.GetValue(11))
                break;
        }

        float fHPAbsorbRatio = state.GetValue(0) + state.GetLevel() * state.GetValue(1);
        float fMPAbsorbRatio = state.GetValue(2) + state.GetLevel() * state.GetValue(3);
        int nRatio = state.GetValue(6) + state.GetLevel() * state.GetValue(7);

        if (fHPAbsorbRatio || fMPAbsorbRatio)
        {
            m_vAbsorbByNormalAttack.emplace_back(_DAMAGE_ABSORB_TAG(nRatio, fHPAbsorbRatio, fMPAbsorbRatio));
        }
    }
    break;

    case SEF_STEAL:
    case SEF_STEAL_WITH_REGEN_STOP:
    {
        if (state.GetValue(8) != CLASS_EVERY_WEAPON)
        {
            auto weapon_class = GetWeaponClass();

            if (weapon_class == 0)
                break;

            if (weapon_class != state.GetValue(8) && weapon_class != state.GetValue(9) && weapon_class != state.GetValue(10) && weapon_class != state.GetValue(11))
                break;
        }

        int nHPSteal = state.GetValue(0) + state.GetLevel() * state.GetValue(1);
        int nMPSteal = state.GetValue(2) + state.GetLevel() * state.GetValue(3);
        int nRatio = state.GetValue(6) + state.GetLevel() * state.GetValue(7);

        if (nHPSteal || nMPSteal)
        {
            m_vStealOnAttack.emplace_back(_STEAL_ON_ATTACK_TAG(nRatio, nHPSteal, nMPSteal));
        }

        if (state.GetEffectType() == SEF_STEAL_WITH_REGEN_STOP)
        {
            if (state.GetValue(12))
                SetFlag(UNIT_FIELD_STATUS, STATUS_HP_REGEN_STOPPED);
            if (state.GetValue(13))
                SetFlag(UNIT_FIELD_STATUS, STATUS_MP_REGEN_STOPPED);
        }
    }
    break;

    case SEF_DAMAGE_REFLECT_PERCENT:
    {
        m_vDamageReflectInfo.emplace_back(DamageReflectInfo(state.GetValue(6) + state.GetLevel() * state.GetValue(7), state.GetValue(9) * GameRule::DEFAULT_UNIT_SIZE,
                                                            (ElementalType)(int)state.GetValue(8), 0,
                                                            state.GetValue(0) + state.GetLevel() * state.GetValue(1),
                                                            state.GetValue(2) + state.GetLevel() * state.GetValue(3),
                                                            state.GetValue(4) + state.GetLevel() * state.GetValue(5),
                                                            state.GetValue(10)));
    }
    break;

    case SEF_DAMAGE_REFLECT:
    {
        m_vDamageReflectInfo.emplace_back(DamageReflectInfo(state.GetValue(6) + state.GetLevel() * state.GetValue(7), GameRule::REFLECT_RANGE,
                                                            (ElementalType)(int)state.GetValue(8), state.GetValue(0) + state.GetLevel() * state.GetValue(1),
                                                            0.0f,
                                                            0.0f,
                                                            0.0f,
                                                            state.GetValue(9)));
    }
    break;

    case SEF_DAMAGE_REFLECT_WHEN_EQUIP_SHIELD:
    {
        if (IsWearShield())
        {
            m_vStateReflectInfo.emplace_back(StateReflectInfo(static_cast<StateCode>((int)state.GetValue(0)), state.GetValue(3) + state.GetLevel() * state.GetValue(4), state.GetValue(1) + state.GetLevel() * state.GetValue(2)));
        }
    }
    break;

    case SEF_REGEN_ADD:
    {
        int nHPRegenAdd = state.GetValue(0) + state.GetLevel() * state.GetValue(1);
        int nMPRegenAdd = state.GetValue(2) + state.GetLevel() * state.GetValue(3);

        m_Attribute.nHPRegenPoint += nHPRegenAdd;
        m_Attribute.nMPRegenPoint += nMPRegenAdd;
    }
    break;

    case SEF_AMP_RECEIVE_DAMAGE:
    {
        int nApplyType = state.GetValue(0);
        if (nApplyType == 1)
        {
            m_NormalStatePenalty.fDamage += (state.GetValue(7) + state.GetLevel() * state.GetValue(8));
        }
        else if (nApplyType == 2)
        {
            m_RangeStatePenalty.fDamage += (state.GetValue(7) + state.GetLevel() * state.GetValue(8));
        }
        else if (nApplyType == 99)
        {
            m_NormalStatePenalty.fDamage += (state.GetValue(7) + state.GetLevel() * state.GetValue(8));
            m_RangeStatePenalty.fDamage += (state.GetValue(7) + state.GetLevel() * state.GetValue(8));
        }

        m_StateStatePenalty.fDamage += (state.GetValue(3) + state.GetLevel() * state.GetValue(4));
        m_PhysicalSkillStatePenalty.fDamage += (state.GetValue(5) + state.GetLevel() * state.GetValue(6));
        m_MagicalSkillStatePenalty.fDamage += (state.GetValue(9) + state.GetLevel() * state.GetValue(10));
    }
    break;

    case SEF_INC_HATE:
    {
        auto weapon_class = GetWeaponClass();

        if (weapon_class == 0)
            break;

        if (weapon_class != state.GetValue(6) && state.GetValue(6) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(7) && state.GetValue(7) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(8) && state.GetValue(8) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(9) && state.GetValue(9) != CLASS_EVERY_WEAPON)
            break;

        m_vHateMod.emplace_back(HateModifier(state.GetValue(10), state.GetValue(11), 0, state.GetValue(0) + state.GetValue(1) * state.GetLevel()));
    }
    break;

    case SEF_AMP_HATE:
    {
        auto weapon_class = GetWeaponClass();

        if (weapon_class == 0)
            break;

        if (weapon_class != state.GetValue(6) && state.GetValue(6) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(7) && state.GetValue(7) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(8) && state.GetValue(8) != CLASS_EVERY_WEAPON &&
            weapon_class != state.GetValue(9) && state.GetValue(9) != CLASS_EVERY_WEAPON)
            break;

        m_vHateMod.emplace_back(HateModifier(state.GetValue(10), state.GetValue(11), state.GetValue(0) + state.GetValue(1) * state.GetLevel(), 0));
    }
    break;

    case SEF_FORCE_CHIP:
    {
        float fNormalStatePenalty = (state.GetValue(0) + state.GetValue(1) * state.GetLevel());
        float fRangeStatePenalty = (state.GetValue(0) + state.GetValue(1) * state.GetLevel());
        float fPhysicalSkillStatePenalty = (state.GetValue(2) + state.GetValue(3) * state.GetLevel());

        if (IsPlayer() || IsSummon())
        {
            fNormalStatePenalty = fNormalStatePenalty * 0.25f;
            fRangeStatePenalty = fRangeStatePenalty * 0.25f;
            fPhysicalSkillStatePenalty = fPhysicalSkillStatePenalty * 0.25f;
        }

        m_NormalStatePenalty.fDamage += fNormalStatePenalty;
        m_RangeStatePenalty.fDamage += fRangeStatePenalty;
        m_PhysicalSkillStatePenalty.fDamage += fPhysicalSkillStatePenalty;
    }
    break;

    case SEF_SOUL_CHIP:
    {
        float fMagicalSkillStatePenalty = (state.GetValue(2) + state.GetLevel() * state.GetValue(3));

        if (IsPlayer() || IsSummon())
            fMagicalSkillStatePenalty = fMagicalSkillStatePenalty * 0.25f;

        m_MagicalSkillStatePenalty.fDamage += fMagicalSkillStatePenalty;
    }
    break;

    case SEF_HEALING_CHIP:
        SetFloatValue(UNIT_FIELD_HEAL_RATIO, GetFloatValue(UNIT_FIELD_HEAL_RATIO) + (state.GetValue(2) + state.GetLevel() * state.GetValue(3)));
        break;

    case SEF_LUNAR_CHIP:
    {
        float fNormalStatePenalty = (state.GetValue(0) + state.GetValue(1) * state.GetLevel());
        float fRangeStatePenalty = (state.GetValue(0) + state.GetValue(1) * state.GetLevel());
        float fPhysicalSkillStatePenalty = (state.GetValue(2) + state.GetValue(3) * state.GetLevel());
        float fMagicalSkillStatePenalty = (state.GetValue(2) + state.GetLevel() * state.GetValue(3));

        if (IsPlayer() || IsSummon())
        {
            fNormalStatePenalty = fNormalStatePenalty * 0.25f;
            fRangeStatePenalty = fRangeStatePenalty * 0.25f;
            fPhysicalSkillStatePenalty = fPhysicalSkillStatePenalty * 0.25f;
            fMagicalSkillStatePenalty = fMagicalSkillStatePenalty * 0.25f;
        }

        m_NormalStatePenalty.fDamage += fNormalStatePenalty;
        m_RangeStatePenalty.fDamage += fRangeStatePenalty;
        m_PhysicalSkillStatePenalty.fDamage += fPhysicalSkillStatePenalty;
        m_MagicalSkillStatePenalty.fDamage += fMagicalSkillStatePenalty;
    }
    break;

    case SEF_MP_COST_INC:
    {
        float fCostReduce = state.GetValue(0) + state.GetLevel() * state.GetValue(1);
        int nElementalType = state.GetValue(5);

        if (nElementalType != 99)
        {
            if (state.GetValue(10) == 99 || state.GetValue(10) == 1)
            {
                if (state.GetValue(11) == 99 || state.GetValue(11) == 0)
                {
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fManaCostRatio += fCostReduce;
                }

                if (state.GetValue(11) == 99 || state.GetValue(11) == 1)
                {
                    m_BadPhysicalElementalSkillStateMod[nElementalType].fManaCostRatio += fCostReduce;
                }
            }

            if (state.GetValue(10) == 99 || state.GetValue(10) == 2)
            {
                if (state.GetValue(11) == 99 || state.GetValue(11) == 0)
                {
                    m_GoodMagicalElementalSkillStateMod[nElementalType].fManaCostRatio += fCostReduce;
                }

                if (state.GetValue(11) == 99 || state.GetValue(11) == 1)
                {
                    m_BadMagicalElementalSkillStateMod[nElementalType].fManaCostRatio += fCostReduce;
                }
            }
        }
        else
        {
            for (int i = 0; i < ElementalType::TYPE_COUNT; ++i)
            {
                if (state.GetValue(10) == 99 || state.GetValue(10) == 1)
                {
                    if (state.GetValue(11) == 99 || state.GetValue(11) == 0)
                    {
                        m_GoodPhysicalElementalSkillStateMod[i].fManaCostRatio += fCostReduce;
                    }

                    if (state.GetValue(11) == 99 || state.GetValue(11) == 1)
                    {
                        m_BadPhysicalElementalSkillStateMod[i].fManaCostRatio += fCostReduce;
                    }
                }

                if (state.GetValue(10) == 99 || state.GetValue(10) == 2)
                {
                    if (state.GetValue(11) == 99 || state.GetValue(11) == 0)
                    {
                        m_GoodMagicalElementalSkillStateMod[i].fManaCostRatio += fCostReduce;
                    }

                    if (state.GetValue(11) == 99 || state.GetValue(11) == 1)
                    {
                        m_BadMagicalElementalSkillStateMod[i].fManaCostRatio += fCostReduce;
                    }
                }
            }
        }
    }
    break;

    case SEF_ADD_PARAMETER_ON_NORMAL_ATTACK:
    {
        if (state.GetValue(9) == 99 || state.GetValue(9) == 1)
        {
            m_RangeStateAdvantage.fDamage += state.GetValue(0) + state.GetValue(1) * state.GetLevel();
            m_RangeStateAdvantage.fCritical += state.GetValue(3) * state.GetLevel();
            m_RangeStateAdvantage.nCritical += state.GetValue(4) * state.GetLevel();
            m_RangeStateAdvantage.fHate += state.GetValue(5) + state.GetValue(6) * state.GetLevel();
        }

        if (state.GetValue(9) == 99 || state.GetValue(9) == 0)
        {
            m_NormalStateAdvantage.fDamage += state.GetValue(0) + state.GetValue(1) * state.GetLevel();
            m_NormalStateAdvantage.fCritical += state.GetValue(3) * state.GetLevel();
            m_NormalStateAdvantage.nCritical += state.GetValue(4) * state.GetLevel();
            m_NormalStateAdvantage.fHate += state.GetValue(6) * state.GetLevel();
        }
    }
    break;

    case SEF_ADD_PARAMETER_ON_SKILL:
    {
        float fDamage = state.GetValue(0) + state.GetValue(1) * state.GetLevel();
        float fMagicDamage = state.GetValue(2) + state.GetValue(3) * state.GetLevel();
        int nCritical = state.GetValue(4) * state.GetLevel();
        int nElementalType = state.GetValue(5);
        float fHate = state.GetValue(6) * state.GetLevel();
        float fCoolTime = state.GetValue(7) * state.GetLevel();
        bool bExhaustive = state.GetValue(8);
        int nApplySkillType = state.GetValue(9);
        int nApplyToHarmful = state.GetValue(10);
        uint32_t nCastingSpeedApplyTime = state.GetValue(11) * 100;
        int fCastingSpeed = state.GetValue(12) + state.GetValue(13) * state.GetLevel();

        if (nElementalType != 99)
        {
            if (nApplySkillType == 1 || nApplySkillType == 99)
            {
                if (nApplyToHarmful == 1 || nApplyToHarmful == 99)
                {
                    m_BadPhysicalElementalSkillStateMod[nElementalType].fPhysicalDamage += fDamage;
                    m_BadPhysicalElementalSkillStateMod[nElementalType].fMagicalDamage += fMagicDamage;
                    m_BadPhysicalElementalSkillStateMod[nElementalType].nCritical += nCritical;
                    m_BadPhysicalElementalSkillStateMod[nElementalType].fHate += fHate;
                    m_BadPhysicalElementalSkillStateMod[nElementalType].fCooltime += fCoolTime;

                    if (!m_BadPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime || m_BadPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                    {
                        m_BadPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                    }

                    m_BadPhysicalElementalSkillStateMod[nElementalType].fCastingSpeed += fCastingSpeed;

                    if (bExhaustive)
                        m_BadPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode.emplace_back(state.GetCode());
                }
                if (nApplyToHarmful == 0 || nApplyToHarmful == 99)
                {
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fPhysicalDamage += fDamage;
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fMagicalDamage += fMagicDamage;
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].nCritical += nCritical;
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fHate += fHate;
                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fCooltime += fCoolTime;

                    if (!m_GoodPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime || m_GoodPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                    {
                        m_GoodPhysicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                    }

                    m_GoodPhysicalElementalSkillStateMod[nElementalType].fCastingSpeed += fCastingSpeed;

                    if (bExhaustive)
                        m_GoodPhysicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode.emplace_back(state.GetCode());
                }
            }
            if (nApplySkillType == 2 || nApplySkillType == 99)
            {
                if (nApplyToHarmful == 1 || nApplyToHarmful == 99)
                {
                    m_BadMagicalElementalSkillStateMod[nElementalType].fPhysicalDamage += fDamage;
                    m_BadMagicalElementalSkillStateMod[nElementalType].fMagicalDamage += fMagicDamage;
                    m_BadMagicalElementalSkillStateMod[nElementalType].nCritical += nCritical;
                    m_BadMagicalElementalSkillStateMod[nElementalType].fHate += fHate;
                    m_BadMagicalElementalSkillStateMod[nElementalType].fCooltime += fCoolTime;

                    if (!m_BadMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime || m_BadMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                    {
                        m_BadMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                    }

                    m_BadMagicalElementalSkillStateMod[nElementalType].fCastingSpeed += fCastingSpeed;

                    if (bExhaustive)
                        m_BadMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode.emplace_back(state.GetCode());
                }
                if (nApplyToHarmful == 0 || nApplyToHarmful == 99)
                {
                    m_GoodMagicalElementalSkillStateMod[nElementalType].fPhysicalDamage += fDamage;
                    m_GoodMagicalElementalSkillStateMod[nElementalType].fMagicalDamage += fMagicDamage;
                    m_GoodMagicalElementalSkillStateMod[nElementalType].nCritical += nCritical;
                    m_GoodMagicalElementalSkillStateMod[nElementalType].fHate += fHate;
                    m_GoodMagicalElementalSkillStateMod[nElementalType].fCooltime += fCoolTime;

                    if (!m_GoodMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime || m_GoodMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                    {
                        m_GoodMagicalElementalSkillStateMod[nElementalType].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                    }

                    m_GoodMagicalElementalSkillStateMod[nElementalType].fCastingSpeed += fCastingSpeed;

                    if (bExhaustive)
                        m_GoodMagicalElementalSkillStateMod[nElementalType].vExhaustiveStateCode.emplace_back(state.GetCode());
                }
            }
        }
        else
        {
            for (int i = 0; i < ElementalType::TYPE_COUNT; ++i)
            {
                if (nApplySkillType == 1 || nApplySkillType == 99)
                {
                    if (nApplyToHarmful == 1 || nApplyToHarmful == 99)
                    {
                        m_BadPhysicalElementalSkillStateMod[i].fPhysicalDamage += fDamage;
                        m_BadPhysicalElementalSkillStateMod[i].fMagicalDamage += fMagicDamage;
                        m_BadPhysicalElementalSkillStateMod[i].nCritical += nCritical;
                        m_BadPhysicalElementalSkillStateMod[i].fHate += fHate;
                        m_BadPhysicalElementalSkillStateMod[i].fCooltime += fCoolTime;

                        if (!m_BadPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime || m_BadPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                        {
                            m_BadPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                        }

                        m_BadPhysicalElementalSkillStateMod[i].fCastingSpeed += fCastingSpeed;

                        if (bExhaustive)
                            m_BadPhysicalElementalSkillStateMod[i].vExhaustiveStateCode.emplace_back(state.GetCode());
                    }
                    if (nApplyToHarmful == 0 || nApplyToHarmful == 99)
                    {
                        m_GoodPhysicalElementalSkillStateMod[i].fPhysicalDamage += fDamage;
                        m_GoodPhysicalElementalSkillStateMod[i].fMagicalDamage += fMagicDamage;
                        m_GoodPhysicalElementalSkillStateMod[i].nCritical += nCritical;
                        m_GoodPhysicalElementalSkillStateMod[i].fHate += fHate;
                        m_GoodPhysicalElementalSkillStateMod[i].fCooltime += fCoolTime;

                        if (!m_GoodPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime || m_GoodPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                        {
                            m_GoodPhysicalElementalSkillStateMod[i].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                        }

                        m_GoodPhysicalElementalSkillStateMod[i].fCastingSpeed += fCastingSpeed;

                        if (bExhaustive)
                            m_BadPhysicalElementalSkillStateMod[i].vExhaustiveStateCode.emplace_back(state.GetCode());
                    }
                }
                if (nApplySkillType == 2 || nApplySkillType == 99)
                {
                    if (nApplyToHarmful == 1 || nApplyToHarmful == 99)
                    {
                        m_BadMagicalElementalSkillStateMod[i].fPhysicalDamage += fDamage;
                        m_BadMagicalElementalSkillStateMod[i].fMagicalDamage += fMagicDamage;
                        m_BadMagicalElementalSkillStateMod[i].nCritical += nCritical;
                        m_BadMagicalElementalSkillStateMod[i].fHate += fHate;
                        m_BadMagicalElementalSkillStateMod[i].fCooltime += fCoolTime;

                        if (!m_BadMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime || m_BadMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                        {
                            m_BadMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                        }

                        m_BadMagicalElementalSkillStateMod[i].fCastingSpeed += fCastingSpeed;

                        if (bExhaustive)
                            m_BadMagicalElementalSkillStateMod[i].vExhaustiveStateCode.emplace_back(state.GetCode());
                    }
                    if (nApplyToHarmful == 0 || nApplyToHarmful == 99)
                    {
                        m_GoodMagicalElementalSkillStateMod[i].fPhysicalDamage += fDamage;
                        m_GoodMagicalElementalSkillStateMod[i].fMagicalDamage += fMagicDamage;
                        m_GoodMagicalElementalSkillStateMod[i].nCritical += nCritical;
                        m_GoodMagicalElementalSkillStateMod[i].fHate += fHate;
                        m_GoodMagicalElementalSkillStateMod[i].fCooltime += fCoolTime;

                        if (!m_GoodMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime || m_GoodMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime > nCastingSpeedApplyTime)
                        {
                            m_GoodMagicalElementalSkillStateMod[i].nCastingSpeedApplyTime = nCastingSpeedApplyTime;
                        }

                        m_GoodMagicalElementalSkillStateMod[i].fCastingSpeed += fCastingSpeed;

                        if (bExhaustive)
                            m_GoodMagicalElementalSkillStateMod[i].vExhaustiveStateCode.emplace_back(state.GetCode());
                    }
                }
            }
        }
    }
    break;

    case SEF_MEZZ:
        if (state.GetValue(0) != 0)
        {
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ITEM_USABLE);
        }

        if (state.GetValue(1) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);

        if (state.GetValue(2) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);

        if (state.GetValue(3) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);

        if (state.GetValue(4) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);

        if (state.GetValue(5) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MORTAL);

        //m_fMaxHPAmplifier -= state.GetValue(8) + state.GetValue(9) * state.GetLevel();
        SetMaxHealth(GetMaxHealth() - state.GetValue(6) + state.GetValue(7) * state.GetLevel());

        //m_fMaxMPAmplifier -= state.GetValue(12) + state.GetValue(13) * state.GetLevel();
        SetMaxMana(GetMaxMana() - state.GetValue(10) + state.GetValue(11) * state.GetLevel());

        break;

    case SEF_TRANSFORMATION:
        if (state.GetValue(1) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);

        if (state.GetValue(2) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);

        if (state.GetValue(3) == 1)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);

        if (state.GetValue(3) == 2)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);

        if (state.GetValue(3) == 3)
        {
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);
        }

        if (state.GetValue(4) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MORTAL);

        if (state.GetValue(5) != 0)
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ITEM_USABLE);

        incParameter2(state.GetValue(6), state.GetValue(7) + state.GetValue(8) * state.GetLevel());
        incParameter2(state.GetValue(9), state.GetValue(10) + state.GetValue(11) * state.GetLevel());

        break;

    case SEF_ADD_HP_MP_ON_CRITICAL:
        m_vAddHPMPOnCritical.emplace_back(AddHPMPOnCriticalInfo(state.GetValue(0) + state.GetValue(1) * state.GetLevel(), state.GetValue(2) + state.GetValue(3) * state.GetLevel(), state.GetValue(4) + state.GetValue(5) * state.GetLevel()));
        break;

    case SEF_HEALING_AMPLIFY:
    {
        float fHPHealRatio = state.GetValue(0) + state.GetValue(1) * state.GetLevel();
        float fMPHealRatio = state.GetValue(2) + state.GetValue(3) * state.GetLevel();

        SetFloatValue(UNIT_FIELD_HEAL_RATIO, GetFloatValue(UNIT_FIELD_HEAL_RATIO) + fHPHealRatio);
        SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO, GetFloatValue(UNIT_FIELD_MP_HEAL_RATIO) + fMPHealRatio);

        SetInt32Value(UNIT_FIELD_ADDITIONAL_HEAL, GetInt32Value(UNIT_FIELD_ADDITIONAL_HEAL) + state.GetValue(4) + state.GetValue(5) * state.GetLevel());
        SetInt32Value(UNIT_FIELD_ADDITIONAL_MP_HEAL, GetInt32Value(UNIT_FIELD_ADDITIONAL_MP_HEAL) + state.GetValue(6) + state.GetValue(7) * state.GetLevel());
    }
    break;

    case SEF_DETECT_HIDE:
        SetFloatValue(UNIT_FIELD_HIDE_DETECTION_RANGE, GetFloatValue(UNIT_FIELD_HIDE_DETECTION_RANGE) + (state.GetValue(0) + state.GetLevel() * state.GetValue(1)) * GameRule::DEFAULT_UNIT_SIZE);
        break;

    case SEF_SKILL_INTERRUPTION:
        for (int i = 0; i < 12 && state.GetValue(i); ++i)
            m_vInterruptedSkill.emplace_back(state.GetValue(i));
        break;

    case SEF_DAMAGE_REDUCE_WITH_RACE_BY_PERCENT:
        m_vDamageReducePercentInfo.emplace_back(DamageReduceInfo(state.GetValue(6), state.GetValue(0) + state.GetLevel() * state.GetValue(1), state.GetValue(2) + state.GetLevel() * state.GetValue(3),
                                                                 state.GetValue(4) + state.GetLevel() * state.GetValue(5), state.GetValue(7), state.GetValue(8), state.GetValue(9), state.GetValue(10), state.GetValue(11)));
        break;

    case SEF_DAMAGE_REDUCE_WITH_RACE_BY_VALUE:
        m_vDamageReduceValueInfo.emplace_back(DamageReduceInfo(state.GetValue(6), state.GetValue(0) + state.GetLevel() * state.GetValue(1), state.GetValue(2) + state.GetLevel() * state.GetValue(3),
                                                               state.GetValue(4) + state.GetLevel() * state.GetValue(5), state.GetValue(7), state.GetValue(8), state.GetValue(9), state.GetValue(10), state.GetValue(11)));
        break;

    case SEF_MANA_SHIELD:
    {
        int nTargetType = state.GetValue(4);
        if (nTargetType == 1 || nTargetType == 99)
            SetFloatValue(UNIT_FIELD_PHYSICAL_MANASHIELD_ABSORB_RATIO, GetFloatValue(UNIT_FIELD_PHYSICAL_MANASHIELD_ABSORB_RATIO) + state.GetValue(0) + state.GetLevel() * state.GetValue(1));
        if (nTargetType == 2 || nTargetType == 99)
            SetFloatValue(UNIT_FIELD_MAGICAL_MANASHIELD_ABSORB_RATIO, GetFloatValue(UNIT_FIELD_MAGICAL_MANASHIELD_ABSORB_RATIO) + state.GetValue(0) + state.GetLevel() * state.GetValue(1));
    }
    break;

    case SEF_AMP_AND_INC_ITEM_CHANCE:
        m_Attribute.nItemChance += (state.GetValue(0) + state.GetValue(1) * state.GetLevel()) * 100;
        break;

    case SEF_MISC:
    {
        float fValue1 = state.GetValue(0) + state.GetValue(1) * state.GetLevel();
        float fValue2 = state.GetValue(2) + state.GetValue(3) * state.GetLevel();
        float fValue3 = state.GetValue(4) + state.GetValue(5) * state.GetLevel();
        float fValue4 = state.GetValue(6) + state.GetValue(7) * state.GetLevel();
        float fValue5 = state.GetValue(8) + state.GetValue(9) * state.GetLevel();
        float fValue6 = state.GetValue(10) + state.GetValue(11) * state.GetLevel();

        switch (state.GetCode())
        {
        case SC_NIGHTMARE:
            m_Attribute.nMagicDefence += fValue1;
            [[fallthrough]];
        case SC_STUN:
        case SC_SLEEP:
        case SC_SEAL:
        case SC_SHINE_WALL:
        case SC_FALL_FROM_SUMMON:
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ITEM_USABLE);
            [[fallthrough]];
        case SC_HOLD:
        case SC_EARTH_RESTRICTION:
        case SC_FROZEN_SNARE:
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
            break;
        case SC_STONECURSE:
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MORTAL);
            [[fallthrough]];
        case SC_STONECURSE_MORTAL:
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_HP_REGEN_STOPPED);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MP_REGEN_STOPPED);
            [[fallthrough]];
        case SC_FROZEN:
            m_AttributeAmplifier.fDefence += fValue1;
            m_AttributeAmplifier.fMagicDefence += fValue2;
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_SKILL_CASTABLE);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ITEM_USABLE);
            [[fallthrough]];
        case SC_MUTE:
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MAGIC_CASTABLE);
            break;
        case SC_FEAR:
            SetFlag(UNIT_FIELD_STATUS, STATUS_FEARED);
            break;
        case SC_PROTECTING_FORCE_OF_BEGINNING:
        {
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_MORTAL);
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);

            std::set<int> allowedSkillSet{};
            allowedSkillSet.emplace(4001);
            allowedSkillSet.emplace(4002);
            m_vAllowedSkill.emplace_back(allowedSkillSet);
        }
        break;
        case SC_HUNTING_CREATURE_CARD:
            //m_fCreatureCardChance += fValue1;
            NG_LOG_INFO("server.worldserver", "SC_HUNTING_CREATURE_CARD not implemented.");
            break;
        case SC_INC_BLOCK_CHANCE:
            m_Attribute.nBlockChance += fValue1;
            break;

        case SC_HIDE:
        case SC_TRACE_OF_FUGITIVE:
            SetFlag(UNIT_FIELD_STATUS, STATUS_HIDING);
            SetFlag(UNIT_FIELD_STATUS, STATUS_ATTACKABLE);
            m_AttributeAmplifier.fMoveSpeed += fValue1;
            break;
        case SC_HAVOC_BURST:
        {
            SetFlag(UNIT_FIELD_STATUS, STATUS_HAVOC_BURST);
            //m_nLastHavocUpdateTime = GetArTime();

            m_AttributeAmplifier.fAttackSpeedRight += fValue1;
            if (IsUsingDoubleWeapon())
                m_AttributeAmplifier.fAttackSpeedLeft += fValue1;
            m_AttributeAmplifier.fCastingSpeed += fValue2;
            m_AttributeAmplifier.fMoveSpeed += fValue3;
            break;
        }

        case SC_FRENZY:
        {
            m_AttributeAmplifier.fAttackSpeedRight += fValue1;

            if (IsUsingDoubleWeapon())
                m_AttributeAmplifier.fAttackSpeedLeft += fValue1;

            if (!IsPlayer() /*|| !static_cast<StructPlayer *>(this)->HasRidingState()*/)
            {
                m_AttributeAmplifier.fMoveSpeed += fValue2;
            }
            m_Attribute.nCritical = 100;

            break;
        }

        case SC_BURNING_STYLE:
        {
            m_AttributeAmplifier.fAttackPointRight += fValue1;

            if (IsUsingDoubleWeapon())
                m_AttributeAmplifier.fAttackPointLeft += fValue1;

            m_AttributeAmplifier.fCritical += fValue2;
            m_AttributeAmplifier.fDefence += fValue3;
            m_AttributeAmplifier.fAvoid += fValue4;
            SetFloatValue(UNIT_FIELD_HATE_RATIO, GetFloatValue(UNIT_FIELD_HATE_RATIO) + fValue5);
            break;
        }
        case SC_DUSK_STYLE:
        {
            m_AttributeAmplifier.fAccuracyRight += fValue1;

            if (IsUsingDoubleWeapon())
                m_AttributeAmplifier.fAccuracyRight += fValue1;

            m_AttributeAmplifier.fCastingSpeed += fValue2;
            //m_fMaxHPAmplifier += fValue3;
            SetFloatValue(UNIT_FIELD_HATE_RATIO, GetFloatValue(UNIT_FIELD_HATE_RATIO) + fValue4);
            break;
        }

        case SC_AGILE_STYLE:
        {
            m_AttributeAmplifier.fMoveSpeed += fValue1;
            m_AttributeAmplifier.fAvoid += fValue2;
            m_AttributeAmplifier.fAttackPointRight += fValue3;

            if (IsUsingDoubleWeapon())
                m_AttributeAmplifier.fAttackPointLeft += fValue3;
            break;
        }

        case SC_LIGHTNING_FORCE_CONGESTION:
        {
            m_AttributeAmplifier.fAttackPointRight += fValue1;
            //m_nAttackPointRightWithoutWeapon *= fValue1;
            m_AttributeAmplifier.fAttackSpeedRight += fValue2;
            if (IsUsingDoubleWeapon())
            {
                m_AttributeAmplifier.fAttackPointLeft += fValue1;
                //m_nAttackPointLeftWithoutWeapon *= fValue1;
                m_AttributeAmplifier.fAttackSpeedLeft += fValue2;
            }
            //m_fResistHarmfulState += fValue3;

            break;
        }
        default:
            break;
        }
    }
    }
}

void Unit::applyStateEffect()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{};
    for (auto &s : m_vStateList)
    {
        if (s->GetEffectType() == SEF_DECREASE_STATE_EFFECT)
        {
            auto nDecreaseLevel = (int)(s->GetValue(0) + (s->GetValue(1) * s->GetLevel()));
            for (int i = 2; i < 11 && s->GetValue(i) != 0; ++i)
            {
                vDecreaseList.emplace_back(std::pair<int, int>((int)s->GetValue(i), (int)nDecreaseLevel));
            }
        }
    }

    for (auto &s : m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s->GetLevel(i);
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s->m_nCode)
            {
                int nLevel[3] = {0, 0, 0};
                for (int i = 2; i >= 0; --i)
                {
                    nLevel[i] = nOriginalLevel[i] - rp.second;
                    if (nLevel[i] < 0)
                    {
                        rp.second = -1 * nLevel[i];
                        nLevel[i] = 0;
                    }
                }

                s->SetLevel(SG_NORMAL, nLevel[0]);
                s->SetLevel(SG_DUPLICATE, nLevel[1]);
                s->SetLevel(SG_DEPENDENCE, nLevel[2]);
                break;
            }
        }

        if (s->GetLevel() > 0)
            applyState(*s);

        for (int i = 0; i < 3; i++)
            s->SetLevel(i, nOriginalLevel[i]);
    }
}

void Unit::applyStatByState()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{};
    for (auto &s : m_vStateList)
    {
        if (s->GetEffectType() == SEF_DECREASE_STATE_EFFECT)
        {
            auto nDecreaseLevel = (int)(s->GetValue(0) + (s->GetValue(1) * s->GetLevel()));
            for (int i = 2; i < 11 && s->GetValue(i) != 0; ++i)
            {
                vDecreaseList.emplace_back(std::pair<int, int>(static_cast<int>(s->GetValue(i)), static_cast<int>(nDecreaseLevel)));
            }
        }
    }
    for (auto &s : m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s->GetLevel(i);

        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s->m_nCode)
            {
                int nLevel[3] = {0, 0, 0};
                for (int i = 2; i >= 0; --i)
                {
                    nLevel[i] = nOriginalLevel[i] - rp.second;
                    if (nLevel[i] < 0)
                    {
                        rp.second = -1 * nLevel[i];
                        nLevel[i] = 0;
                    }
                }

                s->SetLevel(SG_NORMAL, nLevel[0]);
                s->SetLevel(SG_DUPLICATE, nLevel[1]);
                s->SetLevel(SG_DEPENDENCE, nLevel[2]);
                break;
            }
        }

        if (s->GetLevel() > 0)
        {
            if (s->GetEffectType() == SEF_PARAMETER_INC)
            {
                // format is 0 = bitset, 1 = base, 2 = add per level
                incParameter((uint)s->GetValue(0), (int)(s->GetValue(1) + (s->GetValue(2) * s->GetLevel())), true);
                incParameter((uint)s->GetValue(3), (int)(s->GetValue(4) + (s->GetValue(5) * s->GetLevel())), true);
                incParameter((uint)s->GetValue(12), (int)(s->GetValue(13) + (s->GetValue(14) * s->GetLevel())), true);
                incParameter((uint)s->GetValue(15), (int)(s->GetValue(16) + (s->GetValue(17) * s->GetLevel())), true);
            }
            else if (s->GetEffectType() == SEF_PARAMETER_INC_WHEN_EQUIP_SHIELD && IsWearShield())
            {
                incParameter((uint)s->GetValue(0), (int)(s->GetValue(1) + (s->GetValue(2) + s->GetLevel())), true);
                incParameter((uint)s->GetValue(3), (int)(s->GetValue(4) + (s->GetValue(5) + s->GetLevel())), true);
            }
        }
        for (int i = 0; i < 3; i++)
            s->SetLevel(i, nOriginalLevel[i]);
    }
}

void Unit::applyItemEffect()
{
    Item *curItem = GetWornItem(WEAR_WEAPON);
    if (curItem != nullptr && curItem->m_pItemBase != nullptr)
        m_Attribute.nAttackRange = curItem->m_pItemBase->range;

    m_nUnitExpertLevel = 0;
    std::vector<int> ref_list{};

    for (int i = 0; i < MAX_ITEM_WEAR; i++)
    {
        curItem = GetWornItem((ItemWearType)i);
        if (curItem != nullptr && curItem->m_pItemBase != nullptr)
        {
            auto iwt = (ItemWearType)i;
            if (TranslateWearPosition(iwt, curItem, ref_list))
            {
                float fItemRatio = 1.0f;
                if (curItem->GetLevelLimit() > GetLevel() && curItem->GetLevelLimit() <= m_nUnitExpertLevel)
                    fItemRatio = 0.40000001f;

                for (int ol = 0; ol < MAX_OPTION_NUMBER; ol++)
                {
                    if (curItem->m_pItemBase->base_type[ol] != 0)
                    {
                        onItemWearEffect(curItem, true, curItem->m_pItemBase->base_type[ol], curItem->m_pItemBase->base_var[ol][0], curItem->m_pItemBase->base_var[ol][1], fItemRatio);
                    }
                }

                for (int ol = 0; ol < MAX_OPTION_NUMBER; ol++)
                {
                    if (curItem->m_pItemBase->opt_type[ol] != 0)
                    {
                        onItemWearEffect(curItem, false, curItem->m_pItemBase->opt_type[ol], curItem->m_pItemBase->opt_var[ol][0], curItem->m_pItemBase->opt_var[ol][1], fItemRatio);
                    }
                }

                float fAddPoint = 0.0f;
                float fTotalPoints = 0.0f;

                for (int ol = 0; ol < 2; ol++)
                {
                    if (curItem->m_pItemBase->enhance_id[ol] != 0)
                    {
                        int curEnhance = curItem->m_Instance.nEnhance;
                        int realEnhance = curEnhance;

                        if (realEnhance >= 1)
                        {
                            fTotalPoints = curItem->m_pItemBase->_enhance[0][ol] * curEnhance;

                            if (realEnhance > 4)
                            {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[1][ol] * (float)(realEnhance - 4));
                            }
                            if (realEnhance > 8)
                            {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[2][ol] * (float)(realEnhance - 8));
                            }
                            if (realEnhance > 12)
                            {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[3][ol] * (float)(realEnhance - 12));
                            }
                            onItemWearEffect(curItem, false, curItem->m_pItemBase->enhance_id[ol], fTotalPoints, fTotalPoints, fItemRatio);
                        }
                    }
                }
            }
        }
    }
}

void Unit::ampParameter2(uint nBitset, float fValue)
{

    if ((nBitset & 1) != 0)
    {
        m_ResistAmplifier.fResist[0] += fValue;
    }
    if ((nBitset & 2) != 0)
    {
        m_ResistAmplifier.fResist[1] += fValue;
    }
    if ((nBitset & 4) != 0)
    {
        m_ResistAmplifier.fResist[2] += fValue;
    }
    if ((nBitset & 8) != 0)
    {
        m_ResistAmplifier.fResist[3] += fValue;
    }
    if ((nBitset & 0x10) != 0)
    {
        m_ResistAmplifier.fResist[4] += fValue;
    }
    if ((nBitset & 0x20) != 0)
    {
        m_ResistAmplifier.fResist[5] += fValue;
    }
    if ((nBitset & 0x40) != 0)
    {
        m_ResistAmplifier.fResist[6] += fValue;
    }
    if ((nBitset & 0x200000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_NONE, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_NONE, fValue});
    }
    if ((nBitset & 0x400000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_FIRE, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_FIRE, fValue});
    }
    if ((nBitset & 0x800000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WATER, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WATER, fValue});
    }
    if ((nBitset & 0x1000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WIND, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WIND, fValue});
    }
    if ((nBitset & 0x2000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_EARTH, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_EARTH, fValue});
    }
    if ((nBitset & 0x4000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_LIGHT, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_LIGHT, fValue});
    }
    if ((nBitset & 0x8000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_DARK, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_DARK, fValue});
    }
    if ((nBitset & 0x10000000) != 0)
    {
        m_AttributeAmplifier.fCriticalPower += fValue;
    }
    if ((nBitset & 0x20000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, STATUS_HP_REGEN_STOPPED);
    if ((nBitset & 0x40000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, STATUS_MP_REGEN_STOPPED);
}

void Unit::ampParameter(uint nBitset, float fValue, bool bStat)
{

    if (bStat)
    {
        if ((nBitset & 1) != 0)
        {
            m_StatAmplifier.strength += fValue;
        }
        if ((nBitset & 2) != 0)
        {
            m_StatAmplifier.vital += fValue;
        }
        if ((nBitset & 4) != 0)
        {
            m_StatAmplifier.agility += fValue;
        }
        if ((nBitset & 8) != 0)
        {
            m_StatAmplifier.dexterity += fValue;
        }
        if ((nBitset & 0x10) != 0)
        {
            m_StatAmplifier.intelligence += fValue;
        }
        if ((nBitset & 0x20) != 0)
        {
            m_StatAmplifier.mentality += fValue;
        }
        if ((nBitset & 0x40) != 0)
        {
            m_StatAmplifier.luck += fValue;
        }
    }
    else
    {
        auto p = dynamic_cast<Player *>(this);
        if ((nBitset & 0x80) != 0)
        {
            m_AttributeAmplifier.fAttackPointRight += fValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            {
                m_AttributeAmplifier.fAttackPointLeft += fValue;
            }
        }
        if ((nBitset & 0x100) != 0)
        {
            m_AttributeAmplifier.fMagicPoint += fValue;
        }
        if ((nBitset & 0x200) != 0)
        {
            m_AttributeAmplifier.fDefence += fValue;
        }
        if ((nBitset & 0x400) != 0)
        {
            m_AttributeAmplifier.fMagicDefence += fValue;
        }
        if ((nBitset & 0x800) != 0)
        {
            m_AttributeAmplifier.fAttackSpeedRight += fValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            {
                m_AttributeAmplifier.fAttackSpeedLeft += fValue;
            }
        }
        if ((nBitset & 0x1000) != 0)
        {
            m_AttributeAmplifier.fCastingSpeed += fValue;
        }
        if ((nBitset & 0x2000) != 0)
        {
            if (p == nullptr || true /*|| p->m_nRidingStateUid == 0*/)
            {
                m_AttributeAmplifier.fMoveSpeed += fValue;
            }
        }
        if ((nBitset & 0x4000) != 0)
        {
            m_AttributeAmplifier.fAccuracyRight += fValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            {
                m_AttributeAmplifier.fAccuracyLeft += fValue;
            }
        }
        if ((nBitset & 0x8000) != 0)
        {
            m_AttributeAmplifier.fMagicAccuracy += fValue;
        }
        if ((nBitset & 0x10000) != 0)
        {
            m_AttributeAmplifier.fCritical += fValue;
        }
        if ((nBitset & 0x20000) != 0)
        {
            m_AttributeAmplifier.fBlockChance += fValue;
        }
        if ((nBitset & 0x40000) != 0)
        {
            m_AttributeAmplifier.fBlockDefence += fValue;
        }
        if ((nBitset & 0x100000) != 0)
        {
            m_AttributeAmplifier.fMagicAvoid += fValue;
        }
        /*if ((nBitset & 0x200000) != 0)
            m_fMaxHPAmplifier += fValue;

        if ((nBitset & 0x400000) != 0)
            m_fMaxMPAmplifier += fValue;*/
        if ((nBitset & 0x1000000) != 0)
        {
            m_AttributeAmplifier.fHPRegenPoint += fValue;
        }
        if ((nBitset & 0x2000000) != 0)
        {
            m_AttributeAmplifier.fMPRegenPoint += fValue;
        }
        if ((nBitset & 0x8000000) != 0)
        {
            m_AttributeAmplifier.fHPRegenPercentage += fValue;
        }
        if ((nBitset & 0x10000000) != 0)
        {
            m_AttributeAmplifier.fMPRegenPercentage += fValue;
        }
        if ((nBitset & 0x40000000) != 0)
        {
            m_AttributeAmplifier.fMaxWeight += fValue;
        }
    }
}

void Unit::incParameter(uint nBitset, float nValue, bool bStat)
{
    if (bStat)
    {
        if ((nBitset & 1) != 0)
            m_cStat.strength += nValue;
        if ((nBitset & 2) != 0)
            m_cStat.vital += nValue;
        if ((nBitset & 4) != 0)
            m_cStat.agility += nValue;
        if ((nBitset & 8) != 0)
            m_cStat.dexterity += nValue;
        if ((nBitset & 0x10) != 0)
            m_cStat.intelligence += nValue;
        if ((nBitset & 0x20) != 0)
            m_cStat.mentality += nValue;
        if ((nBitset & 0x40) != 0)
            m_cStat.luck += nValue;
    }
    else
    {
        if ((nBitset & 0x80) != 0)
        {
            m_Attribute.nAttackPointRight += nValue;
            //m_nAttackPointRightWithoutWeapon += (short)nValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            {
                m_Attribute.nAttackPointLeft += nValue;
                //this.m_nAttackPointLeftWithoutWeapon += (short)nValue;
            }
        }
        if ((nBitset & 0x100) != 0)
            m_Attribute.nMagicPoint += nValue;
        if ((nBitset & 0x200) != 0)
            m_Attribute.nDefence += nValue;
        if ((nBitset & 0x400) != 0)
            m_Attribute.nMagicDefence += nValue;
        if ((nBitset & 0x800) != 0)
        {
            m_Attribute.nAttackSpeedRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAttackSpeedLeft += nValue;
        }
        if ((nBitset & 0x1000) != 0)
            m_Attribute.nCastingSpeed += nValue;
        if ((nBitset & 0x2000) != 0 && !HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        {
            auto p = dynamic_cast<Player *>(this);
            if (p != nullptr /*|| p.m_nRidingStateUid == 0*/)
                m_Attribute.nMoveSpeed += nValue;
        }
        if ((nBitset & 0x4000) != 0)
        {
            m_Attribute.nAccuracyRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAccuracyLeft += nValue;
        }
        if ((nBitset & 0x8000) != 0)
            m_Attribute.nMagicAccuracy += nValue;
        if ((nBitset & 0x10000) != 0)
            m_Attribute.nCritical += (short)nValue;
        if ((nBitset & 0x20000) != 0)
            m_Attribute.nBlockChance += nValue;
        if ((nBitset & 0x40000) != 0)
            m_Attribute.nBlockDefence += nValue;
        if ((nBitset & 0x80000) != 0)
            m_Attribute.nAvoid += nValue;
        if ((nBitset & 0x100000) != 0)
            m_Attribute.nMagicAvoid += nValue;
        if ((nBitset & 0x200000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_HEALTH, GetInt32Value(UNIT_FIELD_MAX_HEALTH) + (int)nValue);
        if ((nBitset & 0x400000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_MANA, GetInt32Value(UNIT_FIELD_MAX_MANA) + (int)nValue);
        if ((nBitset & 0x1000000) != 0)
            m_Attribute.nHPRegenPoint += nValue;
        if ((nBitset & 0x2000000) != 0)
            m_Attribute.nMPRegenPoint += nValue;
        if ((nBitset & 0x8000000) != 0)
            m_Attribute.nHPRegenPercentage += nValue;
        if ((nBitset & 0x10000000) != 0)
            m_Attribute.nMPRegenPercentage += nValue;
        if ((nBitset & 0x40000000) != 0)
            m_Attribute.nMaxWeight += nValue;
    }
}

void Unit::incParameter2(uint nBitset, float fValue)
{

    if ((nBitset & 1) != 0)
    {
        m_Resist.nResist[0] += (short)fValue;
    }
    if ((nBitset & 2) != 0)
    {
        m_Resist.nResist[1] += (short)fValue;
    }
    if ((nBitset & 4) != 0)
    {
        m_Resist.nResist[2] += (short)fValue;
    }
    if ((nBitset & 8) != 0)
    {
        m_Resist.nResist[3] += (short)fValue;
    }
    if ((nBitset & 0x10) != 0)
    {
        m_Resist.nResist[4] += (short)fValue;
    }
    if ((nBitset & 0x20) != 0)
    {
        m_Resist.nResist[5] += (short)fValue;
    }
    if ((nBitset & 0x40) != 0)
    {
        m_Resist.nResist[6] += (short)fValue;
    }
    if ((nBitset & 0x200000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_NONE, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_NONE, fValue});
    }
    if ((nBitset & 0x400000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_FIRE, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_FIRE, fValue});
    }
    if ((nBitset & 0x800000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WATER, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WATER, fValue});
    }
    if ((nBitset & 0x1000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WIND, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_WIND, fValue});
    }
    if ((nBitset & 0x2000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_EARTH, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_EARTH, fValue});
    }
    if ((nBitset & 0x4000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_LIGHT, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_LIGHT, fValue});
    }
    if ((nBitset & 0x8000000) != 0)
    {
        m_vNormalAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_DARK, fValue});
        m_vRangeAdditionalDamage.emplace_back(AdditionalDamageInfo{100, TYPE_NONE, TYPE_DARK, fValue});
    }
    if ((nBitset & 0x10000000) != 0)
        m_Attribute.nCriticalPower += fValue;
    if ((nBitset & 0x20000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, STATUS_HP_REGEN_STOPPED);
    if ((nBitset & 0x40000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, STATUS_MP_REGEN_STOPPED);
}

void Unit::getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute)
{
    attribute.nCritical += (m_AttributeAmplifier.fCritical * attribute.nCritical);
    attribute.nCriticalPower += (m_AttributeAmplifier.fCriticalPower * attribute.nCriticalPower);
    attribute.nAttackPointRight += (m_AttributeAmplifier.fAttackPointRight * attribute.nAttackPointRight);
    attribute.nAttackPointLeft += (m_AttributeAmplifier.fAttackPointLeft * attribute.nAttackPointLeft);
    attribute.nDefence += (m_AttributeAmplifier.fDefence * attribute.nDefence);
    attribute.nBlockDefence += (m_AttributeAmplifier.fBlockDefence * attribute.nBlockDefence);
    attribute.nMagicPoint += (m_AttributeAmplifier.fMagicPoint * attribute.nMagicPoint);
    attribute.nMagicDefence += (m_AttributeAmplifier.fMagicDefence * attribute.nMagicDefence);
    attribute.nAccuracyRight += (m_AttributeAmplifier.fAccuracyRight * attribute.nAccuracyRight);
    attribute.nAccuracyLeft += (m_AttributeAmplifier.fAccuracyLeft * attribute.nAccuracyLeft);
    attribute.nMagicAccuracy += (m_AttributeAmplifier.fMagicAccuracy * attribute.nMagicAccuracy);
    attribute.nAvoid += (m_AttributeAmplifier.fAvoid * attribute.nAvoid);
    ;
    attribute.nMagicAvoid += (m_AttributeAmplifier.fMagicAvoid * attribute.nMagicAvoid);
    attribute.nBlockChance += (m_AttributeAmplifier.fBlockChance * attribute.nBlockChance);
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        attribute.nMoveSpeed += (m_AttributeAmplifier.fMoveSpeed * attribute.nMoveSpeed);

    attribute.nAttackSpeed += (m_AttributeAmplifier.fAttackSpeed * attribute.nAttackSpeed);
    attribute.nAttackRange += (m_AttributeAmplifier.fAttackRange * attribute.nAttackRange);
    attribute.nMaxWeight += (short)(m_AttributeAmplifier.fMaxWeight * attribute.nMaxWeight);
    attribute.nCastingSpeed += (short)(m_AttributeAmplifier.fCastingSpeed * attribute.nCastingSpeed);
    attribute.nCoolTimeSpeed += (m_AttributeAmplifier.fCoolTimeSpeed * attribute.nCoolTimeSpeed);
    attribute.nItemChance += (m_AttributeAmplifier.fItemChance * attribute.nItemChance);
    attribute.nHPRegenPercentage += (m_AttributeAmplifier.fHPRegenPercentage * attribute.nHPRegenPercentage);
    attribute.nHPRegenPoint += (m_AttributeAmplifier.fHPRegenPoint * attribute.nHPRegenPoint);
    attribute.nMPRegenPercentage += (m_AttributeAmplifier.fMPRegenPercentage * attribute.nMPRegenPercentage);
    attribute.nMPRegenPoint += (m_AttributeAmplifier.fMPRegenPoint * attribute.nMPRegenPoint);
    attribute.nAttackSpeedRight += (m_AttributeAmplifier.fAttackSpeedRight * attribute.nAttackSpeedRight);
    attribute.nAttackSpeedLeft += (m_AttributeAmplifier.fAttackSpeedLeft * attribute.nAttackSpeedLeft);
    attribute.nDoubleAttackRatio += (m_AttributeAmplifier.fDoubleAttackRatio * attribute.nDoubleAttackRatio);
    attribute.nStunResistance += (m_AttributeAmplifier.fStunResistance * attribute.nStunResistance);
    attribute.nMoveSpeedDecreaseResistance += (m_AttributeAmplifier.fMoveSpeedDecreaseResistance * attribute.nMoveSpeedDecreaseResistance);
    attribute.nHPAdd += (m_AttributeAmplifier.fHPAdd * attribute.nHPAdd);
    attribute.nMPAdd += (m_AttributeAmplifier.fMPAdd * attribute.nMPAdd);
    attribute.nHPAddByItem += (m_AttributeAmplifier.fHPAddByItem * attribute.nHPAddByItem);
    attribute.nMPAddByItem += (m_AttributeAmplifier.fMPAddByItem * attribute.nMPAddByItem);
}

void Unit::applyStateAmplifyEffect()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{};
    for (auto &s : m_vStateList)
    {
        if (s->GetEffectType() == 84)
        { // Strong Spirit?
            auto nDecreaseLevel = (int)(s->GetValue(0) + (s->GetValue(1) * s->GetLevel()));
            for (int i = 2; i < 11; ++i)
            {
                if (s->GetValue(i) == 0.0f)
                {
                    vDecreaseList.emplace_back(std::pair<int, int>((int)s->GetValue(1), (int)nDecreaseLevel));
                    break;
                }
            }
        }
    }
    for (auto &s : m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s->m_nLevel[i];
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s->m_nCode)
            {
                //                             *(v1 + 44) = 0;
                //                             *(v1 + 48) = 0;
                //                             *(v1 + 52) = 0;
                //                             *(v1 + 100) = 0;
                //                             do
                //                             {
                //                                 v17 = *(v1 + *(v1 + 100) + 40)
                //                                     - LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y);
                //                                 *(v1 + *(v1 + 100) + 52) = v17;
                //                                 if ( v17 < 0 )
                //                                 {
                //                                     LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y) = -v17;
                //                                     *(v1 + *(v1 + 100) + 52) = 0;
                //                                 }
                //                                 *(v1 + 100) -= 4;
                //                             }
                //                             while ( *(v1 + 100) >= -8 );
                //                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 44);
                //                             HIWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 48);
                //                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end_time) = *(v1 + 52);
                //                             break;
            }
        }

        if (s->GetLevel() != 0)
        {
            applyStateAmplify(s);
        }
        for (int i = 0; i < 3; i++)
            s->m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyStateAmplify(State *state)
{
    switch (state->GetEffectType())
    {
    case SEF_PARAMETER_AMP:
        ampParameter(state->GetValue(0), state->GetValue(1) + state->GetValue(2) * state->GetLevel(), false);
        ampParameter(state->GetValue(3), state->GetValue(4) + state->GetValue(5) * state->GetLevel(), false);
        ampParameter2(state->GetValue(6), state->GetValue(7) + state->GetValue(8) * state->GetLevel());
        ampParameter2(state->GetValue(9), state->GetValue(10) + state->GetValue(11) * state->GetLevel());
        ampParameter(state->GetValue(12), state->GetValue(13) + state->GetValue(14) * state->GetLevel(), false);
        ampParameter(state->GetValue(15), state->GetValue(16) + state->GetValue(17) * state->GetLevel(), false);
        break;
    case SEF_MISC:
    {
        switch (state->m_nCode)
        {
        case SC_SQUALL_OF_ARROW:
            if (state->GetValue(1) != 0)
                SetFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
            else
                RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);

            if (GetWeaponClass() == state->GetValue(0))
            {
                m_AttributeAmplifier.fAttackSpeedRight += (state->GetValue(2) + state->GetValue(3) * state->GetLevel());

                if (IsUsingDoubleWeapon())
                    m_AttributeAmplifier.fAttackSpeedLeft += (state->GetValue(2) + state->GetValue(3) * state->GetLevel());
            }
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
}

void Unit::onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio)
{
    float item_var_penalty = (type != IEP_ATTACK_SPEED) ? (var1 * fRatio) : var1;

    if (type != IEP_ATTACK_SPEED && bIsBaseVar && pItem != nullptr)
    {
        item_var_penalty += (float)(var2 * (float)(pItem->m_Instance.nLevel - 1));
        item_var_penalty = GameRule::GetItemValue(item_var_penalty, (int)var1, GetLevel(), pItem->GetItemRank(), pItem->m_Instance.nLevel);
    }

    /*if(type != IEP_ATTACK_SPEED)
        item_var_penalty *= m_fItemMod;*/

    Player *pPlayer{nullptr};
    if (IsPlayer())
        pPlayer = this->As<Player>();

    switch (type)
    {
    case IEP_MAGIC_POINT:
        m_Attribute.nMagicPoint += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_ATTACK_POINT:
    {
        bool bApplyRight{true};
        bool bApplyLeft{false};

        if (IsUsingDoubleWeapon())
        {
            if (pItem != nullptr && pItem->IsWeapon())
            {
                if (pItem->GetWearInfo() == ItemWearType::WEAR_SHIELD)
                {
                    bApplyLeft = true;
                    bApplyRight = false;
                }
            }
            else
            {
                bApplyLeft = true;
            }
        }

        if (bApplyLeft)
            m_Attribute.nAttackPointLeft += static_cast<int32_t>(item_var_penalty);
        if (bApplyRight)
            m_Attribute.nAttackPointRight += static_cast<int32_t>(item_var_penalty);
    }
    break;

    case IEP_BLOCK_DEFENCE:
        m_Attribute.nBlockDefence += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_DEFENCE:
        m_Attribute.nDefence += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_ACCURACY:
    {
        bool bApplyRight{true};
        bool bApplyLeft{false};

        if (IsUsingDoubleWeapon())
        {
            if (pItem != nullptr && pItem->IsWeapon())
            {
                if (pItem->GetWearInfo() == ItemWearType::WEAR_SHIELD)
                {
                    bApplyLeft = true;
                    bApplyRight = false;
                }
            }
            else
            {
                bApplyLeft = true;
            }
        }

        if (bApplyLeft)
            m_Attribute.nAccuracyLeft += static_cast<int32_t>(item_var_penalty);
        if (bApplyRight)
            m_Attribute.nAccuracyRight += static_cast<int32_t>(item_var_penalty);
    }
    break;

    case IEP_ATTACK_SPEED:
    {
        bool bApplyRight{true};
        bool bApplyLeft{false};

        if (IsUsingDoubleWeapon())
        {
            if (pItem != nullptr && pItem->IsWeapon())
            {
                if (pItem->GetWearInfo() == ItemWearType::WEAR_SHIELD)
                {
                    bApplyLeft = true;
                    bApplyRight = false;
                }
            }
            else
            {
                bApplyLeft = true;
            }
        }

        if (bApplyLeft)
            m_Attribute.nAttackSpeedLeft += item_var_penalty;
        if (bApplyRight)
            m_Attribute.nAttackPointRight += item_var_penalty;
    }
    break;

    case IEP_MAGIC_DEFENCE:
        m_Attribute.nMagicDefence += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_AVOID:
        m_Attribute.nAvoid += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_MOVE_SPEED:
        if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED) /*&& (!IsPlayer() || pPlayer->HasRidingState())*/)
            m_Attribute.nMoveSpeed += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_BLOCK_CHANCE:
        m_Attribute.nBlockChance += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_CARRY_WEIGHT:
        m_Attribute.nMaxWeight += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_CASTING_SPEED:
        m_Attribute.nCastingSpeed += item_var_penalty;
        break;
    case IEP_MAGIC_ACCURACY:
        m_Attribute.nMagicAccuracy += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_MAGIC_AVOID:
        m_Attribute.nMagicAvoid += item_var_penalty;
        break;
    case IEP_COOLTIME_SPEED:
        m_Attribute.nCoolTimeSpeed += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_MAX_HP:
        SetFlag(UNIT_FIELD_MAX_HEALTH, GetMaxHealth() + static_cast<int32_t>(item_var_penalty));
        break;
    case IEP_MAX_MP:
        SetFlag(UNIT_FIELD_MAX_MANA, GetMaxMana() + static_cast<int32_t>(item_var_penalty));
        break;
    case IEP_MP_REGEN_POINT:
        m_Attribute.nMPRegenPoint += static_cast<int32_t>(item_var_penalty);
        break;
    case IEP_BOW_INTERVAL:
        //@todo m_fBowInterval = var1;
        break;
    case IEP_INC_PARAMETER_A:
        incParameter(var1, var2, false);
        break;
    case IEP_INC_PARAMETER_B:
        incParameter2(var1, var2);
        break;
    case IEP_AMP_PARAMETER_A:
        ampParameter(var1, var2, false);
        break;
    case IEP_AMP_PARAMETER_B:
        ampParameter2(var1, var2);
        break;
    default:
        break;
    }
}

void Unit::calcAttribute(CreatureAtributeServer &attribute)
{
    attribute.nCriticalPower = 80;
    attribute.nCritical += ((m_cStat.luck * 0.2f) + 3.0f);

    float b1 = GetLevel();
    float fcm = GetFCM();
    float dl = (fcm * 5.0f);

    if (IsUsingBow() || IsUsingCrossBow())
    {
        attribute.nAttackPointRight += (1.2f * m_cStat.agility) + (2.2f * m_cStat.dexterity) + (fcm * b1);
    }
    else
    {
        attribute.nAttackPointRight += (2.0f * m_cStat.strength) * fcm + b1;
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
            attribute.nAttackPointLeft += (2.0f * m_cStat.strength) * fcm + b1;
    }

    attribute.nMagicPoint += ((2 * m_cStat.intelligence) * fcm + b1);
    attribute.nItemChance += (m_cStat.luck * 0.2000000029802322f);
    attribute.nDefence += ((2.0f * m_cStat.vital) * fcm + b1);
    attribute.nMagicDefence += ((2.0f * m_cStat.mentality) * fcm + b1);
    attribute.nMaxWeight += 10 * (GetLevel() + m_cStat.strength);
    attribute.nAccuracyRight += ((m_cStat.dexterity) * 0.5f * fcm + b1);
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        attribute.nAccuracyLeft += ((m_StatAmplifier.dexterity) * 0.5f * fcm + b1);
    attribute.nMagicAccuracy += ((m_cStat.mentality * 0.4f + m_cStat.dexterity * 0.1f) * fcm + b1);
    attribute.nAvoid += (m_cStat.agility * 0.5f * fcm + b1);
    attribute.nMagicAvoid += (m_cStat.mentality * 0.5f * fcm + b1);
    attribute.nAttackSpeedRight += 100; /*(100 + (m_cStat.agility * 0.1f));*/
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
        attribute.nAttackSpeedLeft += 100; /*((this->m_cStat.dexterity) * 0.5f + (fcm + b1));*/
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        attribute.nMoveSpeed += 120;
    attribute.nCastingSpeed += 100;
    attribute.nCoolTimeSpeed = 100;

    attribute.nHPRegenPercentage += dl;
    attribute.nHPRegenPoint += ((2 * b1) + 48.0f);
    attribute.nMPRegenPercentage += dl;
    attribute.nMPRegenPoint += ((2 * b1) + 48.0f); // + (4.1f * m_cStat.mentality));
    if (attribute.nAttackRange == 0)
        attribute.nAttackRange = 50;
};

void Unit::finalizeStat()
{
    if (m_cStat.strength < 0)
        m_cStat.strength = 0;

    if (m_cStat.vital < 0)
        m_cStat.vital = 0;

    if (m_cStat.dexterity < 0)
        m_cStat.dexterity = 0;

    if (m_cStat.agility < 0)
        m_cStat.agility = 0;

    if (m_cStat.intelligence < 0)
        m_cStat.intelligence = 0;

    if (m_cStat.mentality < 0)
        m_cStat.mentality = 0;

    if (m_cStat.luck < 0)
        m_cStat.luck = 0;
}

void Unit::getAmplifiedStatByAmplifier(CreatureStat &stat)
{
    stat.strength = (int)((m_StatAmplifier.strength * stat.strength) + stat.strength);
    stat.vital = (int)((m_StatAmplifier.vital * stat.vital) + stat.vital);
    stat.dexterity = (int)((m_StatAmplifier.dexterity * stat.dexterity) + stat.dexterity);
    stat.agility = (int)((m_StatAmplifier.agility * stat.agility) + stat.agility);
    stat.intelligence = (int)((m_StatAmplifier.intelligence * stat.intelligence) + stat.intelligence);
    stat.mentality = (int)((m_StatAmplifier.mentality * stat.mentality) + stat.mentality);
    stat.luck = (int)((m_StatAmplifier.luck * stat.luck) + stat.luck);
}

void Unit::applyStatByItem()
{
    std::vector<int> ref_list{};

    for (int i = 0; i < MAX_ITEM_WEAR; ++i)
    {
        if (m_anWear[i] == nullptr)
            continue;

        auto iwt = (ItemWearType)i;
        if (!TranslateWearPosition(iwt, m_anWear[i], ref_list))
            continue;

        const auto base = m_anWear[i]->GetItemBase();

        for (int x = 0; x < MAX_OPTION_NUMBER; ++x)
        {
            if (base->opt_type[x] == 0)
                continue;
            if (base->opt_type[x] == IEP_INC_PARAMETER_A)
                incParameter(base->opt_var[x][0], base->opt_var[x][1], true);
        }

        //_applyStatByEffect(pvEffectList)

        if (base->socket == 0)
            return;

        for (const auto &x : m_anWear[i]->m_Instance.Socket)
        {
            if (x == 0)
                continue;

            auto SocketBase = sObjectMgr.GetItemBase(x);
            if (SocketBase == nullptr)
                continue;

            if (GetLevel() < SocketBase->use_min_level || GetLevel() > SocketBase->use_max_level)
                continue;

            if (m_anWear[i]->GetCurrentEndurance() == 0)
                continue;

            for (int k = 0; k < MAX_OPTION_NUMBER; ++k)
            {
                if (SocketBase->opt_type[k] == 0)
                    continue;
                if (SocketBase->opt_type[k] == IEP_INC_PARAMETER_A)
                    incParameter(SocketBase->opt_var[k][0], SocketBase->opt_var[k][1], true);
            }

            //_applyStatByEffect(SocketBase.pvEffectList);
        }
    }
}

void Unit::applyPassiveSkillEffect(Skill *pSkill)
{
    switch (pSkill->GetSkillBase()->GetSkillEffectType())
    {
    case EF_WEAPON_MASTERY:
    {
        auto *pWeapon = GetWornItem(WEAR_WEAPON);
        if (pSkill->GetSkillId() == SKILL_ADV_WEAPON_EXPERT)
        {
            if (!pWeapon || pWeapon->GetItemRank() < 2)
                break;
        }

        int nAP = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        int nSpeed = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();
        int nAccuracy = pSkill->GetVar(4) + pSkill->GetVar(5) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nAttackPointRight += nAP;
        m_Attribute.nAttackSpeedRight += nSpeed;
        m_Attribute.nAccuracyRight += nAccuracy;

        if (IsUsingDoubleWeapon())
        {
            m_Attribute.nAttackPointLeft += nAP;
            m_Attribute.nAttackSpeedLeft += nSpeed;
            m_Attribute.nAccuracyLeft += nAccuracy;
        }

        m_Attribute.nMagicPoint += pSkill->GetVar(6) + pSkill->GetVar(7) * pSkill->GetCurrentSkillLevel();
        m_Attribute.nCritical += pSkill->GetVar(8) + pSkill->GetVar(9) * pSkill->GetCurrentSkillLevel();
    }
    break;

    case EF_BATTLE_PARAMTER_INCREASE:
    {
        int nAP = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        int nDefence = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();
        int nMP = pSkill->GetVar(4) + pSkill->GetVar(5) * pSkill->GetCurrentSkillLevel();
        int nMD = pSkill->GetVar(6) + pSkill->GetVar(7) * pSkill->GetCurrentSkillLevel();
        int nAvoid = pSkill->GetVar(8) + pSkill->GetVar(9) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nAttackPointRight += nAP;
        //m_nAttackPointRightWithoutWeapon += nAP;

        if (IsUsingDoubleWeapon())
        {
            m_Attribute.nAttackPointLeft += nAP;
            //m_nAttackPointLeftWithoutWeapon += nAP;
        }

        m_Attribute.nDefence += nDefence;
        m_Attribute.nMagicPoint += nMP;
        m_Attribute.nMagicDefence += nMD;
        m_Attribute.nAvoid += nAvoid;
    }
    break;

    case EF_BLOCK_INCREASE:
    {
        int nBlockChance = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        int nBlockDefence = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nBlockChance += nBlockChance;
        m_Attribute.nBlockDefence += nBlockDefence;
    }
    break;

    case EF_ATTACK_RANGE_INCREASE:
    {
        int nRange = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nAttackRange += nRange * GameRule::ATTACK_RANGE_UNIT;
    }
    break;

    case EF_RESISTANCE_INCREASE:
    {
        int nStunResi = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        int nMoveDecResi = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nStunResistance += nStunResi;
        m_Attribute.nMoveSpeedDecreaseResistance += nMoveDecResi;
    }
    break;

    case EF_INCREASE_BASE_ATTRIBUTE:
    {
        int nSkillLevel = pSkill->GetCurrentSkillLevel();

        m_Attribute.nAttackPointRight += pSkill->GetVar(0) * nSkillLevel;
        //m_nAttackPointRightWithoutWeapon += pSkill->GetVar( 0 ) * nSkillLevel;
        if (IsUsingDoubleWeapon())
        {
            m_Attribute.nAttackPointLeft += pSkill->GetVar(0) * nSkillLevel;
            //	m_nAttackPointLeftWithoutWeapon		+= pSkill->GetVar( 0 ) * nSkillLevel;
        }

        m_Attribute.nDefence += pSkill->GetVar(1) * nSkillLevel;

        m_Attribute.nMagicPoint += pSkill->GetVar(2) * nSkillLevel;
        m_Attribute.nMagicDefence += pSkill->GetVar(3) * nSkillLevel;

        m_Attribute.nAttackSpeedRight += pSkill->GetVar(4) * nSkillLevel;
        if (IsUsingDoubleWeapon())
            m_Attribute.nAttackSpeedLeft += pSkill->GetVar(4) * nSkillLevel;

        if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED) /*&& ( !IsPlayer() || !static_cast< StructPlayer * >( this )->HasRidingState() ) */)
            m_Attribute.nMoveSpeed += pSkill->GetVar(5) * nSkillLevel;

        m_Attribute.nAccuracyRight += pSkill->GetVar(6) * nSkillLevel;
        if (IsUsingDoubleWeapon())
            m_Attribute.nAccuracyRight += pSkill->GetVar(6) * nSkillLevel;

        m_Attribute.nMagicAccuracy += pSkill->GetVar(7) * nSkillLevel;

        m_Attribute.nAvoid += pSkill->GetVar(8) * nSkillLevel;
        m_Attribute.nMagicAvoid += pSkill->GetVar(9) * nSkillLevel;
    }
    break;

    case EF_AMPLIFY_BASE_ATTRIBUTE_OLD:
    case EF_AMPLIFY_BASE_ATTRIBUTE:
    {
        int nSkillLevel = pSkill->GetCurrentSkillLevel();

        m_AttributeAmplifier.fAttackPointRight += pSkill->GetVar(0) * nSkillLevel;
        m_AttributeAmplifier.fAttackSpeedRight += pSkill->GetVar(4) * nSkillLevel;
        m_AttributeAmplifier.fAccuracyRight += pSkill->GetVar(6) * nSkillLevel;

        if (IsUsingDoubleWeapon())
        {
            m_AttributeAmplifier.fAttackPointLeft += pSkill->GetVar(0) * nSkillLevel;
            m_AttributeAmplifier.fAttackSpeedLeft += pSkill->GetVar(4) * nSkillLevel;
            m_AttributeAmplifier.fAccuracyLeft += pSkill->GetVar(6) * nSkillLevel;
        }

        m_AttributeAmplifier.fDefence += pSkill->GetVar(1) * nSkillLevel;
        m_AttributeAmplifier.fMagicPoint += pSkill->GetVar(2) * nSkillLevel;
        m_AttributeAmplifier.fMagicDefence += pSkill->GetVar(3) * nSkillLevel;
        m_AttributeAmplifier.fMoveSpeed += pSkill->GetVar(5) * nSkillLevel;
        m_AttributeAmplifier.fMagicAccuracy += pSkill->GetVar(7) * nSkillLevel;
        m_AttributeAmplifier.fAvoid += pSkill->GetVar(8) * nSkillLevel;
        m_AttributeAmplifier.fMagicAvoid += pSkill->GetVar(9) * nSkillLevel;
    }
    break;

    case EF_INCREASE_EXTENSION_ATTRIBUTE:
    {
        int nSkillLevel = pSkill->GetCurrentSkillLevel();

        m_Attribute.nHPRegenPercentage += pSkill->GetVar(0) * nSkillLevel;
        m_Attribute.nHPRegenPoint += pSkill->GetVar(1) * nSkillLevel;
        m_Attribute.nMPRegenPercentage += pSkill->GetVar(2) * nSkillLevel;
        m_Attribute.nMPRegenPoint += pSkill->GetVar(3) * nSkillLevel;
        m_Attribute.nBlockChance += pSkill->GetVar(4) * nSkillLevel;
        m_Attribute.nBlockDefence += pSkill->GetVar(5) * nSkillLevel;
        m_Attribute.nCritical += pSkill->GetVar(6) * nSkillLevel;
        m_Attribute.nCriticalPower += pSkill->GetVar(7) * nSkillLevel;
        m_Attribute.nCastingSpeed += pSkill->GetVar(8) * nSkillLevel;
        m_Attribute.nCoolTimeSpeed -= pSkill->GetVar(9) * nSkillLevel;
    }
    break;

    case EF_MAGIC_REGISTANCE_INCREASE:
    {
        int nElemental;
        int nElementalInc;

        for (int i = 0; i < 4; ++i)
        {
            nElemental = pSkill->GetVar(3 * i);
            nElementalInc = pSkill->GetVar(3 * i + 1) + pSkill->GetVar(3 * i + 2) * pSkill->GetCurrentSkillLevel();

            m_Resist.nResist[nElemental] += nElementalInc;
        }
    }
    break;

    case EF_SPECIALIZE_ARMOR:
    {
        if (GetArmorClass() != pSkill->GetVar(0))
            break;

        m_Attribute.nAccuracyRight += pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetCurrentSkillLevel();

        if (IsUsingDoubleWeapon())
            m_Attribute.nAccuracyLeft += pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nAvoid += pSkill->GetVar(3) + pSkill->GetVar(4) * pSkill->GetCurrentSkillLevel();
        m_Attribute.nMPRegenPercentage += pSkill->GetVar(5) + pSkill->GetVar(6) * pSkill->GetCurrentSkillLevel();
    }
    break;
    case EF_SPECIALIZE_ARMOR_AMP:
    {
        if (GetArmorClass() != pSkill->GetVar(0))
            break;

        m_AttributeAmplifier.fAttackSpeedRight += pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetCurrentSkillLevel();
        if (IsUsingDoubleWeapon())
            m_AttributeAmplifier.fAttackSpeedLeft += pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetCurrentSkillLevel();

        m_AttributeAmplifier.fCritical += pSkill->GetVar(3) + pSkill->GetVar(4) * pSkill->GetCurrentSkillLevel();

        /*m_fMaxHPAmplifier += pSkill->GetVar( 5 ) + pSkill->GetVar( 6 ) * pSkill->GetCurrentSkillLevel();
			m_fMaxMPAmplifier += pSkill->GetVar( 11 ) + pSkill->GetVar( 12 ) * pSkill->GetCurrentSkillLevel();
			m_AttributeAmplifier.fHPRegenPercentage += pSkill->GetVar( 7 ) + pSkill->GetVar( 8 ) * pSkill->GetCurrentSkillLevel();
			m_AttributeAmplifier.fMPRegenPercentage += pSkill->GetVar( 9 ) + pSkill->GetVar( 10 ) * pSkill->GetCurrentSkillLevel();*/
    }
    break;

    case EF_MAGIC_TRAINING:
    {
        int nCastSpeed = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        float fCoolTime = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();
        int nCastKeep = pSkill->GetVar(4) + pSkill->GetVar(5) * pSkill->GetCurrentSkillLevel();

        m_Attribute.nCastingSpeed += nCastSpeed;
        m_Attribute.nCoolTimeSpeed -= m_Attribute.nCoolTimeSpeed * fCoolTime;
        //m_nCastKeep += nCastKeep;
    }
    break;

    case EF_HUNTING_TRAINING:
    {
        int nCreatureType = pSkill->GetVar(0);

        float fDamage = pSkill->GetVar(1) + pSkill->GetVar(2) * pSkill->GetCurrentSkillLevel();
        float fAvoid = pSkill->GetVar(3) + pSkill->GetVar(4) * pSkill->GetCurrentSkillLevel();

        m_Expert[nCreatureType].fDamage += fDamage;
        m_Expert[nCreatureType].fAvoid += fAvoid;
    }
    break;

    case EF_BOW_TRAINING:
        break;

    case EF_INCREASE_HP_MP:
    {
        int nMaxHPInc = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        int nMaxMPInc = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();
        int nHPRegenInc = pSkill->GetVar(6) + pSkill->GetVar(7) * pSkill->GetCurrentSkillLevel();
        int nMPRegenInc = pSkill->GetVar(8) + pSkill->GetVar(9) * pSkill->GetCurrentSkillLevel();

        SetMaxHealth(GetMaxHealth() + nMaxHPInc);
        SetMaxMana(GetMaxMana() + nMaxMPInc);
        m_Attribute.nHPRegenPoint += nHPRegenInc;
        m_Attribute.nMPRegenPoint += nMPRegenInc;
    }
    break;

    case EF_AMPLIFY_HP_MP:
    {
        //m_vAmplifyPassiveSkillList.push_back( pSkill );
    }
    break;

    case EF_HEALING_AMPLIFY:
    {
        float fHPHealRatio = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        float fMPHealRatio = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();

        SetFloatValue(UNIT_FIELD_HEAL_RATIO, GetFloatValue(UNIT_FIELD_HEAL_RATIO) + fHPHealRatio);
        SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO, GetFloatValue(UNIT_FIELD_MP_HEAL_RATIO) + fMPHealRatio);
    }
    break;

    case EF_HEALING_AMPLIFY_BY_ITEM:
    {
        float fHPHealRatioByItem = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        float fMPHealRatioByItem = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();

        SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_ITEM, GetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_ITEM) + fHPHealRatioByItem);
        SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_ITEM, GetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_ITEM) + fMPHealRatioByItem);
    }
    break;

    case EF_HEALING_AMPLIFY_BY_REST:
    {
        float fHPHealRatioByRest = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        float fMPHealRatioByRest = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();

        SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_REST, GetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_REST) + fHPHealRatioByRest);
        SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_REST, GetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_REST) + fMPHealRatioByRest);
    }
    break;

    case EF_HATE_AMPLIFY:
        SetFloatValue(UNIT_FIELD_HATE_RATIO, GetFloatValue(UNIT_FIELD_HATE_RATIO) + pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel());
        break;

    case EF_WEAPON_TRAINING:
    {
        auto *pWeapon = GetWornItem(WEAR_WEAPON);

        if (pWeapon == nullptr)
            break;

        float fAttackPoint = pSkill->GetVar(0) + pSkill->GetVar(1) * pSkill->GetCurrentSkillLevel();
        float fAttackSpeed = pSkill->GetVar(2) + pSkill->GetVar(3) * pSkill->GetCurrentSkillLevel();
        float fAccuracy = pSkill->GetVar(4) + pSkill->GetVar(5) * pSkill->GetCurrentSkillLevel();
        float fMagicPoint = pSkill->GetVar(6) + pSkill->GetVar(7) * pSkill->GetCurrentSkillLevel();
        float fCritical = pSkill->GetVar(8) + pSkill->GetVar(9) * pSkill->GetCurrentSkillLevel();

        m_AttributeAmplifier.fAttackPointRight += fAttackPoint;
        m_AttributeAmplifier.fAttackSpeedRight += fAttackSpeed;
        m_AttributeAmplifier.fAccuracyRight += fAccuracy;

        if (IsUsingDoubleWeapon())
        {
            m_AttributeAmplifier.fAttackPointLeft += fAttackPoint;
            m_AttributeAmplifier.fAttackSpeedLeft += fAttackSpeed;
            m_AttributeAmplifier.fAccuracyLeft += fAccuracy;
        }

        m_AttributeAmplifier.fMagicPoint += fMagicPoint;
        m_AttributeAmplifier.fCritical += fCritical;
    }
    break;

    case EF_AMPLIFY_EXT_ATTRIBUTE:
    {
        int nSkillLevel = pSkill->GetCurrentSkillLevel();

        m_AttributeAmplifier.fHPRegenPercentage += pSkill->GetVar(0) * nSkillLevel;
        m_AttributeAmplifier.fHPRegenPoint += pSkill->GetVar(1) * nSkillLevel;
        m_AttributeAmplifier.fMPRegenPercentage += pSkill->GetVar(2) * nSkillLevel;
        m_AttributeAmplifier.fMPRegenPoint += pSkill->GetVar(3) * nSkillLevel;
        m_AttributeAmplifier.fBlockChance += pSkill->GetVar(4) * nSkillLevel;
        m_AttributeAmplifier.fBlockDefence += pSkill->GetVar(5) * nSkillLevel;
        m_AttributeAmplifier.fCritical += pSkill->GetVar(6) * nSkillLevel;
        m_AttributeAmplifier.fCriticalPower += pSkill->GetVar(7) * nSkillLevel;
        m_AttributeAmplifier.fCastingSpeed += pSkill->GetVar(8) * nSkillLevel;
        m_AttributeAmplifier.fCoolTimeSpeed += pSkill->GetVar(9) * nSkillLevel;
        //m_nCastKeep *= pSkill->GetVar(10) * nSkillLevel;
    }
    break;

    case EF_MISC:
    {
        switch (pSkill->GetSkillId())
        {
        case SKILL_AMORY_UNIT:
        {
            m_vAmplifyPassiveSkillList.emplace_back(pSkill);
        }
        break;
        case SKILL_INCREASE_ENERGY:
        {
            SetInt32Value(UNIT_FIELD_MAX_ENERGY, GetInt32Value(UNIT_FIELD_MAX_ENERGY) + pSkill->GetCurrentSkillLevel());
        }
        break;
        case SKILL_ARMOR_MASTERY:
        case SKILL_GAIA_ARMOR_MASTERY:
            if (GetWornItem(WEAR_ARMOR))
            {
                m_Attribute.nDefence += pSkill->GetVar(0) * pSkill->GetCurrentSkillLevel();
            }
            break;

        case SKILL_LIFE_OF_DARKNESS:
        {
            m_Resist.nResist[ElementalType::TYPE_DARK] += pSkill->GetVar(0) * pSkill->GetCurrentSkillLevel();
        }
        break;
        default:
            break;
        }
    }
    break;
    default:
        break;
    }
}

void Unit::applyPassiveSkillEffect()
{
    if (m_vPassiveSkillList.empty())
        return;

    for (auto &pSkill : m_vPassiveSkillList)
    {
        if (pSkill->GetSkillBase()->IsNeedWeapon())
        {
            if (pSkill->GetSkillBase()->IsUseableWeapon(CLASS_SHIELD))
            {
                if (!IsWearShield())
                    continue;
            }
            else if (!pSkill->GetSkillBase()->IsUseableWeapon(GetWeaponClass()))
            {
                continue;
            }
        }
        applyPassiveSkillEffect(pSkill);
    }
}

void Unit::applyDoubeWeaponEffect()
{
    float fAddPerLevel{0};
    Skill *skill{nullptr};

    if (IsUsingDoubleWeapon())
    {
        auto pSlot0 = GetWornItem(WEAR_WEAPON);
        auto pSlot1 = GetWornItem(WEAR_SHIELD);
        if (pSlot0 != nullptr || pSlot1 != nullptr)
        {
            int skillLevel = 1;
            if (pSlot1->m_pItemBase->iclass == CLASS_ONEHAND_SWORD && pSlot0->m_pItemBase->iclass == CLASS_ONEHAND_SWORD)
            {
                skill = GetSkill((int)SKILL_DUAL_SWORD_EXPERT);
            }
            else if (pSlot1->m_pItemBase->iclass == CLASS_DAGGER && pSlot0->m_pItemBase->iclass == CLASS_DAGGER)
            {
                skill = GetSkill((int)SKILL_TWIN_BLADE_EXPERT);
            }
            else if (pSlot1->m_pItemBase->iclass == CLASS_ONEHAND_AXE && pSlot0->m_pItemBase->iclass == CLASS_ONEHAND_AXE)
            {
                skill = GetSkill((int)SKILL_TWIN_AXE_EXPERT);
            }
            if (skill != nullptr)
                skillLevel = skill->m_nSkillLevel + skill->m_nSkillLevelAdd;

            m_nDoubleWeaponMasteryLevel = skillLevel;
            m_Attribute.nAttackSpeedRight *= (0.75f + ((float)skillLevel * 0.005f));
            m_Attribute.nAttackSpeedLeft *= (0.75f + ((float)skillLevel * 0.005f));
            m_Attribute.nAttackSpeed = (m_Attribute.nAttackSpeedLeft + m_Attribute.nAttackSpeedRight) * 0.5f;
            m_AttributeByState.nAttackSpeedRight *= (0.75f + ((float)skillLevel * 0.005f));
            m_AttributeByState.nAttackSpeedLeft *= (0.75f + ((float)skillLevel * 0.005f));
            m_AttributeByState.nAttackSpeed = (m_AttributeByState.nAttackSpeedLeft + m_AttributeByState.nAttackSpeedRight) * 0.5f;

            m_Attribute.nAccuracyLeft *= (0.89f + ((float)skillLevel * 0.01f));
            m_Attribute.nAccuracyRight *= (0.89f + ((float)skillLevel * 0.01f));
            m_AttributeByState.nAccuracyLeft *= (0.89f + ((float)skillLevel * 0.01f));
            m_AttributeByState.nAccuracyRight *= (0.89f + ((float)skillLevel * 0.01f));

            m_Attribute.nAttackPointRight *= (0.9f + ((float)skillLevel * 0.01f));
            m_Attribute.nAttackPointLeft *= (0.44f + ((float)skillLevel * 0.02f));
            /*m_nAttackPointRightWithoutWeapon *= (0.9f + ((float)skillLevel * 0.01f));
            m_nAttackPointRightWithoutWeapon *= (0.44f + ((float)skillLevel * 0.02f));*/
            m_AttributeByState.nAttackPointRight *= (0.9f + ((float)skillLevel * 0.01f));
            m_AttributeByState.nAttackPointLeft *= (0.44f + ((float)skillLevel * 0.02f));
            return;
        }
    }
    m_Attribute.nAttackSpeed = m_Attribute.nAttackSpeedRight;
    m_AttributeByState.nAttackSpeed = m_AttributeByState.nAttackSpeedRight;
}