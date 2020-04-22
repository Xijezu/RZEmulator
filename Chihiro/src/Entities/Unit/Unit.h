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
#include "CreatureAttribute.h"
#include "DamageTemplate.h"
#include "Object.h"
#include "State.h"
#include "Util.h"

class State;
class XPacket;
class Item;
class World;
class Skill;

enum CREATURE_STATUS : uint32_t
{
    STATUS_LOGIN_COMPLETE = 0x01,
    STATUS_FIRST_ENTER = 0x02,
    STATUS_ATTACK_STARTED = 0x04,
    STATUS_FIRST_ATTACK = 0x08,
    STATUS_MOVE_PENDED = 0x10,
    STATUS_NEED_TO_UPDATE_STATE = 0x20,
    STATUS_MOVING_BY_FEAR = 0x40,
    STATUS_NEED_TO_CALCULATE_STAT = 0x80,
    STATUS_PROCESSING_REFELCT = 0x100,
    STATUS_INVISIBLE = 0x200,
    STATUS_INVINCIBLE = 0x400,
    STATUS_HIDING = 0x800,
    STATUS_MOVABLE = 0x2000,
    STATUS_ATTACKABLE = 0x4000,
    STATUS_SKILL_CASTABLE = 0x8000,
    STATUS_MAGIC_CASTABLE = 0x10000,
    STATUS_ITEM_USABLE = 0x20000,
    STATUS_MORTAL = 0x40000,
    STATUS_HAVOC_BURST = 0x80000,
    STATUS_FEARED = 0x100000,
    STATUS_FORM_CHANGED = 0x200000,
    STATUS_MOVE_SPEED_FIXED = 0x400000,
    STATUS_HP_REGEN_STOPPED = 0x800000,
    STATUS_MP_REGEN_STOPPED = 0x1000000,
    STATUS_USING_DOUBLE_WEAPON = 0x2000000,
};

enum DamageType : int
{
    DT_NORMAL_PHYSICAL_DAMAGE = 0,
    DT_NORMAL_MAGICAL_DAMAGE = 1,
    DT_NORMAL_PHYSICAL_SKILL_DAMAGE = 2,
    DT_ADDITIONAL_DAMAGE = 3,
    DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE = 4,
    DT_ADDITIONAL_LEFT_HAND_DAMAGE = 5,
    DT_ADDITIONAL_MAGICAL_DAMAGE = 6,
    DT_STATE_MAGICAL_DAMAGE = 7,
    DT_STATE_PHYSICAL_DAMAGE = 8,
};

enum class NEXT_ATTACK_MODE : int32_t
{
    AM_ATTACK = 0,
    AM_AIMING = 1,
};

enum _CHARACTER_RESURRECTION_TYPE
{
    CRT_NORMAL = 0,
    CRT_BATTLE = 1,
    CRT_COMPETE = 2,
    CRT_SKILL = 3,
    CRT_SKILL_WITH_RECOVER = 4,
    CRT_ITEM = 5,
    CRT_STATE = 6,
    CRT_FAIRY_POTION = 7,
    CRT_HUNTAHOLIC = 8,
    CRT_BATTLE_ARENA = 9,
};

class Unit : public WorldObject
{
public:
    friend class Messages;
    friend class Skill;
    friend class Player;
    friend class XLua;
    friend class WorldSession;

    explicit Unit(bool isWorldObject);
    // Deleting the copy & assignment operators
    // Better safe than sorry
    Unit(const Unit &) = delete;
    Unit &operator=(const Unit &) = delete;
    virtual ~Unit();

    static void EnterPacket(XPacket &, Unit *, Player *);
    void AddToWorld() override;
    void RemoveFromWorld() override;
    void CleanupBeforeRemoveFromMap(bool finalCleanup);
    void CleanupsBeforeDelete(bool finalCleanup = true);
    void Update(uint32_t time) override;
    virtual void OnUpdate();

    bool AddToEnemyList(uint32_t handle);
    bool RemoveFromEnemyList(uint32_t handle);
    uint32_t GetEnemyCount() { return static_cast<uint32_t>(m_vEnemyList.size()); }

