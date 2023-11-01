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
#include "StateBase.h"
#include "ItemTemplate.hpp"
#include "TS_SC_STAT_INFO.h"

struct TS_SC_STAT_INFO;

struct CreatureElementalResist {
    uint16_t nResist[ElementalType::TYPE_COUNT];
};

struct ExpertMod {
    float fAvoid;
    float fDamage;

    void Reset(float v)
    {
        fAvoid = v;
        fDamage = v;
    }
};

struct DamageReduceInfo {
    DamageReduceInfo(unsigned char _ratio, float _physical_reduce, float _physical_skill_reduce, float _magical_skill_reduce, int32_t _apply_creature_group_1, int32_t _apply_creature_group_2,
        int32_t _apply_creature_group_3, int32_t _apply_creature_group_4, int32_t _apply_creature_group_5)
        : ratio(_ratio)
        , physical_reduce(_physical_reduce)
        , physical_skill_reduce(_physical_skill_reduce)
        , magical_skill_reduce(_magical_skill_reduce)
    {
        apply_creature_group_list[0] = _apply_creature_group_1;
        apply_creature_group_list[1] = _apply_creature_group_2;
        apply_creature_group_list[2] = _apply_creature_group_3;
        apply_creature_group_list[3] = _apply_creature_group_4;
        apply_creature_group_list[4] = _apply_creature_group_5;
    }

    inline bool IsAppliableCreatureGroup(int32_t creature_group)
    {
        return (apply_creature_group_list[0] == 99 || apply_creature_group_list[0] == creature_group) || (apply_creature_group_list[1] == 99 || apply_creature_group_list[1] == creature_group) ||
            (apply_creature_group_list[2] == 99 || apply_creature_group_list[2] == creature_group) || (apply_creature_group_list[3] == 99 || apply_creature_group_list[3] == creature_group) ||
            (apply_creature_group_list[4] == 99 || apply_creature_group_list[4] == creature_group);
    }

    unsigned char ratio;
    float physical_reduce;
    float physical_skill_reduce;
    float magical_skill_reduce;
    int32_t apply_creature_group_list[5];
};

struct StateMod {
    StateMod() { Init(); }

    void Init()
    {
        fDamage = 1.0f;
        nDamage = 0;
        fCritical = 1.0f;
        nCritical = 0;
        fHate = 1.0f;
    }

    float fDamage;
    int32_t nDamage;
    float fCritical;
    int32_t nCritical;
    float fHate;
};

struct ElementalSkillStateMod {
    ElementalSkillStateMod() { Init(); }

    void Init()
    {
        fMagicalDamage = 1.0f;
        fPhysicalDamage = 1.0f;
        nElementalType = 0;
        fCooltime = 1.0f;
        fCastingSpeed = 1.0f;
        fHate = 1.0f;
        nMagicalAccuracy = 0;
        nPhysicalAccuracy = 0;
        nCritical = 0;
        fManaCostRatio = 1.0f;
        vExhaustiveStateCode.clear();
        nCastingSpeedApplyTime = 0;
    }

    float fManaCostRatio;
    float fCooltime;
    float fCastingSpeed;
    float fHate;
    int32_t nMagicalAccuracy;
    int32_t nPhysicalAccuracy;
    int32_t nCritical;

    float fMagicalDamage;
    float fPhysicalDamage;
    int32_t nElementalType;
    uint32_t nCastingSpeedApplyTime;

    std::vector<StateCode> vExhaustiveStateCodeForDelete{};
    std::vector<StateCode> vExhaustiveStateCode{};
};

struct CreatureStat {

    CreatureStat() = default;
    constexpr CreatureStat& operator=(const CreatureStat&) = default;
	CreatureStat( const CreatureStat & stat )
		: stat_id( stat.stat_id )
		, strength( stat.strength )
		, vital( stat.vital )
		, dexterity( stat.dexterity )
		, agility( stat.agility )
		, intelligence( stat.intelligence )
		, mentality( stat.mentality )
		, luck( stat.luck )
	{}

    CreatureStat operator+(const CreatureStat &creatureStat)
    {
        strength += creatureStat.strength;
        vital += creatureStat.vital;
        dexterity += creatureStat.dexterity;
        agility += creatureStat.agility;
        intelligence += creatureStat.intelligence;
        mentality += creatureStat.mentality;
        luck += creatureStat.luck;
        return *this;
    }

