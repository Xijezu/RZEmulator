#ifndef PROJECT_SKILLBASE_H
#define PROJECT_SKILLBASE_H

#include "Common.h"
#include "ItemFields.h"

enum SR_ResultType : int
{
    SRT_Damage = 0,
    SRT_MagicDamage = 1,
    SRT_DamageWithKnockBack = 2,
    SRT_Result = 10,
    SRT_AddHP = 20,
    SRT_AddMP = 21,
    SRT_AddHPMPSP = 22,
    SRT_Rebirth = 23,
    SRT_Rush = 30,
    SRT_NotUse = 100,
};

struct SkillDamage {
    uint8 type;
    uint hTarget;
    int target_hp;
    uint8 damage_type;
    int damage;
    int flag;
    uint16 elemental_damage[7]{};
};

struct AddHPType {
    uint8 type;
    uint hTarget;
    int target_hp;
    int nIncHP;
};

struct SR_Result {
    bool bResult;
    int success_type;
};

struct SkillResult {
    uint8 type;
    uint hTarget;

    // Damage
    int target_hp;
    uint8 damage_type;
    SR_Result result;
    SkillDamage damage;
    AddHPType addHPType;
};

enum SkillState : short {
    ST_Fire          = 0,
    ST_Casting       = 1,
    ST_CastingUpdate = 2,
    ST_Cancel        = 3,
    ST_RegionFire    = 4,
    ST_Complete      = 5
};

enum TargetType : short {
    TT_Misc                          = 0,
    Target                           = 1,
    RegionWithTarget                 = 2,
    RegionWithoutTarget              = 3,
    TT_Region                        = 4,
    TargetExceptCaster               = 6,
    Party                            = 21,
    Guild                            = 22,
    AttackTeam                       = 23,
    TT_Summon                        = 31,
    PartySummon                      = 32,
    RegionNearMainSummonWithoutTaget = 44,
    SelfWithSummon                   = 45,
    PartyWithSummon                  = 51,
    Master                           = 101,
    SelfWithMaster                   = 102,
    CreatureTypeNone                 = 201,
    CreatureTypeFire                 = 202,
    CreatureTypeWater                = 203,
    CreatureTypeWind                 = 204,
    CreatureTypeEarth                = 205,
    CreatureTypeLight                = 206,
    CreatureTypeDark                 = 207,
};

