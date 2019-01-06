#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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

enum CREATURE_STATUS : uint
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
    void Update(uint32 time) override;
    virtual void OnUpdate();
    /// SKILLS
    int GetCurrentSkillLevel(int skill_id) const;
    int GetBaseSkillLevel(int skill_id) const;
    Skill *GetSkill(int skill_id) const;
    Skill *RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID);
    /// END SKILLS

    virtual bool StartAttack(uint target, bool bNeedFastReaction);
    bool IsUsingDoubleWeapon() const { return HasFlag(UNIT_FIELD_STATUS, STATUS_USING_DOUBLE_WEAPON); }

    int32 GetPrevJobId(int nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetInt32Value(UNIT_FIELD_PREV_JOB + nDepth);
    }

    int32 GetPrevJobLv(int nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetInt32Value(UNIT_FIELD_PREV_JLV + nDepth);
    }

    void regenHPMP(uint t);

    uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }

    /// BATTLE START
    void Attack(Unit *pTarget, uint t, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack);
    void EndAttack();

    uint GetTargetHandle() const { return GetUInt32Value(BATTLE_FIELD_TARGET_HANDLE); }

    virtual int GetMoveSpeed();

    inline int GetStrength() const { return m_cStat.strength; }
    inline int GetVital() const { return m_cStat.vital; }
    inline int GetDexterity() const { return m_cStat.dexterity; }
    inline int GetAgility() const { return m_cStat.agility; }
    inline int GetIntelligence() const { return m_cStat.intelligence; }
    inline int GetMentality() const { return m_cStat.mentality; }
    inline int GetLuck() const { return m_cStat.luck; }

    virtual float GetFCM() const { return 1.0f; }
    inline int GetCritical() const { return m_Attribute.nCritical; }
    inline int GetCriticalPower() const { return m_Attribute.nCriticalPower; }
    inline int GetAttackPointRight() const { return m_Attribute.nAttackPointRight; }
    inline int GetAttackPointLeft() const { return m_Attribute.nAttackPointLeft; }
    inline int GetDefence() const { return m_Attribute.nDefence; }
    inline int GetMagicPoint() const { return m_Attribute.nMagicPoint; }
    inline int GetMagicDefence() const { return m_Attribute.nMagicDefence; }
    inline int GetAccuracyRight() const { return m_Attribute.nAccuracyRight; }
    inline int GetAccuracyLeft() const { return m_Attribute.nAccuracyLeft; }
    inline int GetMagicAccuracy() const { return m_Attribute.nMagicAccuracy; }
    inline int GetAvoid() const { return m_Attribute.nAvoid; }
    inline int GetMagicAvoid() const { return m_Attribute.nMagicAvoid; }
    inline int GetBlockChance() const { return m_Attribute.nBlockChance; }
    inline int GetBlockDefence() const { return m_Attribute.nBlockDefence; }
    inline int GetAttackSpeed() const { return m_Attribute.nAttackSpeed; }
    inline int GetMaxWeight() const { return m_Attribute.nMaxWeight; }
    inline int GetCastingSpeed() const { return m_Attribute.nCastingSpeed; }

    uint GetNextAttackableTime() const { return GetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME); }

    uint32_t GetTrapHandle() const { return GetUInt32Value(UNIT_FIELD_TRAP_HANDLE); }

    void SetTrapHandle(uint32_t nHandle) { SetUInt32Value(UNIT_FIELD_TRAP_HANDLE, nHandle); }

    float GetRealAttackRange() const { return (12 * m_Attribute.nAttackRange) / 100.0f; }

    uint GetAttackInterval() const { return (uint)(100.0f / m_Attribute.nAttackSpeed * 115.0f); };
    int GetElementalResist(ElementalType elemental_type) const { return m_Resist.nResist[elemental_type]; }

    void AddEnergy();
    void RemoveEnergy(int nEnergy);

    State *GetState(StateCode code);
    Damage CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag);
    DamageInfo DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0);
    DamageInfo DealPhysicalNormalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0);
    Damage DealDamage(Unit *pFrom, float nDamage, ElementalType type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
    Damage DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealPhysicalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealAdditionalMagicalDamage(Unit *pFrom, float nDamage, ElementalType type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealPhysicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);
    Damage DealMagicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type = ElementalType::TYPE_NONE, int accuracy_bonus = 0, int critical_bonus = 0, int nFlag = 0, StateMod *damage_penalty = nullptr, StateMod *damage_advantage = nullptr);

    DamageInfo DealMagicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    DamageInfo DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    int damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead = true);

    bool IsPhysicalImmune() { return (GetState(SC_SEAL) != nullptr); }
    bool IsMagicalImmune() { return (GetState(SC_SEAL) != nullptr || GetState(SC_SHINE_WALL) != nullptr); }

    float GetManaCostRatio(ElementalType type, bool bPhysical, bool bBad);

    int Heal(int hp);
    int MPHeal(int mp);
    int HealByItem(int hp);
    int MPHealByItem(int mp);

    /// BATTLE END
    void RemoveState(StateCode code, int state_level);
    void RemoveState(int uid);
    void RemoveGoodState(int state_level);

    // Setters
    void SetLevel(uint8 lvl)
    {
        SetInt32Value(UNIT_FIELD_LEVEL, static_cast<int>(lvl));
    }

    void SetCurrentJob(uint job)
    {
        SetInt32Value(UNIT_FIELD_JOB, job);
    }

    void SetJP(int jp);
    void SetCurrentJLv(int jlv);
    void SetSkill(int, int, int, int);
    void SetHealth(int);
    void SetMana(int);

    void BindSkillCard(Item *pItem);
    void UnBindSkillCard(Item *pItem);
    virtual bool IsEnemy(const Unit *pTarget, bool bIncludeHiding);
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

    void SetMaxHealth(uint32 val)
    {
        SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val);
    };

    void SetMaxMana(uint32 val)
    {
        SetUInt32Value(UNIT_FIELD_MAX_MANA, val);
    };

    void SetFullHealth()
    {
        SetHealth(GetMaxHealth());
    }

    void SetEXP(int64 exp);

    int GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad);

    float GetCoolTimeSpeed() const
    {
        return m_Attribute.nCoolTimeSpeed / 100.0f;
    }

    float GetCoolTimeMod(ElementalType type, bool bPhysical, bool bBad) const
    {
        return 1.0f;
    }

    uint GetRemainCoolTime(int skill_id) const;
    uint GetTotalCoolTime(int skill_id) const;

    uint GetBowAttackInterval()
    {
        return (uint)((float)GetAttackInterval() * (1.0f - m_fBowInterval));
    }

    uint GetBowInterval()
    {
        return (uint)((float)GetAttackInterval() * m_fBowInterval);
    }

    // Getters
    int GetLevel() const
    {
        return GetInt32Value(UNIT_FIELD_LEVEL);
    }

    virtual int GetRace() const
    {
        return GetInt32Value(UNIT_FIELD_RACE);
    }

    int GetMagicPoint(ElementalType type, bool bPhysical, bool bBad);

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

    int GetHealth() const
    {
        return GetInt32Value(UNIT_FIELD_HEALTH);
    }

    int GetMaxHealth() const
    {
        return GetInt32Value(UNIT_FIELD_MAX_HEALTH);
    }

    int GetMana() const
    {
        return GetInt32Value(UNIT_FIELD_MANA);
    }

    int GetMaxMana() const
    {
        return GetInt32Value(UNIT_FIELD_MAX_MANA);
    }

    int GetCurrentJob() const
    {
        return GetInt32Value(UNIT_FIELD_JOB);
    };

    int GetCurrentJLv() const
    {
        return GetInt32Value(UNIT_FIELD_JLV);
    }

    int GetStamina() const
    {
        return GetInt32Value(UNIT_FIELD_STAMINA);
    }

    int GetJP() const
    {
        return GetInt32Value(UNIT_FIELD_JOBPOINT);
    }

    uint32 GetTotalJP() const
    {
        return GetUInt32Value(UNIT_FIELD_JOBPOINT);
    }

    float GetCastingMod(ElementalType type, bool bPhysical, bool bBad, uint nOriginalCoolTime)
    {
        return 1.0f;
    }

    float GetItemChance() const;

    int64 GetEXP() const
    {
        return static_cast<int64>(GetUInt64Value(UNIT_FIELD_EXP));
    }

    virtual uint GetCreatureGroup() const;

    void AddHealth(int hp)
    {
        SetHealth(GetHealth() + hp);
    }

    void AddMana(int mp)
    {
        SetMana(GetMana() + mp);
    }

    virtual void AddEXP(int64 exp, uint jp, bool bApplyStamina);
    void CancelSkill();
    void CancelAttack();
    int CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem);
    bool OnCompleteSkill();
    void SetMultipleMove(std::vector<Position> &_to, uint8_t _speed, uint _start_time);
    void SetMove(Position _to, uint8 _speed, uint _start_time);
    void CalculateStat();
    int GetArmorClass() const;
    // Event handler
    bool IsWornByCode(int code) const;
    virtual bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList);
    Item *GetWornItem(ItemWearType);
    ushort Puton(ItemWearType pos, Item *item, bool bIsTranslated = false);
    ushort Putoff(ItemWearType pos);
    ItemWearType GetAbsoluteWearPos(ItemWearType pos);
    ItemClass GetWeaponClass();
    bool IsWearShield();
    std::pair<float, int> GetHateMod(int nHateModType, bool bIsHarmful);

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

    uint16 AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura = false, int nStateValue = 0, std::string szStateValue = "");

  protected:
    uint16 onItemUseEffect(Unit *pCaster, Item *pItem, int type, float var1, float var2, const std::string &szParameter);
    void applyStatByItem();

    virtual bool onProcAura(Skill *pSkill, int nRequestedLevel);
    virtual void procStateDamage(uint t);

    virtual void applyJobLevelBonus(){};

    virtual void onModifyStatAndAttribute(){};

    virtual void onBeforeCalculateStat(){};
    virtual void applyPassiveSkillEffect();
    void applyStatByState();
    void getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute);
    void amplifyStatByState();
    virtual void applyState(State &state);
    void applyStateEffect();
    void applyStateAmplifyEffect();
    void applyPassiveSkillAmplifyEffect();
    virtual void applyPassiveSkillAmplifyEffect(Skill *);

    void applyStateAmplify(State *state);
    void applyDoubeWeaponEffect();

    virtual void onApplyAttributeAdjustment(){};
    virtual void applyPassiveSkillEffect(Skill *skill);
    void getAmplifiedStatByAmplifier(CreatureStat &);
    void finalizeStat();
    void calcAttribute(CreatureAtributeServer &attribute);
    void applyItemEffect();

    virtual void onCompleteCalculateStat(){}; /* overwritten in player class*/
    int64 GetBulletCount() const;
    ///- Gets overwritten in player for Beltslots and Max Chaos
    virtual void onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio);
    State *GetStateByEffectType(int effectType) const;

    virtual void onRegisterSkill(int64, int, int, int){};

    virtual void onExpChange(){};

    virtual void onEnergyChange(){};

    virtual void onJobLevelUp(){};
    virtual void onAttackAndSkillProcess();

    virtual void onCantAttack(uint target, uint t){};
    virtual uint16_t putonItem(ItemWearType pos, Item *item);
    virtual uint16_t putoffItem(ItemWearType);

    // Overwritten in Monster
    virtual int onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical)
    {
        return nDamage;
    }

    virtual void onDead(Unit *pFrom, bool decreaseEXPOnDead);
    void processAttack();
    void broadcastAttackMessage(Unit *pTarget, AttackInfo arDamage[], int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack);
    void onAfterAddState(State *);
    virtual void onAfterRemoveState(State *state);
    virtual void onUpdateState(State *state, bool bIsExpire);
    void procMoveSpeedChange();
    void processPendingMove();
    void _InitTimerFieldsAndStatus();
    std::vector<State *> m_vStateList{};
    uint32 m_unitTypeMask;
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
    uint m_nMovableTime{0};
    int m_nUnitExpertLevel{0};
    int m_nNextAttackMode{0};
    Skill *m_castingSkill{nullptr};
    float m_nRegenHP{}, m_fRegenMP{};

  private:
    std::unordered_map<int, int32_t> m_vAura{};
    std::vector<Skill *> m_vSkillList{};
    std::vector<Skill *> m_vActiveSkillList{};
    std::vector<Skill *> m_vPassiveSkillList{};
    std::vector<int> m_vInterruptedSkill{};
    std::vector<std::set<int>> m_vAllowedSkill{};

    std::vector<Skill *> m_vAmplifyPassiveSkillList{};

    float m_fBowInterval{0};
    bool ClearExpiredState(uint t);
    uint m_nCurrentStateUID{0};
    int m_nDoubleWeaponMasteryLevel{0};
    void incParameter(uint nBitset, float nValue, bool bStat);
    void incParameter2(uint nBitset, float fValue);
    void ampParameter2(uint nBitset, float fValue);
    void ampParameter(uint nBitset, float fValue, bool bStat);

    void ProcessAddHPMPOnCritical();
    void AddStateByAttack(Unit *pTarget, bool bIsAttacking, bool bIsSkill, bool bIsPhysicalSkill, bool bIsHarmful);
    void RemoveStatesOnDamage();
    int GetCriticalDamage(int damage, float critical_amp, int critical_bonus);
};