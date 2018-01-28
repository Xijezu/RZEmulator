#ifndef PROJECT_DAMAGETEMPLATE_H
#define PROJECT_DAMAGETEMPLATE_H

#include "Common.h"
#include "ItemFields.h"

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
    FLAG_PERFECT_BLOCK = 1,
    FLAG_BLOCK         = 2,
    FLAG_MISS          = 4,
    FLAG_CRITICAL      = 8
};

enum AttackAction : int
{
    ATTACK_END    = 1,
    ATTACK_AIMING = 2,
    ATTACK_ATTACK = 3,
    ATTACK_CANCEL = 4
};

enum AttackFlag : int
{
    ATTACK_FLAG_BOW           = 1,
    ATTACK_FLAG_CROSS_BOW     = 2,
    ATTACK_FLAG_DOUBLE_WEAPON = 4,
    ATTACK_FLAG_DOUBLE_ATTACK = 8
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

 struct AdditionalDamageInfo
 {
     AdditionalDamageInfo(uint8 _ratio, ElementalType _require_type, ElementalType _type, uint16 _nDamage, float _fDamage)
     {
         ratio        = _ratio;
         require_type = _require_type;
         type         = _type;
         nDamage      = _nDamage;
         fDamage      = _fDamage;
     }

     AdditionalDamageInfo(uint8 _ratio, ElementalType _require_type, ElementalType _type, float _fDamage)
     {
         ratio        = _ratio;
         require_type = _require_type;
         type         = _type;
         nDamage      = (uint16)_fDamage;
         fDamage      = _fDamage;
     }

     uint8         ratio;
     ElementalType require_type;
     ElementalType type;
     uint16        nDamage;
     float         fDamage;
 };

#endif // PROJECT_DAMAGETEMPLATE_H