enum SkillId : int {
    AdvWeaponExpert                       = 1022,
    LifeOfDarkness                        = 1033,
    IncreaseEnergy                        = 1082,
    DualSwordExpert                       = 1181,
    ArmorMastery                          = 1201,
    CreatureControl                       = 1801,
    CreatureMastery                       = 1811,
    TechnicalCreatureControl              = 1881,
    GaiaArmorMastery                      = 0x4B2,
    RaiseExp                              = 0x2AFA,
    RaiseJP                               = 0x2AFB,
    GaiaForceSaving                       = 0xA47,
    ItemResurrectionScroll                = 0x1771,
    ItemRegenerationScroll                = 0x1772,
    ItemHealingScroll                     = 0x1773,
    ItemManaRecoveryScroll                = 0x1774,
    ItemAntidoteScroll                    = 0x1775,
    ItemRechargeScroll                    = 0x1776,
    TownPortal                            = 0x1777,
    Return                                = 0xEDA,
    ForceChip                             = 0x1778,
    SoulChip                              = 0x1779,
    LunaChip                              = 0x177A,
    ItemPerfectCreatureResurrectionScroll = 0x177D,
    ItemPieceOfStrength                   = 0x177E,
    ItemPieceOfVitality                   = 0x177F,
    ItemPieceOfDexterity                  = 0x1780,
    ItemPieceOfAgility                    = 0x1781,
    ItemPieceOfIntelligence               = 0x1782,
    ItemPieceOfMentality                  = 0x1783,
    ReturnFeather                         = 0x1784,
    ReturnBackFeather                     = 0x1785,
    FireBombPhysical                      = -727,
    FireBombMagical                       = -726,
//             0x1786,SKILL_ITEM_REGENERATION_SCROLL_LV1
//             0x1787,SKILL_ITEM_REGENERATION_SCROLL_LV2
//             0x1788,SKILL_ITEM_REGENERATION_SCROLL_LV3
//             0x1789,SKILL_ITEM_REGENERATION_SCROLL_LV4
//             0x178A,SKILL_ITEM_REGENERATION_SCROLL_LV5
//             0x178B,SKILL_ITEM_REGENERATION_SCROLL_LV6
//             0x178C,SKILL_ITEM_REGENERATION_SCROLL_LV7
//             0x178D,SKILL_ITEM_REGENERATION_SCROLL_LV8
//             0x178E,SKILL_ITEM_HEALING_SCROLL_LV1
//             0x178F,SKILL_ITEM_HEALING_SCROLL_LV2
//             0x1790,SKILL_ITEM_HEALING_SCROLL_LV3
//             0x1791,SKILL_ITEM_HEALING_SCROLL_LV4
//             0x1792,SKILL_ITEM_HEALING_SCROLL_LV5
//             0x1793,SKILL_ITEM_HEALING_SCROLL_LV6
//             0x1794,SKILL_ITEM_HEALING_SCROLL_LV7
//             0x1795,SKILL_ITEM_HEALING_SCROLL_LV8
//             0x1796,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV1
//             0x1797,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV2
//             0x1798,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV3
//             0x1799,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV4
//             0x179A,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV5
//             0x179B,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV6
//             0x179C,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV7
//             0x179D,SKILL_ITEM_MANA_RECOVERY_SCROLL_LV8
//             0x179E,SKILL_CALL_BLACK_PINE_TEA
//             0x179F,SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV1
//             0x17A0,SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV2
//             0x17A1,SKILL_ITEM_SUMMON_SPEED_UP_SCROLL_LV3
//             0x17AD,SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH
//             0x17AE,SKILL_ITEM_ALTERED_PIECE_OF_VITALITY
//             0x17AF,SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY
//             0x17B0,SKILL_ITEM_ALTERED_PIECE_OF_AGILITY
//             0x17B1,SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE
//             0x17B2,SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY
//             0x17B3,SKILL_ITEM_ALTERED_PIECE_OF_STRENGTH_QUEST
//             0x17B4,SKILL_ITEM_ALTERED_PIECE_OF_VITALITY_QUEST
//             0x17B5,SKILL_ITEM_ALTERED_PIECE_OF_DEXTERITY_QUEST
//             0x17B6,SKILL_ITEM_ALTERED_PIECE_OF_AGILITY_QUEST
//             0x17B7,SKILL_ITEM_ALTERED_PIECE_OF_INTELLIGENCE_QUEST
//             0x17B8,SKILL_ITEM_ALTERED_PIECE_OF_MENTALITY_QUEST
//             0x1AF5,SKILL_COLLECTING
//             0x1AF6,SKILL_MINING
//             0x1AF7,SKILL_OPERATING
//             0x1AF8,SKILL_ACTIVATING
//             0x1AF9,SKILL_OPERATE_DEVICE
//             0x1AFA,SKILL_OPERATE_DUNGEON_CORE
//             0x1AFD,SKILL_OPERATE_EXPLORATION
//             0xFFFFFD26,SKILL_TREE_OF_HEALING_TYPE_A
//             0xFFFFFD27,SKILL_TREE_OF_HEALING_TYPE_B
    Shoveling                             = 6907,
    PetTaming                             = 6908,
    CreatureTaming                        = 4003,
//             0x2719,SKILL_THROW_SMALL_SNOWBALL
//             0x271A,SKILL_THROW_BIG_SNOWBALL
//             0x2AF9,SKILL_CREATURE_RIDING
//             0x2EE1,SKILL_UNIT_EXPERT_LV2
//             0x2EE2,SKILL_UNIT_EXPERT_LV3
//             0x2EE3,SKILL_UNIT_EXPERT_LV4
//             0x2EE4,SKILL_UNIT_EXPERT_LV5
//             0x2EE5,SKILL_UNIT_EXPERT_LV6
//             0x36B1,SKILL_AMORY_UNIT
    TwinBladeExpert                       = 61010,
//             0xFFFFEE52,SKILL_TWIN_BLADE_EXPERT
    TwinAxeExpert                         = 61015,
//             0xFFFFFD28,SKILL_NEW_YEAR_CHAMPAGNE
//             0xFFFFFD2D,SKILL_NAMUIR_LEAF_POISON
//             0xFFFFFD2E,SKILL_NAMUIR_RIND_BLEEDING
};