    void WriteToPacket(TS_SC_STAT_INFO &packet)
    {
        packet.stat.stat_id = stat_id;
        packet.stat.strength = (strength > 32000 ? 32000 : strength);
        packet.stat.vital = (vital > 32000 ? 32000 : vital);
        packet.stat.dexterity = (dexterity > 32000 ? 32000 : dexterity);
        packet.stat.agility = (agility > 32000 ? 32000 : agility);
        packet.stat.intelligence = (intelligence > 32000 ? 32000 : intelligence);
        packet.stat.mentality = (mentality > 32000 ? 32000 : mentality);
        packet.stat.luck = (luck > 32000 ? 32000 : luck);
    }

    short stat_id;
    float strength;
    float vital;
    float dexterity;
    float agility;
    float intelligence;
    float mentality;
    float luck;
};

struct CreatureStatAmplifier {
    float stat_id;
    float strength;
    float vital;
    float dexterity;
    float agility;
    float intelligence;
    float mentality;
    float luck;
};

struct CreatureAttributeAmplifier {
    float fCritical;
    float fCriticalPower;
    float fAttackPointRight;
    float fAttackPointLeft;
    float fDefence;
    float fBlockDefence;
    float fMagicPoint;
    float fMagicDefence;
    float fAccuracyRight;
    float fAccuracyLeft;
    float fMagicAccuracy;
    float fAvoid;
    float fMagicAvoid;
    float fBlockChance;
    float fMoveSpeed;
    float fAttackSpeed;
    float fAttackRange;
    float fMaxWeight;
    float fCastingSpeed;
    float fCoolTimeSpeed;
    float fItemChance;
    float fHPRegenPercentage;
    float fHPRegenPoint;
    float fMPRegenPercentage;
    float fMPRegenPoint;
    float fAttackSpeedRight;
    float fAttackSpeedLeft;
    float fDoubleAttackRatio;
    float fStunResistance;
    float fMoveSpeedDecreaseResistance;
    float fHPAdd;
    float fMPAdd;
    float fHPAddByItem;
    float fMPAddByItem;
};

struct CreatureAtributeServer {

	float nCritical;			
	float nCriticalPower;		
	float nAttackPointRight;	
	float nAttackPointLeft;	
	float nDefence;			
	float nBlockDefence;		
	float nMagicPoint;		
	float nMagicDefence;		
	float nAccuracyRight;		
	float nAccuracyLeft;		
	float nMagicAccuracy;		
	float nAvoid;				
	float nMagicAvoid;		
	float nBlockChance;		
	float nMoveSpeed;			
	float nAttackSpeed;		
	float nAttackRange;		
	float nMaxWeight;			
	float nCastingSpeed;		
	float nCoolTimeSpeed;		
	float nItemChance;		
	float nHPRegenPercentage;	
	float nHPRegenPoint;		
	float nMPRegenPercentage;	
	float nMPRegenPoint;		

	float nAttackSpeedRight;	
	float nAttackSpeedLeft;	
	float nDoubleAttackRatio;
	float nStunResistance;
	float nMoveSpeedDecreaseResistance;
	float nHPAdd;
	float nMPAdd;
	float nHPAddByItem;
	float nMPAddByItem;

    CreatureAtributeServer() = default;
    constexpr CreatureAtributeServer& operator=(const CreatureAtributeServer&) = default;
    CreatureAtributeServer(const CreatureAtributeServer &attr)
		: nCritical( attr.nCritical )
		, nCriticalPower( attr.nCriticalPower )
		, nAttackPointRight( attr.nAttackPointRight )
		, nAttackPointLeft( attr.nAttackPointLeft )
		, nDefence( attr.nDefence )
		, nBlockDefence( attr.nBlockDefence )
		, nMagicPoint( attr.nMagicPoint )
		, nMagicDefence( attr.nMagicDefence )
		, nAccuracyRight( attr.nAccuracyRight )
		, nAccuracyLeft( attr.nAccuracyLeft )
		, nMagicAccuracy( attr.nMagicAccuracy )
		, nAvoid( attr.nAvoid )
		, nMagicAvoid( attr.nMagicAvoid )
		, nBlockChance( attr.nBlockChance )
		, nMoveSpeed( attr.nMoveSpeed )
		, nAttackSpeed( attr.nAttackSpeed )
		, nAttackRange( attr.nAttackRange )
		, nMaxWeight( attr.nMaxWeight )
		, nCastingSpeed( attr.nCastingSpeed )
		, nCoolTimeSpeed( attr.nCoolTimeSpeed )
		, nItemChance( attr.nItemChance )
		, nHPRegenPercentage( attr.nHPRegenPercentage )
		, nHPRegenPoint( attr.nHPRegenPoint )
		, nMPRegenPercentage( attr.nMPRegenPercentage )
		, nMPRegenPoint( attr.nMPRegenPoint )	

