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

#include "Common.h"

struct TS_SC_STAT_INFO;

struct CreatureElementalResist
{
    uint16 nResist[7];

    void Reset(uint16 v)
    {
        for (unsigned short &i : nResist)
        {
            i = v;
        }
    }
};

struct ExpertMod
{
    float fAvoid;
    float fDamage;

    void Reset(float v)
    {
        fAvoid  = v;
        fDamage = v;
    }
};

struct DamageReduceInfo
{
    // Function       :     public void StructCreature::DamageReduceInfo::DamageReduceInfo(unsigned char, float, float, float, int, int, int, int, int)
    bool IsAppliableCreatureGroup(int creature_group)
    {
        return apply_creature_group_list[0] == 99 || apply_creature_group_list[0] == creature_group
               || apply_creature_group_list[1] == 99 || apply_creature_group_list[1] == creature_group
               || apply_creature_group_list[2] == 99 || apply_creature_group_list[2] == creature_group
               || apply_creature_group_list[3] == 99 || apply_creature_group_list[3] == creature_group
               || apply_creature_group_list[4] == 99 || apply_creature_group_list[4] == creature_group;
    }

    uint8 ratio;
    float physical_reduce;
    float physical_skill_reduce;
    float magical_skill_reduce;
    int   apply_creature_group_list[5];
};

struct StateMod
{
    StateMod()
    {
        fDamage   = 1.0f;
        nDamage   = 0;
        fCritical = 1.0f;
        nCritical = 0;
        fHate     = 1.0f;
    }

    StateMod(const StateMod &src)
    {
        fDamage   = src.fDamage;
        nDamage   = src.nDamage;
        fCritical = src.fCritical;
        nCritical = src.nCritical;
        fHate     = src.fHate;
    }

    void Copy(const StateMod &src)
    {
        fDamage   = src.fDamage;
        nDamage   = src.nDamage;
        fCritical = src.fCritical;
        nCritical = src.nCritical;
        fHate     = src.fHate;
    }

    float fDamage;
    int   nDamage;
    float fCritical;
    int   nCritical;
    float fHate;
};

class CreatureStat
{
    public:
        CreatureStat() { Reset(0); }

        void Reset(int16_t);

        void Copy(const CreatureStat &);

        void Add(const CreatureStat &);

        void WriteToPacket(TS_SC_STAT_INFO &);

        short stat_id;
        float strength;
        float vital;
        float dexterity;
        float agility;
        float intelligence;
        float mentality;
        float luck;
};

class CreatureStatAmplifier
{
    public:
        float stat_id;
        float strength;
        float vital;
        float dexterity;
        float agility;
        float intelligence;
        float mentality;
        float luck;

//         CreatureStatAmplifier(cCreatureStatAmplifier &)
        // CreatureStatAmplifier()
        // CreatureStatAmplifier & operator=(const struct CreatureStatAmplifier &)
        void Reset(float v)
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
};

class CreatureAttributeAmplifier
{
    public:
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
//         void CreatureAttributeAmplifier(const struct CreatureAttributeAmplifier &)
//         void CreatureAttributeAmplifier::CreatureAttributeAmplifier()
//         struct CreatureAttributeAmplifier & operator=(const struct CreatureAttributeAmplifier &)

        void Reset(float v)
        {
            fCritical                    = v;
            fCriticalPower               = v;
            fAttackPointRight            = v;
            fAttackPointLeft             = v;
            fDefence                     = v;
            fBlockDefence                = v;
            fMagicPoint                  = v;
            fMagicDefence                = v;
            fAccuracyRight               = v;
            fAccuracyLeft                = v;
            fMagicAccuracy               = v;
            fAvoid                       = v;
            fMagicAvoid                  = v;
            fBlockChance                 = v;
            fMoveSpeed                   = v;
            fAttackSpeed                 = v;
            fAttackRange                 = v;
            fMaxWeight                   = v;
            fCastingSpeed                = v;
            fCoolTimeSpeed               = v;
            fItemChance                  = v;
            fHPRegenPercentage           = v;
            fHPRegenPoint                = v;
            fMPRegenPercentage           = v;
            fMPRegenPoint                = v;
            fAttackSpeedRight            = v;
            fAttackSpeedLeft             = v;
            fDoubleAttackRatio           = v;
            fStunResistance              = v;
            fMoveSpeedDecreaseResistance = v;
            fHPAdd                       = v;
            fMPAdd                       = v;
            fHPAddByItem                 = v;
            fMPAddByItem                 = v;

        }
};

class CreatureAtribute
{
    public:
        CreatureAtribute() = default;

        ~CreatureAtribute() = default;

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
};

class CreatureAtributeServer : public CreatureAtribute
{
    public:
        CreatureAtributeServer() = default;

        void Reset(int16_t);

        void Copy(const CreatureAtributeServer &);

        void WriteToPacket(TS_SC_STAT_INFO &packet);

        float nAttackSpeedRight;
        float nAttackSpeedLeft;
        float nDoubleAttackRatio;
        float nStunResistance;
        float nMoveSpeedDecreaseResistance;
        float nHPAdd;
        float nMPAdd;
        float nHPAddByItem;
        float nMPAddByItem;
};

class CreatureElementalResistAmplifier
{
    public:
        float fResist[7]{0};

// Function       :   void CreatureElementalResistAmplifier(const struct CreatureElementalResistAmplifier &)
// Function       :   void CreatureElementalResistAmplifier()
// Function       :   struct CreatureElementalResistAmplifier & operator=(const struct CreatureElementalResistAmplifier &)
        void Reset(float v)
        {
            for (float &i : fResist)
            {
                i = v;
            }
        }
};