enum EffectType : int {
    ET_Misc                                       = 0,
    Unk1                                          = 1,
    RespawnMonsterNear                            = 2,
    SummonScroll                                  = 6,
    PhysicalSingleDamageT1                        = 101,
    PhysicalMultipleDamageT1                      = 102,
    PhysicalSingleDamageT2                        = 103,
    PhysicalMultipleDamageT2                      = 104,
    PhysicalDirectionDamage                       = 105,
    PhysicalSingleDamageT3                        = 106,
    PhysicalMultipleDamageT3                      = 107,
    PhysicalMultipleDamageTripleAttackOld         = 108,
    PhysicalSingleRegionDamageOld                 = 111,
    PhysicalMultipleRegionDamageOld               = 112,
    PhysicalSingleSpecialRegionDamageOld          = 113,
    PhysicalSingleDamageWithShield                = 117,
    PhysicalAbsorbDamage                          = 121,
    PhysicalMultipleSpecialRegionDamageOld        = 122,
    PhysicalSingleDamageAddEnergyOld              = 125,
    PhysicalSingleDamageKnockbackOld              = 131,
    PhysicalSingleRegionDamageKnockbackOld        = 132,
    PhysicalSingleDamageWithoutWeapnRushKnockBack = 151,
    PhysicalSingleDamageRushKnockBackOld          = 152,
    MagicSingleDamageT1Old                        = 201,
    MagicMultipleDamageT1Old                      = 202,
    MagicSingleDamageT2Old                        = 203,
    MagicMultipleDamageT2Old                      = 204,
    MagicMultipleDamageT3Old                      = 205,
    MagicMultipleDamageT1DealSummonHpOld          = 206,
    MagicSingleRegionDamageOld                    = 211,
    MagicMultipleRegionDamageOld                  = 212,
    MagicSpecialRegionDamageOld                   = 213,
    MagicMultipleRegionDamageT2Old                = 214,
    MagicAbsorbDamageOld                          = 221,
    MagicSingleDamage                             = 231,
    MagicMultipleDamage                           = 232,
    MagicMultipleDamageDealSummonHp               = 233,
    MagicSingleDamageOrDeath                      = 234,
    MagicDamageWithAbsorbHPMP                     = 235,
    MagicSinglePercentDamage                      = 236,
    MagicSinglePercentManaburn                    = 237,
    MagicSingelPercentOfMaxMPManaburn             = 238,
    MagicSingleDamageAddRandomState               = 239,
    MagicalSingleDamageByConsumingTargetsState    = 240,
    MagicMultipleDamageAtOnce                     = 241,
    MagicSingleRegionDamage                       = 261,
    MagicSpecialRegionDamage                      = 262,
    MagicMultipleRegionDamage                     = 263,
    MagicRegionPercentDamage                      = 264,
    MagicSingleRegionDamageUsingCorpse            = 265,
    AddHPMPAbsorbHPMP                             = 266,
    MagicSingleRegionDamageBySummonDead           = 267,
    MagicSingleRegionDamageAddRandomState         = 268,
    MagicMultipleRegionDamageAtOnce               = 269,
    AreaEffectMagicDamage                         = 271,
    AreaEffectMagicDamageAndHeal                  = 272,
    AreaEffectMagicDamageAndHealT2                = 273,
    AddState                                      = 301,
    AddRegionState                                = 302,
    CastingCancelWithAddState                     = 304,
    AddStateBySelfCost                            = 305,
    AddRegionStateBySelfCost                      = 306,
    AddStateByTargetType                          = 307,
// Data           :     constant 0x134, Constant, Type: int, EF_ADD_STATES_WITH_EACH_DIFF_LV
// Data           :     constant 0x135, Constant, Type: int, EF_ADD_STATES_WITH_EACH_DIFF_LV_DURATION
    AddStateByItemCost                            = 314,
// Data           :     constant 0x160, Constant, Type: int, EF_AREA_EFFECT_MAGIC_DAMAGE_OLD
// Data           :     constant 0x161, Constant, Type: int, EF_AREA_EFFECT_HEAL
// Data           :     constant 0x17D, Constant, Type: int, EF_TRAP_PHYSICAL_DAMAGE
// Data           :     constant 0x17E, Constant, Type: int, EF_TRAP_MAGICAL_DAMAGE
// Data           :     constant 0x17F, Constant, Type: int, EF_TRAP_MULTIPLE_PHYSICAL_DAMAGE
// Data           :     constant 0x180, Constant, Type: int, EF_TRAP_MULTIPLE_MAGICAL_DAMAGE
// Data           :     constant 0x191, Constant, Type: int, EF_REMOVE_BAD_STATE
// Data           :     constant 0x192, Constant, Type: int, EF_REMOVE_GOOD_STATE
// Data           :     constant 0x1F5, Constant, Type: int, EF_ADD_HP
// Data           :     constant 0x1F6, Constant, Type: int, EF_ADD_MP
// Data           :     constant 0x1F8, Constant, Type: int, EF_RESURRECTION
// Data           :     constant 0x1F9, Constant, Type: int, EF_ADD_HP_MP
    AddHpMpBySummonDamage                         = 506,// Data           :     constant 0x1FA, Constant, Type: int, EF_ADD_HP_MP_BY_SUMMON_DAMAGE
    AddHpMpBySummonDead                           = 507,
// Data           :     constant 0x1FC, Constant, Type: int, EF_ADD_REGION_HP_MP
// Data           :     constant 0x1FD, Constant, Type: int, EF_ADD_HP_BY_ITEM
// Data           :     constant 0x1FE, Constant, Type: int, EF_ADD_MP_BY_ITEM
// Data           :     constant 0x1FF, Constant, Type: int, EF_CORPSE_ABSORB
    AddHpMpByStealSummonHpMp                      = 512,
// Data           :     constant 0x201, Constant, Type: int, EF_ADD_HP_MP_WITH_LIMIT_PERCENT
// Data           :     constant 0x209, Constant, Type: int, EF_ADD_REGION_HP
// Data           :     constant 0x20A, Constant, Type: int, EF_ADD_REGION_MP
    ET_Summon                                     = 601,
    Unsummon                                      = 602,
    UnsummonAndAddState                           = 605,
    ToggleAura                                    = 701,
    ToggleDifferentialAura                        = 702,
// Data           :     constant 0x384, Constant, Type: int, EF_TAUNT
    RegionTaunt                                   = 901,
    RemoveHate                                    = 902,
    RegionRemoveHate                              = 903,
// Data           :     constant 0x3E9, Constant, Type: int, EF_CORPSE_EXPLOSION
// Data           :     constant 0x2329, Constant, Type: int, EF_CREATE_ITEM
    ActivateFieldProp                             = 9501,
    RegionHealByFieldProp                         = 9502,
    AreaAffectHealByHieldProp                     = 9503,
    WeaponMastery                                 = 10001,
// Data           :     constant 0x2712, Constant, Type: int, EF_BATTLE_PARAMTER_INCREASE
// Data           :     constant 0x2713, Constant, Type: int, EF_BLOCK_INCREASE
// Data           :     constant 0x2714, Constant, Type: int, EF_ATTACK_RANGE_INCREASE
// Data           :     constant 0x2715, Constant, Type: int, EF_RESISTANCE_INCREASE
// Data           :     constant 0x2716, Constant, Type: int, EF_MAGIC_REGISTANCE_INCREASE
// Data           :     constant 0x2717, Constant, Type: int, EF_SPECIALIZE_ARMOR
    IncreaseBaseAttribute                         = 10008,
// Data           :     constant 0x2719, Constant, Type: int, EF_INCREASE_EXTENSION_ATTRIBUTE
// Data           :     constant 0x271A, Constant, Type: int, EF_SPECIALIZE_ARMOR_AMP
// Data           :     constant 0x271B, Constant, Type: int, EF_AMPLIFY_BASE_ATTRIBUTE
// Data           :     constant 0x271C, Constant, Type: int, EF_MAGIC_TRAINING
// Data           :     constant 0x271D, Constant, Type: int, EF_HUNTING_TRAINING
// Data           :     constant 0x271E, Constant, Type: int, EF_BOW_TRAINING
// Data           :     constant 0x271F, Constant, Type: int, EF_INCREASE_STAT
// Data           :     constant 0x2720, Constant, Type: int, EF_AMPLIFY_STAT