		, nAttackSpeedRight( 0 )
		, nAttackSpeedLeft( 0 )
		, nDoubleAttackRatio( 0 )
		, nStunResistance( 0 )
		, nMoveSpeedDecreaseResistance( 0 )
		, nHPAdd( 0 )
		, nMPAdd( 0 )
		, nHPAddByItem( 0 )
		, nMPAddByItem( 0 )
    {}

    void WriteToPacket(TS_SC_STAT_INFO &packet)
    {
        packet.attribute.nCritical = static_cast<int16_t>(nCritical > 32000 ? 32000 : nCritical);
        packet.attribute.nCriticalPower = static_cast<int16_t>(nCriticalPower > 32000 ? 32000 : nCriticalPower);
        packet.attribute.nAttackPointRight = static_cast<int16_t>(nAttackPointRight > 32000 ? 32000 : nAttackPointRight);
        packet.attribute.nAttackPointLeft = static_cast<int16_t>(nAttackPointLeft > 32000 ? 32000 : nAttackPointLeft);
        packet.attribute.nDefence = static_cast<int16_t>(nDefence > 32000 ? 32000 : nDefence);
        packet.attribute.nBlockDefence = static_cast<int16_t>(nBlockDefence > 32000 ? 32000 : nBlockDefence);
        packet.attribute.nMagicPoint = static_cast<int16_t>(nMagicPoint > 32000 ? 32000 : nMagicPoint);
        packet.attribute.nMagicDefence = static_cast<int16_t>(nMagicDefence > 32000 ? 32000 : nMagicDefence);
        packet.attribute.nAccuracyRight = static_cast<int16_t>(nAccuracyRight > 32000 ? 32000 : nAccuracyRight);
        packet.attribute.nAccuracyLeft = static_cast<int16_t>(nAccuracyLeft > 32000 ? 32000 : nAccuracyLeft);
        packet.attribute.nMagicAccuracy = static_cast<int16_t>(nMagicAccuracy > 32000 ? 32000 : nMagicAccuracy);
        packet.attribute.nAvoid = static_cast<int16_t>(nAvoid > 32000 ? 32000 : nAvoid);
        packet.attribute.nMagicAvoid = static_cast<int16_t>(nMagicAvoid > 32000 ? 32000 : nMagicAvoid);
        packet.attribute.nBlockChance = static_cast<int16_t>(nBlockChance > 32000 ? 32000 : nBlockChance);
        packet.attribute.nMoveSpeed = static_cast<int16_t>(nMoveSpeed > 32000 ? 32000 : nMoveSpeed);
        packet.attribute.nAttackSpeed = static_cast<int16_t>(nAttackSpeedRight > 32000 ? 32000 : nAttackSpeedRight);
        packet.attribute.nAttackRange = static_cast<int16_t>(nAttackRange > 32000 ? 32000 : nAttackRange);
        packet.attribute.nMaxWeight = static_cast<int16_t>(nMaxWeight);
        packet.attribute.nCastingSpeed = static_cast<int16_t>(nCastingSpeed > 32000 ? 32000 : nCastingSpeed);
        packet.attribute.nCoolTimeSpeed = static_cast<int16_t>(nCoolTimeSpeed > 32000 ? 32000 : nCoolTimeSpeed);
        packet.attribute.nItemChance = static_cast<int16_t>(nItemChance > 32000 ? 32000 : nItemChance);
        packet.attribute.nHPRegenPercentage = static_cast<int16_t>(nHPRegenPercentage > 32000 ? 32000 : nHPRegenPercentage);
        packet.attribute.nHPRegenPoint = static_cast<int16_t>(nHPRegenPoint > 32000 ? 32000 : nHPRegenPoint);
        packet.attribute.nMPRegenPercentage = static_cast<int16_t>(nMPRegenPercentage > 32000 ? 32000 : nMPRegenPercentage);
        packet.attribute.nMPRegenPoint = static_cast<int16_t>(nMPRegenPoint > 32000 ? 32000 : nMPRegenPoint);
    }
};

struct CreatureElementalResistAmplifier {
    float fResist[ElementalType::TYPE_COUNT]{0};
};