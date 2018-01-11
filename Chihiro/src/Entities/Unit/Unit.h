#ifndef _UNIT_H_
#define _UNIT_H_

#include "Common.h"
#include "CreatureAttribute.h"
#include "DamageTemplate.h"
#include "Entities/Object/Object.h"
#include "Item.h"
#include "State.h"
#include "Utilities/EventProcessor.h"
#include "Utilities/Util.h"

class XPacket;
class Item;
class World;
class Skill;

enum StatusFlags : uint
{
    LoginComplete = 0x01,
    FirstEnter = 0x02,
    AttackStarted = 0x04,
    FirsAttack = 0x08,
    MovePending = 0x10,
    NeedToUpdateState = 0x20,
    MovingByFear = 0x40,
    NeedToCalculateStat = 0x80,
    ProcessingReflect = 0x100,
    Invisible = 0x200,
    Invincible = 0x400,
    Hiding = 0x800,
    Movable = 0x2000,
    Attackable = 0x4000,
    SkillCastable = 0x8000,
    MagicCastable = 0x10000,
    ItemUsable = 0x20000,
    Mortal = 0x40000,
    HavocBurst = 0x80000,
    Feared = 0x100000,
    FormChanged = 0x200000,
    MoveSpeedFixed = 0x400000,
    HPRegenStopped = 0x800000,
    MPRegenStopped = 0x1000000,
    UsingDoubleWeapon = 0x2000000,
    CompeteDead = 0x4000000,
};
enum DamageType : int
{
    NormalPhysical = 0,
    NormalMagical = 1,
    NormalPhysicalSkill = 2,
    Additional = 3,
    NormalPhysicalLeftHand = 4,
    AdditionalLeftHand = 5,
    AdditionalMagical = 6,
    StateMagical = 7,
    StatePhysical = 8,
};
enum DamageFlag : int
{
    IgnoreAvoid = 2,
    IgnoreDefence = 4,
    IgnoreBlock = 8,
    IgnoreCritical = 16,
};
class Unit : public WorldObject
{
    friend class Messages;
    friend class Skill;
    friend class Player;
    friend class XLua;
    friend class WorldSession;
public:
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
    uint32 GetPrevJobId(int nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetUInt32Value(UNIT_FIELD_PREV_JOB + nDepth);
    }
    uint32 GetPrevJobLv(int nDepth) const
    {
        if (nDepth > 3)
            return 0;
        else
            return GetUInt32Value(UNIT_FIELD_PREV_JLV + nDepth);
    }
    void regenHPMP(uint t);
    uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }

    /// BATTLE START
    void Attack(Unit *pTarget, uint t, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack);
    void EndAttack();
    uint GetTargetHandle() const { return GetUInt32Value(BATTLE_FIELD_TARGET_HANDLE); }
    uint GetNextAttackableTime() const { return GetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME); }
    float GetUnitSize() const { return (GetSize() * 12) * GetScale(); }
    float GetRealAttackRange() const { return (12 * m_Attribute.nAttackRange) / 100.0f; }
    uint GetAttackInterval() const { return (uint)(100.0f / m_Attribute.nAttackSpeed * 115.0f); };


    Damage CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag);
    DamageInfo DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    Damage DealDamage(Unit *pFrom, float nDamage, ElementalType type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
    Damage DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
    Damage DealMagicalDamage(Unit* pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);

    DamageInfo DealMagicalSkillDamage(Unit* pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    DamageInfo DealPhysicalSkillDamage(Unit* pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    int damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead);
    /// BATTLE END

    void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
    // Setters
    void SetLevel(uint8 lvl) { SetInt32Value(UNIT_FIELD_LEVEL, lvl); }
    void SetCurrentJob(uint job) { SetUInt32Value(UNIT_FIELD_JOB, job); }
    void SetJP(int jp);
    void SetCurrentJLv(int jlv) { SetInt32Value(UNIT_FIELD_JLV, jlv); }
    void SetSkill(int, int, int, int);
    void SetHealth(uint32);
    void SetMana(uint32);
    void SetMaxHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val); };
    void SetMaxMana(uint32 val) { SetUInt32Value(UNIT_FIELD_MAX_MANA, val); };
    void SetFullHealth() { SetHealth(GetMaxHealth()); }
    void SetEXP(uint exp);

    // eh
    int GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad) const;
    float GetCoolTimeSpeed() const { return m_Attribute.nCoolTimeSpeed / 100.0f; }
    float GetCoolTimeMod(ElementalType type, bool bPhysical, bool bBad) const { return 1.0f; }
    uint GetRemainCoolTime(int skill_id) const;
    uint GetTotalCoolTime(int skill_id) const;
    // Getters
    virtual float GetScale() const { return 1.0f; }
    virtual float GetSize() const { return 1.0f; }
    uint32_t GetLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
    virtual int GetRace() const { return GetInt32Value(UNIT_FIELD_RACE); }
    uint32 GetHealth() const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
    uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAX_HEALTH); }
    uint32 GetMana() const { return GetUInt32Value(UNIT_FIELD_MANA); }
    uint32 GetMaxMana() const { return GetUInt32Value(UNIT_FIELD_MAX_MANA); }
    uint32 GetCurrentJob() const { return GetUInt32Value(UNIT_FIELD_JOB); };
    uint32 GetCurrentJLv() const { return GetUInt32Value(UNIT_FIELD_JLV); }
    uint32 GetStamina() const { return GetUInt32Value(UNIT_FIELD_STAMINA); }
    uint32 GetJP() const { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }
    uint32 GetTotalJP() const { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }
    float GetCastingMod(ElementalType type, bool bPhysical, bool bBad, uint nOriginalCoolTime) { return 1.0f; }
    float GetItemChance() const;
    uint64 GetEXP() const { return GetUInt64Value(UNIT_FIELD_EXP); }
    uint GetCreatureGroup();
    void AddHealth(int hp) { SetHealth(GetHealth() + hp); }
    void AddMana(int mp) { SetMana(GetMana() + mp); }
    virtual void AddEXP(uint64 exp, uint jp, bool bApplyStanima);
    void CancelSkill();
    void CancelAttack();
    int CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem);
    void SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time);
    void SetMove(Position _to, uint8 _speed, uint _start_time);
    void CalculateStat();
    // Event handler
    bool IsWornByCode(int code) const;
    EventProcessor _Events;
    virtual bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList);
    Item *GetWornItem(ItemWearType);
    ushort Puton(ItemWearType pos, Item *item);
    ushort Putoff(ItemWearType pos);
    ItemWearType GetAbsoluteWearPos(ItemWearType pos);
    ItemClass GetWeaponClass();
    bool IsWearShield();
    std::pair<float, int> GetHateMod(int nHateModType, bool bIsHarmful);
    virtual CreatureStat *GetBaseStat() const { return nullptr; }
    uint16 AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura, int nStateValue, std::string szStateValue);
    explicit Unit(bool isWorldObject);