    IncreaseHPMP                                = 10021,
// Data           :     constant 0x2726, Constant, Type: int, EF_AMPLIFY_HP_MP
// Data           :     constant 0x2727, Constant, Type: int, EF_HEALING_AMPLIFY
// Data           :     constant 0x2728, Constant, Type: int, EF_HEALING_AMPLIFY_BY_ITEM
// Data           :     constant 0x2729, Constant, Type: int, EF_HEALING_AMPLIFY_BY_REST
// Data           :     constant 0x272A, Constant, Type: int, EF_HATE_AMPLIFY
    IncreaseSummonHPMPSP                        = 10031,
    AmplifySummonHPMPSP                         = 10032,
    CreatureAssignmentIncrease                  = 10033,
// Data           :     constant 0x2732, Constant, Type: int, EF_CREATURE_ACQUIREMENT_INCREASE
// Data           :     constant 0x2733, Constant, Type: int, EF_BELT_ON_PARAMETER_INC
// Data           :     constant 0x2734, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_INC
// Data           :     constant 0x2735, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_EX_INC
// Data           :     constant 0x2736, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_HPMP_INC
// Data           :     constant 0x2739, Constant, Type: int, EF_UNIT_EXPERT
// Data           :     constant 0x273A, Constant, Type: int, EF_BELT_ON_PARAMETER_AMP
// Data           :     constant 0x273B, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_AMP
// Data           :     constant 0x273C, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_EX_AMP
// Data           :     constant 0x273D, Constant, Type: int, EF_BELT_ON_ATTRIBUTE_HPMP_AMP
// Data           :     constant 0x273E, Constant, Type: int, EF_SUMMON_ITEM_EXPERT
// Data           :     constant 0x7531, Constant, Type: int, EF_PHYSICAL_SINGLE_DAMAGE
// Data           :     constant 0x7532, Constant, Type: int, EF_PHYSICAL_SINGLE_DAMAGE_ABSORB
// Data           :     constant 0x7533, Constant, Type: int, EF_PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY
    PhysicalSingleDamageRush                    = 30004,
    PhysicalSingleDamageRushKnockback           = 30005,
// Data           :     constant 0x7536, Constant, Type: int, EF_PHYSICAL_SINGLE_DAMAGE_KNOCKBACK
// Data           :     constant 0x7537, Constant, Type: int, EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK
// Data           :     constant 0x7538, Constant, Type: int, EF_PHYSICAL_SINGLE_REGION_DAMAGE_KNOCKBACK_SELF
// Data           :     constant 0x7539, Constant, Type: int, EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE
// Data           :     constant 0x753A, Constant, Type: int, EF_PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK
// Data           :     constant 0x753B, Constant, Type: int, EF_PHYSICAL_SINGLE_REGION_DAMAGE
// Data           :     constant 0x753C, Constant, Type: int, EF_PHYSICAL_MULTIPLE_REGION_DAMAGE
// Data           :     constant 0x753D, Constant, Type: int, EF_PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE
// Data           :     constant 0x753E, Constant, Type: int, EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE
// Data           :     constant 0x753F, Constant, Type: int, EF_PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE_SELF
// Data           :     constant 0x7540, Constant, Type: int, EF_PHYSICAL_MULTIPLE_DAMAGE
// Data           :     constant 0x7541, Constant, Type: int, EF_PHYSICAL_REALTIME_MULTIPLE_DAMAGE_KNOCKBACK
// Data           :     constant 0x7542, Constant, Type: int, EF_PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE
    PhysicalSingleDamageByConsumingTargetsState = 30019,
// Data           :     constant 0x754E, Constant, Type: int, EF_PHYSICAL_SINGLE_REGION_DAMAGE_WITH_CAST_CANCEL
// Data           :     constant 0x7725, Constant, Type: int, EF_RESURRECTION_WITH_RECOVER
    RemoveStateGroup                            = 30601,
    EF_WEAPON_TRAINING                          = 31001,
// Data           :     constant 0x7919, Constant, Type: int, EF_WEAPON_TRAINING
// Data           :     constant 0x791A, Constant, Type: int, EF_AMPLIFY_BASE_ATTRIBUTE_OLD
// Data           :     constant 0x791B, Constant, Type: int, EF_AMPLIFY_EXT_ATTRIBUTE
    AmplifyExpForSummon                         = 32001,
};

