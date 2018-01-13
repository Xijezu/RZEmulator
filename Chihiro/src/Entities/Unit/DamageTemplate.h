#ifndef PROJECT_DAMAGETEMPLATE_H
#define PROJECT_DAMAGETEMPLATE_H

#include "Common.h"


struct HateModifier {
    // Function       :     public void StructCreature::HateModifier::HateModifier(int, int, float, int)
    bool  bIsApplyToPhysicalSkill;
    bool  bIsApplyToMagicalSkill;
    bool  bIsApplyToPhysicalAttack;
    bool  bIsApplyToHarmful;
    bool  bIsApplyToHelpful;
    float fAmpValue;
    int   nIncValue;
};

enum AFlag : int
{
    AF_PerfectBlock = 1,
    AF_Block = 2,
    AF_Miss = 4,
    AF_Critical = 8
};

enum AttackAction : int
{
    AA_End = 1,
    AA_Aiming = 2,
    AA_Attack = 3,
    AA_Cancel = 4
};

enum AttackFlag : int
{
    AF_Bow = 1,
    AF_CrossBow = 2,
    AF_DoubleWeapon = 4,
    AF_DoubleAttack = 8
};

struct Damage {
    // UserDefinedType:   _DAMAGE
    // Function       :     public void _DAMAGE()
    int  nDamage;
    int  nResistedDamage;
    bool bCritical;
    bool bMiss;
    bool bBlock;
    bool bPerfectBlock;
    int  target_hp;
};

struct DamageInfo : public Damage {
    // Function       :     public void StructCreature::_DAMAGE_INFO::_DAMAGE_INFO()
    void SetDamage(const Damage& damage)
    {
        nDamage       = damage.nDamage;
        bCritical     = damage.bCritical;
        bMiss         = damage.bMiss;
        bBlock        = damage.bBlock;
        bPerfectBlock = damage.bPerfectBlock;
        target_hp     = damage.target_hp;
    }

    uint16 elemental_damage[7];
};

struct AttackInfo : public DamageInfo {
    // Function       :     public void StructCreature::_ATTACK_INFO::_ATTACK_INFO()
    void SetDamageInfo(const DamageInfo& damage_info)
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

    short  mp_damage;
    short  attacker_damage;
    short  attacker_mp_damage;
    ushort target_mp;
    uint   attacker_hp;
    ushort attacker_mp;
};

#endif // PROJECT_DAMAGETEMPLATE_H
