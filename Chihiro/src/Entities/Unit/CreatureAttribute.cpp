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

#include "CreatureAttribute.h"
#include "XPacket.h"

void CreatureStat::Reset(int16_t v)
{
    stat_id      = v;
    strength     = v;
    vital        = v;
    dexterity    = v;
    agility      = v;
    intelligence = v;
    mentality    = v;
    luck         = v;
}

void CreatureStat::Copy(const CreatureStat& v)
{
    stat_id      = v.stat_id;
    strength     = v.strength;
    vital        = v.vital;
    dexterity    = v.dexterity;
    agility      = v.agility;
    intelligence = v.intelligence;
    mentality    = v.mentality;
    luck         = v.luck;
}

void CreatureStat::Add(const CreatureStat& v)
{
    strength += v.strength;
    vital += v.vital;
    dexterity += v.dexterity;
    agility += v.agility;
    intelligence += v.intelligence;
    mentality += v.mentality;
    luck += v.luck;
}

void CreatureStat::WriteToPacket(XPacket &packet)
{
    packet << (int16_t) stat_id;
    packet << (int16_t) (strength > 32000 ? 32000 : strength);
    packet << (int16_t) (vital > 32000 ? 32000 : vital);
    packet << (int16_t) (dexterity > 32000 ? 32000 : dexterity);
    packet << (int16_t) (agility > 32000 ? 32000 : agility);
    packet << (int16_t) (intelligence > 32000 ? 32000 : intelligence);
    packet << (int16_t) (mentality > 32000 ? 32000 : mentality);
    packet << (int16_t) (luck > 32000 ? 32000 : luck);
}

void CreatureAtributeServer::Reset(int16_t v)
{
    nCritical          = v;
    nCriticalPower     = v;
    nAttackPointRight  = v;
    nAttackPointLeft   = v;
    nDefence           = v;
    nBlockDefence      = v;
    nMagicPoint        = v;
    nMagicDefence      = v;
    nAccuracyRight     = v;
    nAccuracyLeft      = v;
    nMagicAccuracy     = v;
    nAvoid             = v;
    nMagicAvoid        = v;
    nBlockChance       = v;
    nMoveSpeed         = v;
    nAttackSpeed       = v;
    nAttackRange       = v;
    nMaxWeight         = v;
    nCastingSpeed      = v;
    nCoolTimeSpeed     = v;
    nItemChance        = v;
    nHPRegenPercentage = v;
    nHPRegenPoint      = v;
    nMPRegenPercentage = v;
    nMPRegenPoint      = v;

    nAttackSpeedRight            = v;
    nAttackSpeedLeft             = v;
    nDoubleAttackRatio           = v;
    nStunResistance              = v;
    nMoveSpeedDecreaseResistance = v;
    nHPAdd                       = v;
    nMPAdd                       = v;
    nHPAddByItem                 = v;
    nMPAddByItem                 = v;
}

void CreatureAtributeServer::Copy(const CreatureAtributeServer& v)
{
    nCritical          = v.nCritical;
    nCriticalPower     = v.nCriticalPower;
    nAttackPointRight  = v.nAttackPointRight;
    nAttackPointLeft   = v.nAttackPointLeft;
    nDefence           = v.nDefence;
    nBlockDefence      = v.nBlockDefence;
    nMagicPoint        = v.nMagicPoint;
    nMagicDefence      = v.nMagicDefence;
    nAccuracyRight     = v.nAccuracyRight;
    nAccuracyLeft      = v.nAccuracyLeft;
    nMagicAccuracy     = v.nMagicAccuracy;
    nAvoid             = v.nAvoid;
    nMagicAvoid        = v.nMagicAvoid;
    nBlockChance       = v.nBlockChance;
    nMoveSpeed         = v.nMoveSpeed;
    nAttackSpeed       = v.nAttackSpeed;
    nAttackRange       = v.nAttackRange;
    nMaxWeight         = v.nMaxWeight;
    nCastingSpeed      = v.nCastingSpeed;
    nCoolTimeSpeed     = v.nCoolTimeSpeed;
    nItemChance        = v.nItemChance;
    nHPRegenPercentage = v.nHPRegenPercentage;
    nHPRegenPoint      = v.nHPRegenPoint;
    nMPRegenPercentage = v.nMPRegenPercentage;
    nMPRegenPoint      = v.nMPRegenPoint;

    nAttackSpeedRight            = v.nAttackSpeedRight;
    nAttackSpeedLeft             = v.nAttackSpeedLeft;
    nDoubleAttackRatio           = v.nDoubleAttackRatio;
    nStunResistance              = v.nStunResistance;
    nMoveSpeedDecreaseResistance = v.nMoveSpeedDecreaseResistance;
    nHPAdd                       = v.nHPAdd;
    nMPAdd                       = v.nMPAdd;
    nHPAddByItem                 = v.nHPAddByItem;
    nMPAddByItem                 = v.nMPAddByItem;
}

void CreatureAtributeServer::WriteToPacket(XPacket &packet)
{
    packet << (int16) (nCritical > 32000 ? 32000 : nCritical);
    packet << (int16) (nCriticalPower > 32000 ? 32000 : nCriticalPower);
    packet << (int16) (nAttackPointRight > 32000 ? 32000 : nAttackPointRight);
    packet << (int16) (nAttackPointLeft > 32000 ? 32000 : nAttackPointLeft);
    packet << (int16) (nDefence > 32000 ? 32000 : nDefence);
    packet << (int16) (nBlockDefence > 32000 ? 32000 : nBlockDefence);
    packet << (int16) (nMagicPoint > 32000 ? 32000 : nMagicPoint);
    packet << (int16) (nMagicDefence > 32000 ? 32000 : nMagicDefence);
    packet << (int16) (nAccuracyRight > 32000 ? 32000 : nAccuracyRight);
    packet << (int16) (nAccuracyLeft > 32000 ? 32000 : nAccuracyLeft);
    packet << (int16) (nMagicAccuracy > 32000 ? 32000 : nMagicAccuracy);
    packet << (int16) (nAvoid > 32000 ? 32000 : nAvoid);
    packet << (int16) (nMagicAvoid > 32000 ? 32000 : nMagicAvoid);
    packet << (int16) (nBlockChance > 32000 ? 32000 : nBlockChance);
    packet << (int16) (nMoveSpeed > 32000 ? 32000 : nMoveSpeed);
    packet << (int16) (nAttackSpeedRight > 32000 ? 32000 : nAttackSpeedRight);
    packet << (int16) (nAttackRange > 32000 ? 32000 : nAttackRange);
    packet << (int16_t) nMaxWeight;
    packet << (int16) (nCastingSpeed > 32000 ? 32000 : nCastingSpeed);
    packet << (int16) (nCoolTimeSpeed > 32000 ? 32000 : nCoolTimeSpeed);
    packet << (int16) (nItemChance > 32000 ? 32000 : nItemChance);
    packet << (int16) (nHPRegenPercentage > 32000 ? 32000 : nHPRegenPercentage);
    packet << (int16) (nHPRegenPoint > 32000 ? 32000 : nHPRegenPoint);
    packet << (int16) (nMPRegenPercentage > 32000 ? 32000 : nMPRegenPercentage);
    packet << (int16) (nMPRegenPoint > 32000 ? 32000 : nMPRegenPoint);

}