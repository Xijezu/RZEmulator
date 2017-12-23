#ifndef PROJECT_CREATUREATTRIBUTE_H
#define PROJECT_CREATUREATTRIBUTE_H

#include "Common.h"

class XPacket;

class CreatureStat {
public:
    CreatureStat()
    { Reset(0); }

    void Reset(int16_t);

    void Copy(CreatureStat);

    void Add(CreatureStat);

    void WriteToPacket(XPacket &);

    short stat_id;
    float strength;
    float vital;
    float dexterity;
    float agility;
    float intelligence;
    float mentality;
    float luck;
};

class CreatureStatAmplifier {
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

class CreatureAttributeAmplifier {
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

class CreatureAtribute {
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

class CreatureAtributeServer : public CreatureAtribute {
public:
    CreatureAtributeServer() = default;

    void Reset(int16_t);

    void Copy(CreatureAtributeServer);

    void WriteToPacket(XPacket &packet);

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

class CreatureElementalResistAmplifier {
public:
    float fResist[7]{0};

// Function       :   void CreatureElementalResistAmplifier(const struct CreatureElementalResistAmplifier &)
// Function       :   void CreatureElementalResistAmplifier()
// Function       :   struct CreatureElementalResistAmplifier & operator=(const struct CreatureElementalResistAmplifier &)
    void Reset(float v)
    {
        for (int i = 0; i < 7; ++i) {
            fResist[i] = v;
        }
    }
};


#endif // PROJECT_CREATUREATTRIBUTE_H