struct SkillTreeBase {
    int   job_id{ };
    int   skill_id{ };
    int   min_skill_lv{ };
    int   max_skill_lv{ };
    int   lv{ };
    int   job_lv{ };
    float jp_ratio{ };
    int   need_skill_id[3]{ };
    int   need_skill_lv[3]{ };
};

struct SkillTreeGroup {
    int                        job_id{ };
    int                        skill_id{ };
    std::vector<SkillTreeBase> skillTrees{ };
};

struct SkillBase {

    int GetNeedJobPoint(int skill_lv)
    {
        int result;

        if (skill_lv <= 50)
            result = this->m_need_jp[skill_lv - 1];
        else
            result = this->m_need_jp[49];
        return result;
    }

    bool IsUseableWeapon(ItemClass cl);

    int GetStateSecond(int skill_lv, int enhance_lv)
    {
        return (int) state_second + (int) enhance_lv * (int) state_second_per_enhance + skill_lv * (int) state_second_per_level;
    }

    int GetHitBonus(int enhance, int level_diff) const {
        return hit_bonus + level_diff * percentage + enhance * hit_bonus_per_enhance;
    }

    int GetStateLevel(int skill_lv, int enhance_lv)
    {
        return (int) (state_level_base
                      + (state_level_per_enhance * enhance_lv)
                      + (state_level_per_skl * skill_lv));
    }

