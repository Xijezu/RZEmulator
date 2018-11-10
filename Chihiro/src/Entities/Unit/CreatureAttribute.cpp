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

void CreatureStat::Copy(const CreatureStat &v)
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

void CreatureStat::Add(const CreatureStat &v)
{
    strength += v.strength;
    vital += v.vital;
    dexterity += v.dexterity;
    agility += v.agility;
    intelligence += v.intelligence;
    mentality += v.mentality;
    luck += v.luck;
}

void CreatureStat::WriteToPacket(TS_SC_STAT_INFO &packet)
{
    packet.stat.stat_id      = stat_id;
    packet.stat.strength     = (strength > 32000 ? 32000 : strength);
    packet.stat.vital        = (vital > 32000 ? 32000 : vital);
    packet.stat.dexterity    = (dexterity > 32000 ? 32000 : dexterity);
    packet.stat.agility      = (agility > 32000 ? 32000 : agility);
    packet.stat.intelligence = (intelligence > 32000 ? 32000 : intelligence);
    packet.stat.mentality    = (mentality > 32000 ? 32000 : mentality);
    packet.stat.luck         = (luck > 32000 ? 32000 : luck);
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

void CreatureAtributeServer::Copy(const CreatureAtributeServer &v)
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

void CreatureAtributeServer::WriteToPacket(TS_SC_STAT_INFO &packet)
{
    packet.attribute.nCritical          = static_cast<int16_t>(nCritical > 32000 ? 32000 : nCritical);
    packet.attribute.nCriticalPower     = static_cast<int16_t>(nCriticalPower > 32000 ? 32000 : nCriticalPower);
    packet.attribute.nAttackPointRight  = static_cast<int16_t>(nAttackPointRight > 32000 ? 32000 : nAttackPointRight);
    packet.attribute.nAttackPointLeft   = static_cast<int16_t>(nAttackPointLeft > 32000 ? 32000 : nAttackPointLeft);
    packet.attribute.nDefence           = static_cast<int16_t>(nDefence > 32000 ? 32000 : nDefence);
    packet.attribute.nBlockDefence      = static_cast<int16_t>(nBlockDefence > 32000 ? 32000 : nBlockDefence);
    packet.attribute.nMagicPoint        = static_cast<int16_t>(nMagicPoint > 32000 ? 32000 : nMagicPoint);
    packet.attribute.nMagicDefence      = static_cast<int16_t>(nMagicDefence > 32000 ? 32000 : nMagicDefence);
    packet.attribute.nAccuracyRight     = static_cast<int16_t>(nAccuracyRight > 32000 ? 32000 : nAccuracyRight);
    packet.attribute.nAccuracyLeft      = static_cast<int16_t>(nAccuracyLeft > 32000 ? 32000 : nAccuracyLeft);
    packet.attribute.nMagicAccuracy     = static_cast<int16_t>(nMagicAccuracy > 32000 ? 32000 : nMagicAccuracy);
    packet.attribute.nAvoid             = static_cast<int16_t>(nAvoid > 32000 ? 32000 : nAvoid);
    packet.attribute.nMagicAvoid        = static_cast<int16_t>(nMagicAvoid > 32000 ? 32000 : nMagicAvoid);
    packet.attribute.nBlockChance       = static_cast<int16_t>(nBlockChance > 32000 ? 32000 : nBlockChance);
    packet.attribute.nMoveSpeed         = static_cast<int16_t>(nMoveSpeed > 32000 ? 32000 : nMoveSpeed);
    packet.attribute.nAttackSpeed       = static_cast<int16_t>(nAttackSpeedRight > 32000 ? 32000 : nAttackSpeedRight);
    packet.attribute.nAttackRange       = static_cast<int16_t>(nAttackRange > 32000 ? 32000 : nAttackRange);
    packet.attribute.nMaxWeight         = static_cast<int16_t>nMaxWeight;
    packet.attribute.nCastingSpeed      = static_cast<int16_t>(nCastingSpeed > 32000 ? 32000 : nCastingSpeed);
    packet.attribute.nCoolTimeSpeed     = static_cast<int16_t>(nCoolTimeSpeed > 32000 ? 32000 : nCoolTimeSpeed);
    packet.attribute.nItemChance        = static_cast<int16_t>(nItemChance > 32000 ? 32000 : nItemChance);
    packet.attribute.nHPRegenPercentage = static_cast<int16_t>(nHPRegenPercentage > 32000 ? 32000 : nHPRegenPercentage);
    packet.attribute.nHPRegenPoint      = static_cast<int16_t>(nHPRegenPoint > 32000 ? 32000 : nHPRegenPoint);
    packet.attribute.nMPRegenPercentage = static_cast<int16_t>(nMPRegenPercentage > 32000 ? 32000 : nMPRegenPercentage);
    packet.attribute.nMPRegenPoint      = static_cast<int16_t>(nMPRegenPoint > 32000 ? 32000 : nMPRegenPoint);
}