    /// SKILLS
    int32_t GetCurrentSkillLevel(int32_t skill_id) const;
    int32_t GetBaseSkillLevel(int32_t skill_id) const;
    Skill *SetSkill(int32_t skill_uid, int32_t skill_id, int32_t skill_level, int32_t remain_cool_time);
    Skill *GetSkill(int32_t skill_id) const;
    Skill *GetSkillByEffectType(SKILL_EFFECT_TYPE nEffectTypeID) const;
    void RegisterSkill(int32_t skill_id, int32_t skill_level, uint32_t remain_cool_time, int32_t nJobID);
    void EnumPassiveSkill(struct SkillFunctor &fn);
    /// END SKILLS

    virtual bool StartAttack(uint32_t target, bool bNeedFastReaction);
    bool IsUsingDoubleWeapon() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON);
    }

    int32_t GetPrevJobId(int32_t nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetInt32Value(UNIT_FIELD_PREV_JOB + nDepth);
    }

    int32_t GetPrevJobLv(int32_t nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetInt32Value(UNIT_FIELD_PREV_JLV + nDepth);
    }

    void regenHPMP(uint32_t t);

    uint32_t HasUnitTypeMask(uint32_t mask) const
    {
        return mask & m_unitTypeMask;
    }

    /// BATTLE START
    void Attack(Unit *pTarget, uint32_t t, uint32_t attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack);
    void EndAttack();

    uint32_t GetTargetHandle() const
    {
        return GetUInt32Value(BATTLE_FIELD_TARGET_HANDLE);
    }

    virtual int32_t GetMoveSpeed();
    inline int32_t GetRealMoveSpeed()
    {
        return GetMoveSpeed() / 7;
    }
    uint8_t GetRealRidingSpeed();

    inline int32_t GetStrength() const
    {
        return m_cStat.strength;
    }
    inline int32_t GetVital() const
    {
        return m_cStat.vital;
    }
    inline int32_t GetDexterity() const
    {
        return m_cStat.dexterity;
    }
    inline int32_t GetAgility() const
    {
        return m_cStat.agility;
    }
    inline int32_t GetIntelligence() const
    {
        return m_cStat.intelligence;
    }
    inline int32_t GetMentality() const
    {
        return m_cStat.mentality;
    }
    inline int32_t GetLuck() const
    {
        return m_cStat.luck;
    }

    virtual float GetFCM() const
    {
        return 1.0f;
    }
    inline int32_t GetCritical() const
    {
        return m_Attribute.nCritical;
    }
    inline int32_t GetCriticalPower() const
    {
        return m_Attribute.nCriticalPower;
    }
    inline int32_t GetAttackPointRight() const
    {
        return m_Attribute.nAttackPointRight;
    }
    inline int32_t GetAttackPointLeft() const
    {
        return m_Attribute.nAttackPointLeft;
    }
    inline int32_t GetDefence() const
    {
        return m_Attribute.nDefence;
    }
    inline int32_t GetMagicPoint() const
    {
        return m_Attribute.nMagicPoint;
    }
    inline int32_t GetMagicDefence() const
    {
        return m_Attribute.nMagicDefence;
    }
    inline int32_t GetAccuracyRight() const
    {
        return m_Attribute.nAccuracyRight;
    }
    inline int32_t GetAccuracyLeft() const
    {
        return m_Attribute.nAccuracyLeft;
    }
    inline int32_t GetMagicAccuracy() const
    {
        return m_Attribute.nMagicAccuracy;
    }
    inline int32_t GetAvoid() const
    {
        return m_Attribute.nAvoid;
    }
    inline int32_t GetMagicAvoid() const
    {
        return m_Attribute.nMagicAvoid;
    }
    inline int32_t GetBlockChance() const
    {
        return m_Attribute.nBlockChance;
    }
    inline int32_t GetBlockDefence() const
    {
        return m_Attribute.nBlockDefence;
    }
    inline int32_t GetAttackSpeed() const
    {
        return m_Attribute.nAttackSpeed;
    }
    inline int32_t GetMaxWeight() const
    {
        return m_Attribute.nMaxWeight;
    }
    inline int32_t GetCastingSpeed() const
    {
        return m_Attribute.nCastingSpeed;
    }

    inline CreatureAtributeServer &GetAttribute()
    {
        return m_Attribute;
    }
    inline CreatureAttributeAmplifier &GetAttributeAmplifier()
    {
        return m_AttributeAmplifier;
    }
    inline CreatureStat &GetCreatureStat()
    {
        return m_cStat;
    }
    inline CreatureStatAmplifier &GetCreatureStatAmplifier()
    {
        return m_StatAmplifier;
    }

    uint32_t GetNextAttackableTime() const
    {
        return GetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME);
    }

    uint32_t GetTrapHandle() const
    {
        return GetUInt32Value(UNIT_FIELD_TRAP_HANDLE);
    }

    void SetTrapHandle(uint32_t nHandle)
    {
        SetUInt32Value(UNIT_FIELD_TRAP_HANDLE, nHandle);
    }

    float GetRealAttackRange() const
    {
        return (12 * m_Attribute.nAttackRange) / 100.0f;
    }

    uint32_t GetAttackInterval() const
    {
        return (uint32_t)(100.0f / m_Attribute.nAttackSpeed * 115.0f);
    };
    int32_t GetElementalResist(ElementalType elemental_type) const
    {
        return m_Resist.nResist[elemental_type];
    }

    void AddEnergy();
    void RemoveEnergy(int32_t nEnergy);

    State *GetState(StateCode code);
    Damage CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int32_t accuracy_bonus, float critical_amp, int32_t critical_bonus, int32_t nFlag);
    DamageInfo DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0);
    DamageInfo DealPhysicalNormalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0);
    Damage DealDamage(Unit *pFrom, float nDamage, ElementalType type, DamageType damageType, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealPhysicalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalMagicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealPhysicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealMagicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int32_t accuracy_bonus = 0, int32_t critical_bonus = 0, int32_t nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);

    DamageInfo DealMagicalSkillDamage(Unit *pFrom, int32_t nDamage, ElementalType elemental_type, int32_t accuracy_bonus, int32_t critical_bonus, int32_t nFlag);
    DamageInfo DealPhysicalSkillDamage(Unit *pFrom, int32_t nDamage, ElementalType elemental_type, int32_t accuracy_bonus, int32_t critical_bonus, int32_t nFlag);
    void ProcessAdditionalDamage(DamageInfo &damage_info, DamageType additionalDamage, std::vector<AdditionalDamageInfo> &vAdditionalDamage, Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE);
    int32_t damage(Unit *pFrom, int32_t nDamage, bool decreaseEXPOnDead = true);

    bool IsPhysicalImmune()
    {
        return (GetState(SC_SEAL) != nullptr);
    }
    bool IsMagicalImmune()
    {
        return (GetState(SC_SEAL) != nullptr || GetState(SC_SHINE_WALL) != nullptr);
    }

    float GetManaCostRatio(ElementalType type, bool bPhysical, bool bBad);
    bool ResurrectByState();

    int32_t Heal(int32_t hp);
    int32_t MPHeal(int32_t mp);
    int32_t HealByItem(int32_t hp);
    int32_t MPHealByItem(int32_t mp);

    bool Resurrect(_CHARACTER_RESURRECTION_TYPE eResurrectType, int32_t nIncHP, int32_t nIncMP, int64_t nRecoveryEXP, bool bIsRestoreRemovedStateByDead);

    /// BATTLE END
    void RemoveState(StateCode code, int32_t state_level);
    void RemoveState(int32_t uid);
    void RemoveGoodState(int32_t state_level);

    // Setters
    void SetLevel(uint8_t lvl)
    {
        SetInt32Value(UNIT_FIELD_LEVEL, static_cast<int32_t>(lvl));
    }

    void SetCurrentJob(uint32_t job)
    {
        SetInt32Value(UNIT_FIELD_JOB, job);
    }

    void SetJP(int32_t jp);
    void SetCurrentJLv(int32_t jlv);
    void SetHealth(int32_t);
    void SetMana(int32_t);

    void BindSkillCard(Item *pItem);
    void UnBindSkillCard(Item *pItem);
    virtual bool IsEnemy(const Unit *pTarget, bool bIncludeHiding = false);
    virtual bool IsAlly(const Unit *pTarget);
    bool IsVisible(const Unit *pTarget);

    bool IsDead() const
    {
        return GetHealth() == 0;
    }
    bool IsAlive() const
    {
        return !IsDead();
    }

    bool IsAttacking() const
    {
        return GetEnemyHandle() != 0;
    }
    uint32_t GetEnemyHandle() const
    {
        return GetUInt32Value(BATTLE_FIELD_TARGET_HANDLE);
    }

    bool IsFeared() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_FEARED);
    }
    bool IsHavocBurst() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_HAVOC_BURST);
    }
    bool IsProcessingReflectDamage() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_PROCESSING_REFELCT);
    }
    bool IsFormChanged() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_FORM_CHANGED);
    }
    bool IsHiding() const
    {
        return HasFlag(UNIT_FIELD_STATUS, STATUS_HIDING);
    }
    bool IsUsingSkill() const
    {
        return m_castingSkill != nullptr;
    }

    virtual bool IsActable();
    virtual bool IsAttackable();
    virtual bool IsMovable();
    virtual bool IsSkillCastable();
    virtual bool IsMagicCastable();
    virtual bool IsSitDown() const
    {
        return false;
    }

    void SetMaxHealth(uint32_t val)
    {
        SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val);
    };

    void SetMaxMana(uint32_t val)
    {
        SetUInt32Value(UNIT_FIELD_MAX_MANA, val);
    };

    void SetFullHealth()
    {
        SetHealth(GetMaxHealth());
    }

    void SetEXP(int64_t exp);

    int32_t GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad);

    float GetCoolTimeSpeed() const
    {
        return m_Attribute.nCoolTimeSpeed / 100.0f;
    }

    float GetCoolTimeMod(ElementalType type, bool bPhysical, bool bBad) const;

    uint32_t GetRemainCoolTime(int32_t skill_id) const;
    uint32_t GetTotalCoolTime(int32_t skill_id) const;

    uint32_t GetBowAttackInterval()
    {
        return (uint32_t)((float)GetAttackInterval() * (1.0f - m_fBowInterval));
    }

    uint32_t GetBowInterval()
    {
        return (uint32_t)((float)GetAttackInterval() * m_fBowInterval);
    }

    // Getters
    int32_t GetLevel() const
    {
        return GetInt32Value(UNIT_FIELD_LEVEL);
    }

    virtual int32_t GetRace() const
    {
        return GetInt32Value(UNIT_FIELD_RACE);
    }

    int32_t GetMagicPoint(ElementalType type, bool bPhysical, bool bBad);

    /// @Todo: Implement correctly
    bool IsKnockbackable() const
    {
        return true;
    }

    uint32_t GetNextMovableTime() const
    {
        return m_nMovableTime;
    }

    void SetNextMovableTime(uint32_t t)
    {
        m_nMovableTime = t;
    }

    float GetMagicalHateMod(ElementalType type, bool bPhysical, bool bBad);

    int32_t GetHealth() const
    {
        return GetInt32Value(UNIT_FIELD_HEALTH);
    }

    int32_t GetMaxHealth() const
    {
        return GetInt32Value(UNIT_FIELD_MAX_HEALTH);
    }

    int32_t GetMana() const
    {
        return GetInt32Value(UNIT_FIELD_MANA);
    }

    int32_t GetMaxMana() const
    {
        return GetInt32Value(UNIT_FIELD_MAX_MANA);
    }

    int32_t GetCurrentJob() const
    {
        return GetInt32Value(UNIT_FIELD_JOB);
    };

    int32_t GetCurrentJLv() const
    {
        return GetInt32Value(UNIT_FIELD_JLV);
    }

    int32_t GetStamina() const
    {
        return GetInt32Value(UNIT_FIELD_STAMINA);
    }

    int32_t GetJP() const
    {
        return GetInt32Value(UNIT_FIELD_JOBPOINT);
    }

    uint32_t GetTotalJP() const
    {
        return GetUInt32Value(UNIT_FIELD_JOBPOINT);
    }

    float GetCastingMod(ElementalType type, bool bPhysical, bool bBad, uint32_t nOriginalCoolTime) const;
    float GetItemChance() const;

    int64_t GetEXP() const
    {
        return static_cast<int64_t>(GetUInt64Value(UNIT_FIELD_EXP));
    }

    virtual uint32_t GetCreatureGroup() const;

    void AddHealth(int32_t hp)
    {
        SetHealth(GetHealth() + hp);
    }

    void AddMana(int32_t mp)
    {
        SetMana(GetMana() + mp);
    }

    void RestoreRemovedStateByDeath();
    void ClearRemovedStateByDeath();

    void RemoveAllAura();
    void RemoveAllHate();

    virtual void AddEXP(int64_t exp, uint32_t jp, bool bApplyStamina);
    void CancelSkill();
    void CancelAttack();
    int32_t CastSkill(int32_t nSkillID, int32_t nSkillLevel, uint32_t target_handle, Position pos, uint8_t layer, bool bIsCastedByItem);
    bool OnCompleteSkill();
    void SetMultipleMove(std::vector<Position> &_to, uint8_t _speed, uint32_t _start_time);
    void SetMove(Position _to, uint8_t _speed, uint32_t _start_time);
    int32_t GetArmorClass() const;
    // Event handler
    bool IsWornByCode(int32_t code) const;
    virtual bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int32_t> *ItemList);
    Item *GetWornItem(ItemWearType);
    uint16_t Puton(ItemWearType pos, Item *item, bool bIsTranslated = false);
    uint16_t Putoff(ItemWearType pos);
    ItemWearType GetAbsoluteWearPos(ItemWearType pos);
    ItemClass GetWeaponClass();
    bool IsWearShield();
    std::pair<float, int32_t> GetHateMod(int32_t nHateModType, bool bIsHarmful);
    void PrepareRemoveExhaustiveSkillStateMod(bool bPhysical, bool bHarmful, int32_t nElementalType, uint32_t nOriginalCastingDelay);
    void RemoveExhaustiveSkillStateMod(bool bPhysical, bool bHarmful, int32_t nElementalType, uint32_t nOriginalCastingDelay);

    Skill *GetCastSkill() const
    {
        return m_castingSkill;
    }

    virtual CreatureStat *GetBaseStat() const
    {
        return nullptr;
    }

    virtual bool IsUsingBow() const
    {
        return false;
    }

    virtual bool IsUsingCrossBow() const
    {
        return false;
    }

    bool IsUnit() const override
    {
        return true;
    }

    bool TurnOnAura(Skill *pSkill);
    bool TurnOffAura(Skill *pSkill);
    void ToggleAura(Skill *pSkill);
    bool IsActiveAura(Skill *pSkill) const;

    void CalculateStat();

    uint16_t AddState(StateType type, StateCode code, uint32_t caster, int32_t level, uint32_t start_time, uint32_t end_time, bool bIsAura = false, int32_t nStateValue = 0, std::string szStateValue = "");

