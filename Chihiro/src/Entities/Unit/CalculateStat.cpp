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

#include "Unit.h"
#include "ObjectMgr.h"
#include "Item.h"
#include "Messages.h"
#include "Skill.h"
#include "Summon.h"
#include "Player.h"
#include "GameRule.h"

void Unit::CalculateStat()
{
    CreatureAtributeServer stateAttr{ };
    CreatureStat           stateStat{ };

    int prev_max_hp = GetMaxHealth();
    int prev_max_mp = GetMaxMana();
    int prev_hp     = GetHealth();
    int prev_mp     = GetMana();

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

    m_cStatByState.Reset(0);
    m_StatAmplifier.Reset(0.0f);
    m_AttributeByState.Reset(0);
    m_AttributeAmplifier.Reset(0);
    m_Attribute.Reset(0);
    m_Resist.Reset(0);
    m_ResistAmplifier.Reset(0.0f);
    m_vNormalAdditionalDamage.clear();
    m_vRangeAdditionalDamage.clear();
    m_vMagicialSkillAdditionalDamage.clear();
    m_vPhysicalSkillAdditionalDamage.clear();

    auto         statptr = GetBaseStat();
    CreatureStat basestat{ };
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
    float b1  = GetLevel();
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
    auto hp = GetMaxHealth();
    auto mp = GetMaxMana();
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

    std::vector<std::pair<int, int>> vDecreaseList{ };
    for (auto &s : m_vStateList)
    {
        if (s.GetEffectType() == 84)
        {// Strong Spirit?
            auto     nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for (int i              = 2; i < 11; ++i)
            {
                if (s.GetValue(i) == 0.0f)
                {
                    vDecreaseList.emplace_back(std::pair<int, int>((int)s.GetValue(1), (int)nDecreaseLevel));
					break;
                }
            }
        }
    }
    for (auto &s :  m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s.m_nCode)
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

        if (s.GetLevel() != 0 && s.GetEffectType() == 2)
        {
            // format is 0 = bitset, 1 = base, 2 = add per level
            ampParameter((uint)s.GetValue(0), (int)(s.GetValue(1) + (s.GetValue(2) * s.GetLevel())), true);
            ampParameter((uint)s.GetValue(3), (int)(s.GetValue(4) + (s.GetValue(5) * s.GetLevel())), true);
            ampParameter((uint)s.GetValue(12), (int)(s.GetValue(13) + (s.GetValue(14) * s.GetLevel())), true);
            ampParameter((uint)s.GetValue(15), (int)(s.GetValue(16) + (s.GetValue(17) * s.GetLevel())), true);
        }
        for (int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyState(State &state)
{
    int effectType = state.GetEffectType();

    switch (effectType)
    {
        case StateBaseEffect::SEF_MISC:
            switch (state.m_nCode)
            {
                case StateCode::SC_SLEEP:
                case StateCode::SC_STUN:
                    //this.m_StatusFlag &= ~(CREATURE_STATUS.STATUS_MOVABLE|CREATURE_STATUS.STATUS_MAGIC_CASTABLE|CREATURE_STATUS.STATUS_ITEM_USABLE);

                    break;
                default:
                    break;
            }
            break;
        case StateBaseEffect::SEF_PARAMETER_INC:
            incParameter((uint)state.GetValue(0), (int)(state.GetValue(1) + (state.GetValue(2) * state.GetLevel())), false);
            incParameter((uint)state.GetValue(3), (int)(state.GetValue(4) + (state.GetValue(5) * state.GetLevel())), false);
            incParameter2((uint)state.GetValue(6), (int)(state.GetValue(7) + (state.GetValue(8) * state.GetLevel())));
            incParameter2((uint)state.GetValue(9), (int)(state.GetValue(10) + (state.GetValue(11) * state.GetLevel())));
            incParameter((uint)state.GetValue(12), (int)(state.GetValue(13) + (state.GetValue(14) * state.GetLevel())), false);
            incParameter((uint)state.GetValue(15), (int)(state.GetValue(16) + (state.GetValue(17) * state.GetLevel())), false);
            break;
        case SEF_ADDITIONAL_DAMAGE_ON_ATTACK:
            ampParameter2((uint)(0x200000 * (std::pow(2, (uint)state.GetValue(8)))), state.GetValue(0) + (state.GetValue(1) * state.GetLevel()));
            break;
        default:
            break;
    }
}

void Unit::applyStateEffect()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{ };
    for (auto &s : m_vStateList)
    {
        if (s.GetEffectType() == 84)
        {// Strong Spirit?
            auto     nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for (int i              = 2; i < 11; ++i)
            {
                if (s.GetValue(i) == 0.0f)
                {
                    vDecreaseList.emplace_back(std::pair<int, int>((int)s.GetValue(1), (int)nDecreaseLevel));
					break;
                }
            }
        }
    }
    for (auto &s :  m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s.m_nCode)
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

        if (s.GetLevel() != 0)
        {
            applyState(s);
        }
        for (int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyStatByState()
{
    if (m_vStateList.empty())
        return;

    std::vector<std::pair<int, int>> vDecreaseList{ };
    for (auto &s : m_vStateList)
    {
        if (s.GetEffectType() == 84)
        {// Strong Spirit?
            auto     nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for (int i              = 2; i < 11; ++i)
            {
                if (s.GetValue(i) == 0.0f)
                {
                    vDecreaseList.emplace_back(std::pair<int, int>(static_cast<int>(s.GetValue(1)), static_cast<int>(nDecreaseLevel)));
					break;
                }
            }
        }
    }
    for (auto &s :  m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s.m_nCode)
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

        if (s.GetLevel() != 0)
        {
            if (s.GetEffectType() == 1)
            {
                // format is 0 = bitset, 1 = base, 2 = add per level
                incParameter((uint)s.GetValue(0), (int)(s.GetValue(1) + (s.GetValue(2) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(3), (int)(s.GetValue(4) + (s.GetValue(5) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(12), (int)(s.GetValue(13) + (s.GetValue(14) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(15), (int)(s.GetValue(16) + (s.GetValue(17) * s.GetLevel())), true);
            }
            else if (s.GetEffectType() == 3 && IsWearShield())
            {
                incParameter((uint)s.GetValue(0), (int)(s.GetValue(1) + (s.GetValue(2) + s.GetLevel())), true);
                incParameter((uint)s.GetValue(3), (int)(s.GetValue(4) + (s.GetValue(5) + s.GetLevel())), true);
            }
        }
        for (int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyItemEffect()
{
    Item *curItem = GetWornItem(WEAR_WEAPON);
    if (curItem != nullptr && curItem->m_pItemBase != nullptr)
        m_Attribute.nAttackRange = curItem->m_pItemBase->range;

    m_nUnitExpertLevel = 0;
    std::vector<int> ref_list{ };

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
                    fItemRatio   = 0.40000001f;

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

                float fAddPoint    = 0.0f;
                float fTotalPoints = 0.0f;

                for (int ol = 0; ol < 2; ol++)
                {
                    if (curItem->m_pItemBase->enhance_id[ol] != 0)
                    {
                        int curEnhance  = curItem->m_Instance.nEnhance;
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
            if (p == nullptr || true/*|| p->m_nRidingStateUid == 0*/)
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
    if (bStat) {
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
    } else {
        if ((nBitset & 0x80) != 0) {
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
        if ((nBitset & 0x800) != 0) {
            m_Attribute.nAttackSpeedRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAttackSpeedLeft += nValue;
        }
        if ((nBitset & 0x1000) != 0)
            m_Attribute.nCastingSpeed += nValue;
        if ((nBitset & 0x2000) != 0 && !HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        {
            auto p = dynamic_cast<Player*>(this);
            if (p != nullptr /*|| p.m_nRidingStateUid == 0*/)
                m_Attribute.nMoveSpeed += nValue;
        }
        if ((nBitset & 0x4000) != 0) {
            m_Attribute.nAccuracyRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAccuracyLeft += nValue;
        }
        if ((nBitset & 0x8000) != 0)
            m_Attribute.nMagicAccuracy += nValue;
        if ((nBitset & 0x10000) != 0)
            m_Attribute.nCritical += (short) nValue;
        if ((nBitset & 0x20000) != 0)
            m_Attribute.nBlockChance += nValue;
        if ((nBitset & 0x40000) != 0)
            m_Attribute.nBlockDefence += nValue;
        if ((nBitset & 0x80000) != 0)
            m_Attribute.nAvoid += nValue;
        if ((nBitset & 0x100000) != 0)
            m_Attribute.nMagicAvoid += nValue;
        if ((nBitset & 0x200000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_HEALTH, GetInt32Value(UNIT_FIELD_MAX_HEALTH) + (int) nValue);
        if ((nBitset & 0x400000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_MANA, GetInt32Value(UNIT_FIELD_MAX_MANA) + (int) nValue);
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
    attribute.nAvoid += (m_AttributeAmplifier.fAvoid * attribute.nAvoid);;
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

    std::vector<std::pair<int, int>> vDecreaseList{ };
    for (auto &s : m_vStateList)
    {
        if (s.GetEffectType() == 84)
        {// Strong Spirit?
            auto     nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for (int i              = 2; i < 11; ++i)
            {
                if (s.GetValue(i) == 0.0f)
                {
                    vDecreaseList.emplace_back(std::pair<int, int>((int)s.GetValue(1), (int)nDecreaseLevel));
					break;
                }
            }
        }
    }
    for (auto &s :  m_vStateList)
    {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for (auto &rp : vDecreaseList)
        {
            if (rp.first == (int)s.m_nCode)
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

        if (s.GetLevel() != 0)
        {
            applyStateAmplify(s);
        }
        for (int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyStateAmplify(State &state)
{
    int effectType = state.GetEffectType();
    int level      = state.GetLevel();
    if (effectType != 0)
    {
        if (effectType == StateBaseEffect::SEF_PARAMETER_AMP)
        {
            ampParameter((uint)state.GetValue(0), (state.GetValue(1) + (state.GetValue(2) * level)), false);
            ampParameter((uint)state.GetValue(3), (state.GetValue(4) + (state.GetValue(5) * level)), false);
            ampParameter2((uint)state.GetValue(6), (state.GetValue(7) + (state.GetValue(8) * level)));
            ampParameter2((uint)state.GetValue(9), (state.GetValue(10) + (state.GetValue(11) * level)));
            ampParameter((uint)state.GetValue(12), (state.GetValue(13) + (state.GetValue(14) * level)), false);
            ampParameter((uint)state.GetValue(15), (state.GetValue(16) + (state.GetValue(17) * level)), false);
        }
    }
    else
    {
        if (state.m_nCode == StateCode::SC_SQUALL_OF_ARROW)
        {
            if (state.GetValue(1) == 0.0f)
                SetFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);
            else
                ToggleFlag(UNIT_FIELD_STATUS, STATUS_MOVABLE);

            if ((int)GetWeaponClass() == (int)state.GetValue(0))
            {
                m_AttributeAmplifier.fAttackSpeedRight += state.GetValue(2) + (state.GetValue(3) * level);
                if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                {
                    m_AttributeAmplifier.fAttackSpeedLeft += state.GetValue(2) + (state.GetValue(3) * level);
                }
            }
        }
    }
}

void Unit::onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio)
{
    float item_var_penalty{ };

    Player *p{nullptr};
    if (IsPlayer())
        p = dynamic_cast<Player *>(this);

    if (type == 14)
        item_var_penalty = var1;
    else
        item_var_penalty = var1 * fRatio;

    if (type != 14)
    {
        if (pItem != nullptr && bIsBaseVar)
        {
            item_var_penalty += (float)(var2 * (float)(pItem->m_Instance.nLevel - 1));
            item_var_penalty = GameRule::GetItemValue(item_var_penalty, (int)var1, GetLevel(), pItem->GetItemRank(), pItem->m_Instance.nLevel);
        }
        // TODO m_fItemMod = item_var_penalty
    }

    if (type > 96)
    {
        int t1 = type - 97;
        if (t1 != 0)
        {
            int t2 = t1 - 1;
            if (t2 != 0)
            {
                if (t2 == 1)
                    ampParameter2((uint)var1, var2);
            }
            else
            {
                ampParameter((uint)var1, var2, false);
            }
        }
        else
        {
            incParameter2((uint)var1, var2);
        }
    }
    else
    {
        if (type == 96)
        {
            incParameter((uint)var1, (int)var2, bIsBaseVar);
        }
        else
        {
            switch (type)
            {
                case 12:
                    m_Attribute.nMagicPoint += item_var_penalty;
                    return;
                case 11:
                    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                    {
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAttackPointLeft += item_var_penalty;
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem->m_Instance.nWearInfo != WEAR_SHIELD)
                    {
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAttackPointLeft += item_var_penalty;
                    }
                    return;
                case 21:
                    m_Attribute.nBlockDefence += item_var_penalty;
                    return;
                case 15:
                    m_Attribute.nDefence += item_var_penalty;
                    return;
                case 13:
                    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                    {
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAccuracyLeft += item_var_penalty;
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem->m_Instance.nWearInfo != WEAR_SHIELD)
                    {
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAccuracyLeft += item_var_penalty;
                    }
                    return;
                case 14:
                    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                        m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }
                    else if (pItem->m_Instance.nWearInfo != WEAR_SHIELD)
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }
                    return;
                case 16:
                    m_Attribute.nMagicDefence += item_var_penalty;
                    return;
                case 17:
                    m_Attribute.nAvoid += item_var_penalty;
                    return;
                case 18:
                    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED) && (p == nullptr || true)) // TODO: RidingStateUID
                        m_Attribute.nMoveSpeed += item_var_penalty;
                    return;
                case 19:
                    m_Attribute.nBlockChance += item_var_penalty;
                    return;
                case 20:
                    m_Attribute.nMaxWeight += item_var_penalty;
                    return;
                case 22:
                    m_Attribute.nCastingSpeed += item_var_penalty;
                    return;
                case 23:
                    m_Attribute.nMagicAccuracy += item_var_penalty;
                    break;
                case 24:
                    m_Attribute.nMagicAvoid += item_var_penalty;
                    break;
                case 25:
                    m_Attribute.nCoolTimeSpeed += item_var_penalty;
                    break;
                case 30:
                    SetMaxHealth(GetMaxHealth() + (short)item_var_penalty);
                    break;
                case 31:
                    SetInt32Value(UNIT_FIELD_MAX_MANA, GetMaxMana() + (short)item_var_penalty);;
                    break;
                case 33:
                    m_Attribute.nMPRegenPoint += item_var_penalty;
                    break;
                case 34:
                    // m_fBowInterval = var1;
                    break;
                default:
                    return;
            }
        }
    }
}

void Unit::calcAttribute(CreatureAtributeServer &attribute)
{
    float v;
    attribute.nCriticalPower = 80;
    attribute.nCritical += ((m_cStat.luck * 0.2f) + 3.0f);

    float b1                   = GetLevel();
    float fcm                  = GetFCM();
    float dl                   = (fcm * 5.0f);

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
        attribute.nAttackSpeedLeft += 100;/*((this->m_cStat.dexterity) * 0.5f + (fcm + b1));*/
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        attribute.nMoveSpeed += 120;
    attribute.nCastingSpeed += 100;
    attribute.nCoolTimeSpeed   = 100;

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
    stat.strength     = (int)((m_StatAmplifier.strength * stat.strength) + stat.strength);
    stat.vital        = (int)((m_StatAmplifier.vital * stat.vital) + stat.vital);
    stat.dexterity    = (int)((m_StatAmplifier.dexterity * stat.dexterity) + stat.dexterity);
    stat.agility      = (int)((m_StatAmplifier.agility * stat.agility) + stat.agility);
    stat.intelligence = (int)((m_StatAmplifier.intelligence * stat.intelligence) + stat.intelligence);
    stat.mentality    = (int)((m_StatAmplifier.mentality * stat.mentality) + stat.mentality);
    stat.luck         = (int)((m_StatAmplifier.luck * stat.luck) + stat.luck);
}

void Unit::applyStatByItem()
{
    std::vector<int> ref_list{ };

    for (int i1 = 0; i1 < MAX_ITEM_WEAR; ++i1)
    {
        Item *item = m_anWear[i1];
        if (item != nullptr && item->m_pItemBase != nullptr)
        {
            auto iwt = (ItemWearType)i1;
            if (item->m_Instance.nWearInfo != WEAR_NONE)
            {
                if (TranslateWearPosition(iwt, item, ref_list))
                {
                    for (int j = 0; j < 4; j++)
                    {
                        short ot = item->m_pItemBase->opt_type[j];
                        auto  bs = (uint)item->m_pItemBase->opt_var[j][0];
                        auto  fv = (int)item->m_pItemBase->opt_var[j][1];
                        if (ot == 96)
                            incParameter(bs, fv, true);
                    }
                }
            }

            if (item->m_pItemBase->socket > 0)
            {
                for (const auto &x : item->m_Instance.Socket)
                {
                    if (x != 0)
                    {
                        auto ibs = sObjectMgr->GetItemBase(x);
                        if (ibs == nullptr)
                            continue;

                        if (ibs->use_min_level > GetLevel())
                            continue;

                        onItemWearEffect(item, true, ibs->opt_type[0], ibs->opt_var[0][0], ibs->opt_var[0][1], 1);
                    }
                }
            }
        }
    }
}


void Unit::applyPassiveSkillEffect(Skill *skill)
{
    float atk = 0;
    switch (skill->m_SkillBase->effect_type)
    {
        case SKILL_EFFECT_TYPE::EF_WEAPON_MASTERY:
        {
            auto weapon = GetWornItem(WEAR_WEAPON);
            if (weapon == nullptr)
                return;

            if (skill->m_SkillBase->id == SKILL_ADV_WEAPON_EXPERT)
                if (weapon->GetItemRank() < 2)
                    return;
            atk = (skill->m_SkillBase->var[0] + (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
            m_Attribute.nAttackPointRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAttackPointLeft += atk;

            m_Attribute.nAttackSpeed += (skill->m_SkillBase->var[2] + (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
            m_Attribute.nMagicPoint += (skill->m_SkillBase->var[6] + (skill->m_SkillBase->var[7] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
        }
            break;
        case SKILL_EFFECT_TYPE::EF_INCREASE_BASE_ATTRIBUTE:
        {
            atk = (skill->m_SkillBase->var[0] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nAttackPointRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAttackPointLeft += atk;

            m_Attribute.nDefence += (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nMagicPoint += (skill->m_SkillBase->var[2] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nMagicDefence += (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));

            atk = (skill->m_SkillBase->var[6] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nAccuracyRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
                m_Attribute.nAccuracyLeft += atk;
            m_Attribute.nMagicAccuracy += (skill->m_SkillBase->var[7] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
        }
            break;
        case SKILL_EFFECT_TYPE::EF_INCREASE_HP_MP:
        {
            SetMaxHealth((uint)(GetMaxHealth() + skill->m_SkillBase->var[0] + (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel))));
            SetMaxMana((uint)(GetMaxMana() + skill->m_SkillBase->var[2] + (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel))));
        }
            break;
        case SKILL_EFFECT_TYPE::EF_WEAPON_TRAINING:
        {
            m_Attribute.nAttackSpeedRight += (skill->m_SkillBase->var[3] * 100 * skill->m_nSkillLevel);
        }
            break;
        default:
            //NG_LOG_DEBUG("entities.unit", "Unknown SKill Effect Type Unit::applyPassiveSkillEffect: %u", skill->m_SkillBase->id);
            break;
    }

    // SPECIAL CASES
    switch (skill->m_nSkillID)
    {
        case 1201:
        case 1202: // Defense Practice
            if(m_anWear[WEAR_ARMOR] != nullptr)
                m_Attribute.nDefence += (skill->m_SkillBase->var[0] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            break;
        default:
            break;
    }
}

void Unit::applyPassiveSkillEffect()
{
    for(auto& s : m_vSkillList) {
        // yes, is_passive == 0 to get passive skills.. Gala :shrug:
        if(s != nullptr && s->m_SkillBase != nullptr && s->m_SkillBase->is_passive == 0) {
            bool r = s->m_SkillBase->vf_shield_only == 0 ? s->m_SkillBase->IsUseableWeapon(GetWeaponClass()) : IsWearShield();
            if(s->m_SkillBase->vf_is_not_need_weapon != 0 || r)
                applyPassiveSkillEffect(s);
        }
    }
}


void Unit::applyDoubeWeaponEffect()
{
    float fAddPerLevel{0};
    Skill *skill{nullptr};

    if (HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON))
    {
        auto pSlot0 = GetWornItem(WEAR_WEAPON);
        auto pSlot1 = GetWornItem(WEAR_SHIELD);
        if(pSlot0 != nullptr || pSlot1 != nullptr)
        {
            int skillLevel = 1;
            if (pSlot1->m_pItemBase->iclass == CLASS_ONEHAND_SWORD
                && pSlot0->m_pItemBase->iclass == CLASS_ONEHAND_SWORD)
            {
                skill = GetSkill((int)SKILL_DUAL_SWORD_EXPERT);
            }
            else if (pSlot1->m_pItemBase->iclass == CLASS_DAGGER
                     && pSlot0->m_pItemBase->iclass == CLASS_DAGGER)
            {
                skill = GetSkill((int)SKILL_TWIN_BLADE_EXPERT);
            }
            else if (pSlot1->m_pItemBase->iclass == CLASS_ONEHAND_AXE
                     && pSlot0->m_pItemBase->iclass == CLASS_ONEHAND_AXE)
            {
                skill = GetSkill((int)SKILL_TWIN_AXE_EXPERT);
            }
            if(skill != nullptr)
                skillLevel = skill->m_nSkillLevel + skill->m_nSkillLevelAdd;

            m_nDoubleWeaponMasteryLevel = skillLevel;
            m_Attribute.nAttackSpeedRight *= (0.75f+((float)skillLevel*0.005f));
            m_Attribute.nAttackSpeedLeft *= (0.75f+((float)skillLevel*0.005f));
            m_Attribute.nAttackSpeed = (m_Attribute.nAttackSpeedLeft+m_Attribute.nAttackSpeedRight) * 0.5f;
            m_AttributeByState.nAttackSpeedRight *= (0.75f+((float)skillLevel*0.005f));
            m_AttributeByState.nAttackSpeedLeft *= (0.75f+((float)skillLevel*0.005f));
            m_AttributeByState.nAttackSpeed = (m_AttributeByState.nAttackSpeedLeft+m_AttributeByState.nAttackSpeedRight) * 0.5f;

            m_Attribute.nAccuracyLeft *= (0.89f + ((float)skillLevel * 0.01f));
            m_Attribute.nAccuracyRight *= (0.89f + ((float)skillLevel * 0.01f));
            m_AttributeByState.nAccuracyLeft *= (0.89f + ((float)skillLevel * 0.01f));
            m_AttributeByState.nAccuracyRight *= (0.89f + ((float)skillLevel * 0.01f));

            m_Attribute.nAttackPointRight *= (0.9f+((float)skillLevel*0.01f));
            m_Attribute.nAttackPointLeft *= (0.44f+((float)skillLevel*0.02f));
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