    uint GetCastDelay(int skill_lv, int enhance)
    {
        return (uint)( (float)((float)delay_cast + (float)skill_lv * (float)delay_cast_per_skl ) * (float)(delay_cast_mode_per * (float)enhance + 1.0f));
    }

    uint GetCoolTime(int enhance) const
    {
        return 0;
        return (uint)((delay_cooltime_mode * (float)enhance + 1.0f) * delay_cooltime);
    }

    int m_need_jp[50]{ };

    int     id{ };
    int     text_id{ };
    short   is_valid{ };
    uint8_t elemental{ };
    uint8_t is_passive{ };
    uint8_t is_physical_act{ };
    uint8_t is_harmful{ };
    uint8_t is_need_target{ };
    uint8_t is_corpse{ };
    uint8_t is_toggle{ };
    int     toggle_group{ };
    uint8_t casting_type{ };
    uint8_t casting_level{ };
    int     cast_range{ };
    int     valid_range{ };
    int     cost_hp{ };
    int     cost_hp_per_skl{ };
    int     cost_mp{ };
    int     cost_mp_per_skl{ };
    int     cost_mp_per_enhance{ };
    float   cost_hp_per{ };
    float   cost_hp_per_skl_per{ };
    float   cost_mp_per{ };
    float   cost_mp_per_skl_per{ };
    int     cost_havoc{ };
    int     cost_havoc_per_skl{ };
    float   cost_energy{ };
    float   cost_energy_per_skl{ };
    int     cost_exp{ };
    int     cost_exp_per_enhance{ };
    int     cost_jp{ };
    int     cost_jp_per_enhance{ };
    int     cost_item{ };
    int     cost_item_count{ };
    int     cost_item_count_per{ };
    int     need_level{ };
    int     need_hp{ };
    int     need_mp{ };
    int     need_havoc{ };
    int     need_havoc_burst{ };
    int     need_state_id{ };
    short   need_state_level{ };
    short   need_state_exhaust{ };
    uint8_t vf_one_hand_sword{ };
    uint8_t vf_two_hand_sword{ };
    uint8_t vf_double_sword{ };
    uint8_t vf_dagger{ };
    uint8_t vf_double_dagger{ };
    uint8_t vf_spear{ };
    uint8_t vf_axe{ };
    uint8_t vf_one_hand_axe{ };
    uint8_t vf_double_axe{ };
    uint8_t vf_one_hand_mace{ };
    uint8_t vf_two_hand_mace{ };
    uint8_t vf_lightbow{ };
    uint8_t vf_heavybow{ };
    uint8_t vf_crossbow{ };
    uint8_t vf_one_hand_staff{ };
    uint8_t vf_two_hand_staff{ };
    uint8_t vf_shield_only{ };
    uint8_t vf_is_not_need_weapon{ };
    float   delay_cast{ };
    float   delay_cast_per_skl{ };
    float   delay_cast_mode_per{ };
    float   delay_common{ };
    float   delay_cooltime{ };
    float   delay_cooltime_mode{ };
    int     cool_time_group_id{ };
    uint8_t uf_self{ };
    uint8_t uf_party{ };
    uint8_t uf_guild{ };
    uint8_t uf_neutral{ };
    uint8_t uf_purple{ };
    uint8_t uf_enemy{ };
    uint8_t tf_avatar{ };
    uint8_t tf_summon{ };
    uint8_t tf_monster{ };
    short   target{ };
    short   effect_type{ };
    int     state_id{ };
    int     state_level_base{ };
    float   state_level_per_skl{ };
    float   state_level_per_enhance{ };
    float   state_second{ };
    float   state_second_per_level{ };
    float   state_second_per_enhance{ };
    uint8_t state_type{ };
    int     probability_on_hit{ };
    int     probability_inc_by_slv{ };
    short   hit_bonus{ };
    short   hit_bonus_per_enhance{ };
    short   percentage{ };
    float   hate_mod{ };
    short   hate_basic{ };
    float   hate_per_skl{ };
    float   hate_per_enhance{ };
    int     critical_bonus{ };
    int     critical_bonus_per_skl{ };
    float   var[20]{ };
    short   is_projectile{ };
    float   projectile_speed{ };
    float   projectile_acceleration{ };
};


#endif // PROJECT_SKILLBASE_H