protected:
    uint16_t onItemUseEffect(Unit *pCaster, Item *pItem, int32_t type, float var1, float var2, const std::string &szParameter);
    void removeExhaustiveSkillStateMod(ElementalSkillStateMod &skillStateMod, uint32_t nOriginalCastingTime);

    virtual bool onProcAura(Skill *pSkill, int32_t nRequestedLevel);
    virtual void procStateDamage(uint32_t t);
    virtual void procState(uint32_t t);

    int64_t GetBulletCount() const;
    State *GetStateByEffectType(int32_t effectType) const;

    virtual void onRegisterSkill(int64_t, int, int, int32_t){};

    virtual void onExpChange(){};

    virtual void onEnergyChange(){};

    virtual void onJobLevelUp(){};
    virtual void onAttackAndSkillProcess();

    virtual void onCantAttack(uint32_t target, uint32_t t){};
    virtual uint16_t putonItem(ItemWearType pos, Item *item);
    virtual uint16_t putoffItem(ItemWearType);

    // Overwritten in Monster
    virtual int32_t onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int32_t nDamage, bool bCritical)
    {
        return nDamage;
    }

    virtual void onDead(Unit *pFrom, bool decreaseEXPOnDead);
    void removeStateByDead();
    void processAttack();
    void broadcastAttackMessage(Unit *pTarget, AttackInfo arDamage[], int32_t tm, int32_t delay, bool bIsDoubleAttack, bool bIsAiming = false, bool bEndAttack = false, bool bCancelAttack = false);
    void onAfterAddState(State *);
    virtual void onAfterRemoveState(State *state);
    virtual void onUpdateState(State *state, bool bIsExpire);
    void procMoveSpeedChange();
    void processPendingMove();

    ///-CalculateStat functions
    virtual void incParameter(uint32_t nBitset, float nValue, bool bStat);
    virtual void incParameter2(uint32_t nBitset, float fValue);
    virtual void ampParameter2(uint32_t nBitset, float fValue);
    virtual void ampParameter(uint32_t nBitset, float fValue, bool bStat);
    virtual void applyPassiveSkillAmplifyEffect();
    virtual void applyPassiveSkillAmplifyEffect(Skill *);
    virtual void onApplyAttributeAdjustment(){};
    virtual void applyPassiveSkillEffect(Skill *skill);
    virtual void onApplyStat(){};
    virtual void applyState(State &state);
    virtual void applyStatByState();
    virtual void onItemWearEffect(Item *pItem, bool bIsBaseVar, int32_t type, float var1, float var2, float fRatio);
    virtual void applyJobLevelBonus(){};
    virtual void onModifyStatAndAttribute(){};
    virtual void onBeforeCalculateStat(){};
    virtual void applyPassiveSkillEffect();
    virtual void onCompleteCalculateStat(){}; /* overwritten in player class*/

    void getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute);
    void applyStateAmplify(State *state);
    void applyDoubeWeaponEffect();
    void applyStatByItem();
    void getAmplifiedStatByAmplifier(CreatureStat &);
    void finalizeStat();
    void calcAttribute(CreatureAtributeServer &attribute);
    void applyItemEffect();
    void applyStateEffect();
    void applyStateAmplifyEffect();
    void amplifyStatByState();
    ///-END CalculateStat functions

    void _InitTimerFieldsAndStatus();
    std::vector<State *> m_vStateList{};
    std::vector<State *> m_vStateListRemovedByDeath{};
    uint32_t m_unitTypeMask;
    //	typedef std::list<GameObject*> GameObjectList;
    //	GameObjectList m_gameObj;

    ExpertMod m_Expert[10]{};
    CreatureStat m_cStat{};
    CreatureStat m_cStatByState{};
    CreatureAtributeServer m_Attribute{};
    CreatureAtributeServer m_AttributeByState{};
    CreatureStatAmplifier m_StatAmplifier{};
    CreatureAttributeAmplifier m_AttributeAmplifier{};
    CreatureElementalResist m_Resist{};
    CreatureElementalResistAmplifier m_ResistAmplifier{};
    std::vector<HateModifier> m_vHateMod{};

    ///- Statemods
    StateMod m_NormalStateAdvantage{};
    StateMod m_RangeStateAdvantage{};
    StateMod m_NormalStatePenalty{};
    StateMod m_RangeStatePenalty{};
    StateMod m_PhysicalSkillStatePenalty{};
    StateMod m_MagicalSkillStatePenalty{};
    StateMod m_StateStatePenalty{};

    ///- ElementalSkillStateMods
    ElementalSkillStateMod m_GoodPhysicalElementalSkillStateMod[ElementalType::TYPE_COUNT];
    ElementalSkillStateMod m_BadPhysicalElementalSkillStateMod[ElementalType::TYPE_COUNT];
    ElementalSkillStateMod m_GoodMagicalElementalSkillStateMod[ElementalType::TYPE_COUNT];
    ElementalSkillStateMod m_BadMagicalElementalSkillStateMod[ElementalType::TYPE_COUNT];

    ///- Additional Damage
    std::vector<AdditionalDamageInfo> m_vNormalAdditionalDamage{};
    std::vector<AdditionalDamageInfo> m_vRangeAdditionalDamage{};
    std::vector<AdditionalDamageInfo> m_vPhysicalSkillAdditionalDamage{};
    std::vector<AdditionalDamageInfo> m_vMagicalSkillAdditionalDamage{};

    std::vector<AddHPMPOnCriticalInfo> m_vAddHPMPOnCritical{};

    ///- Reflect Info
    std::vector<DamageReflectInfo> m_vDamageReflectInfo{};
    std::vector<StateReflectInfo> m_vStateReflectInfo{};

    ///- DamageReduceInfo
    std::vector<DamageReduceInfo> m_vDamageReducePercentInfo;
    std::vector<DamageReduceInfo> m_vDamageReduceValueInfo;

    ///- State Tags
    std::vector<_ADD_STATE_TAG> m_vStateByNormalAttack{};
    std::vector<_ADD_STATE_TAG> m_vStateByHelpfulPhysicalSkill{};
    std::vector<_ADD_STATE_TAG> m_vStateByHelpfulMagicalSkill{};
    std::vector<_ADD_STATE_TAG> m_vStateByHarmfulPhysicalSkill{};
    std::vector<_ADD_STATE_TAG> m_vStateByHarmfulMagicalSkill{};
    std::vector<_ADD_STATE_TAG> m_vStateByBeingNormalAttacked{};
    std::vector<_ADD_STATE_TAG> m_vStateByBeingHelpfulPhysicalSkilled{};
    std::vector<_ADD_STATE_TAG> m_vStateByBeingHelpfulMagicalSkilled{};
    std::vector<_ADD_STATE_TAG> m_vStateByBeingHarmfulPhysicalSkilled{};
    std::vector<_ADD_STATE_TAG> m_vStateByBeingHarmfulMagicalSkilled{};
    std::vector<_DAMAGE_ABSORB_TAG> m_vAbsorbByNormalAttack{};
    std::vector<_HEAL_ON_ATTACK_TAG> m_vHealOnAttack{};
    std::vector<_STEAL_ON_ATTACK_TAG> m_vStealOnAttack{};

    Item *m_anWear[MAX_ITEM_WEAR]{nullptr};
    uint32_t m_nMovableTime{0};
    int32_t m_nUnitExpertLevel{0};
    int32_t m_nNextAttackMode{0};
    Skill *m_castingSkill{nullptr};
    float m_nRegenHP{}, m_fRegenMP{};

private:
    std::vector<std::pair<Skill *, int32_t>> m_vAura{};
    std::vector<Skill *> m_vSkillList{};
    std::vector<Skill *> m_vActiveSkillList{};
    std::vector<Skill *> m_vPassiveSkillList{};
    std::vector<int32_t> m_vInterruptedSkill{};
    std::vector<std::set<int32_t>> m_vAllowedSkill{};
    std::vector<uint32_t> m_vEnemyList{};

    std::vector<Skill *> m_vAmplifyPassiveSkillList{};

    float m_fBowInterval{0};
    bool ClearExpiredState(uint32_t t);
    uint32_t m_nCurrentStateUID{0};
    int32_t m_nDoubleWeaponMasteryLevel{0};

    void ProcessAddHPMPOnCritical();
    void AddStateByAttack(Unit *pTarget, bool bIsAttacking, bool bIsSkill, bool bIsPhysicalSkill, bool bIsHarmful);
    void RemoveStatesOnDamage();
    int32_t GetCriticalDamage(int32_t damage, float critical_amp, int32_t critical_bonus);
    template <typename COMPARER>
    void RemoveStateIf(COMPARER comparer, std::vector<State *> *result = nullptr, bool bByDead = false);
};