protected:
    uint16 onItemUseEffect(Unit *pCaster, Item *pItem, int type, float var1, float var2, const std::string &szParameter);
    void applyStatByItem();
    virtual void applyJobLevelBonus() {};
    virtual void onModifyStatAndAttribute() {};
    virtual void onBeforeCalculateStat() {};
    void applyPassiveSkillEffect();
    void applyStatByState();
    void getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute);
    void amplifyStatByState();
    void applyState(State &state);
    void applyStateEffect();
    void applyStateAmplifyEffect();
    void applyStateAmplify(State &state);
    void applyDoubeWeaponEffect();
    virtual void onApplyAttributeAdjustment() {};
    virtual void applyPassiveSkillEffect(Skill *skill);
    void getAmplifiedStatByAmplifier(CreatureStat &);
    void finalizeStat();
    void calcAttribute(CreatureAtributeServer &attribute);
    void applyItemEffect();
    ///- Gets overwritten in player for Beltslots and Max Chaos
    virtual void onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio);
    State *GetStateByEffectType(int effectType) const;
    virtual void onRegisterSkill(int64, int, int, int) {};
    virtual void onExpChange() {};
    virtual void onJobLevelUp() { };
    virtual void onAttackAndSkillProcess();
    virtual void onCantAttack(uint target, uint t) {};
    virtual uint16_t putonItem(ItemWearType pos, Item *item);
    virtual uint16_t putoffItem(ItemWearType);
    // Overwritten in Monster
    virtual int onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical) { return nDamage; }
    virtual void onDead(Unit *pFrom, bool decreaseEXPOnDead);
    void processAttack();
    void broadcastAttackMessage(Unit *pTarget, AttackInfo arDamage[], int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack);
    uint m_nLastUpdatedTime{ }, m_nLastStateProcTime{ };
    void onAfterAddState(State);
    void onUpdateState(State state, bool bIsExpire);
    void procMoveSpeedChange();
    void processPendingMove();
    std::vector<State> m_vStateList{ };
    uint32 m_unitTypeMask;
    //	typedef std::list<GameObject*> GameObjectList;
    //	GameObjectList m_gameObj;

    ExpertMod m_Expert[10]{ };
    CreatureStat m_cStat{ };
    CreatureStat m_cStatByState{ };
    CreatureAtributeServer m_Attribute{ };
    CreatureAtributeServer m_AttributeByState{ };
    CreatureStatAmplifier m_StatAmplifier{ };
    CreatureAttributeAmplifier m_AttributeAmplifier{ };
    CreatureElementalResist m_Resist{ };
    CreatureElementalResistAmplifier m_ResistAmplifier{ };
    std::vector<Skill *> m_vSkillList;
    std::vector<HateModifier> m_vHateMod{ };
    Item *m_anWear[Item::MAX_ITEM_WEAR]{nullptr};
    uint m_nMovableTime{0};
    int m_nUnitExpertLevel{0};
    Skill *m_castingSkill{nullptr};
    float m_nRegenHP{ }, m_fRegenMP{ };
private:
    bool ClearExpiredState(uint t);
    uint m_nCurrentStateUID{0};
    void incParameter(uint nBitset, float nValue, bool bStat);
    void incParameter2(uint nBitset, float fValue);
    void ampParameter2(uint nBitset, float fValue);
    void ampParameter(uint nBitset, float fValue, bool bStat);
};
#endif // !_UNIT_H_
