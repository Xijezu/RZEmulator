#ifndef _UNIT_H_
#define _UNIT_H_

#include "Common.h"
#include "Item.h"
#include "Entities/Object/Object.h"
#include "Utilities/Util.h"
#include "Utilities/EventProcessor.h"
#include "CreatureAttribute.h"
#include "DamageTemplate.h"

class XPacket;
class Item;
class World;
class Skill;

enum DeathState {
    ALIVE          = 0,
    JUST_DIED      = 1,
    CORPSE         = 2,
    DEAD           = 3,
    JUST_RESPAWNED = 4,
};

enum StatusFlags : uint {
    LoginComplete       = 0x01,
    FirstEnter          = 0x02,
    AttackStarted       = 0x04,
    FirsAttack          = 0x08,
    MovePending         = 0x10,
    NeedToUpdateState   = 0x20,
    MovingByFear        = 0x40,
    NeedToCalculateStat = 0x80,
    ProcessingReflect   = 0x100,
    Invisible           = 0x200,
    Invincible          = 0x400,
    Hiding              = 0x800,
    Movable             = 0x2000,
    Attackable          = 0x4000,
    SkillCastable       = 0x8000,
    MagicCastable       = 0x10000,
    ItemUsable          = 0x20000,
    Mortal              = 0x40000,
    HavocBurst          = 0x80000,
    Feared              = 0x100000,
    FormChanged         = 0x200000,
    MoveSpeedFixed      = 0x400000,
    HPRegenStopped      = 0x800000,
    MPRegenStopped      = 0x1000000,
    UsingDoubleWeapon   = 0x2000000,
    CompeteDead         = 0x4000000,
};

enum DamageType : int {
    NormalPhysical         = 0,
    NormalMagical          = 1,
    NormalPhysicalSkill    = 2,
    Additional             = 3,
    NormalPhysicalLeftHand = 4,
    AdditionalLeftHand     = 5,
    AdditionalMagical      = 6,
    StateMagical           = 7,
    StatePhysical          = 8,
};

enum DamageFlag : int {
    IgnoreAvoid    = 2,
    IgnoreDefence  = 4,
    IgnoreBlock    = 8,
    IgnoreCritical = 16,
};


class Unit : public WorldObject {
public:
    virtual ~Unit();

    static void EnterPacket(XPacket&, Unit*);

    void AddToWorld() override;
    void RemoveFromWorld() override;
    void CleanupBeforeRemoveFromMap(bool finalCleanup);

    void CleanupsBeforeDelete(bool finalCleanup = true);                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

    void Update(uint32 time) override;
    virtual void OnUpdate();

    /// SKILLS
    int GetCurrentSkillLevel(int skill_id) const;
    int GetBaseSkillLevel(int skill_id) const;
    Skill* GetSkill(int skill_id) const;
    Skill* RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID);
    /// END SKILLS

    bool StartAttack(uint target, bool bNeedFastReaction);

    uint32 GetPrevJobId(int nDepth) const
    { if (nDepth > 3) return 0; else return GetUInt32Value(UNIT_FIELD_PREV_JOB + nDepth); }

    uint32 GetPrevJobLv(int nDepth) const
    { if (nDepth > 3) return 0; else return GetUInt32Value(UNIT_FIELD_PREV_JLV + nDepth); }

    void regenHPMP(uint t);

    uint32 HasUnitTypeMask(uint32 mask) const
    { return mask & m_unitTypeMask; }

    /// BATTLE START
    void Attack(Unit* pTarget, uint t, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack);
    void EndAttack();

    uint GetTargetHandle() const
    { return GetUInt32Value(BATTLE_FIELD_TARGET_HANDLE); }

    uint GetNextAttackableTime() const
    { return GetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME); }

    virtual float GetScale() const
    { return 1.0f; }

    virtual float GetSize() const
    { return 1.0f; }

    float GetUnitSize() const
    { return (GetSize() * 12) * GetScale(); }

    float GetRealAttackRange() const
    { return (12 * m_Attribute.nAttackRange) / 100.0f; }

    uint GetAttackInterval() const
    { return (uint)(100.0f / m_Attribute.nAttackSpeedRight * 115.0f); }

    Damage CalcDamage(Unit* pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag);

    DamageInfo DealPhysicalNormalDamage(Unit* pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
    Damage DealDamage(Unit* pFrom, float nDamage, ElementalType elemental_type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
    Damage DealPhysicalDamage(Unit* pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
    int damage(Unit* pFrom, int nDamage, bool decreaseEXPOnDead);
    /// BATTLE END

    void AddUnitTypeMask(uint32 mask)
    { m_unitTypeMask |= mask; }
// 	bool isSummon() const { return m_unitTypeMask & UNIT_MASK_SUMMON; }
// 	bool isPet() const { return m_unitTypeMask & UNIT_MASK_PET; }

    uint32_t GetLevel() const
    { return GetUInt32Value(UNIT_FIELD_LEVEL); }

    void SetLevel(uint8 lvl)
    { SetInt32Value(UNIT_FIELD_LEVEL, lvl); }
// 	uint8 getRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
// 	uint32 getRaceMask() const { return 1 << (getRace() - 1); }
// 	uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
// 	uint32 getClassMask() const { return 1 << (getClass() - 1); }
// 	uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 2); }

    uint32 GetHealth() const
    { return GetUInt32Value(UNIT_FIELD_HEALTH); }

    uint32 GetMaxHealth() const
    { return GetUInt32Value(UNIT_FIELD_MAX_HEALTH); }

    uint32 GetMana() const
    { return GetUInt32Value(UNIT_FIELD_MANA); }

    uint32 GetMaxMana() const
    { return GetUInt32Value(UNIT_FIELD_MAX_MANA); }

    void SetCurrentJob(uint job)
    { SetUInt32Value(UNIT_FIELD_JOB, job); }

    uint32 GetCurrentJob() const
    { return GetUInt32Value(UNIT_FIELD_JOB); };

    void SetCurrentJLv(int jlv)
    { SetInt32Value(UNIT_FIELD_JLV, jlv); }

    uint32 GetCurrentJLv() const
    { return GetUInt32Value(UNIT_FIELD_JLV); }

    uint32 GetStamina() const
    { return GetUInt32Value(UNIT_FIELD_STAMINA); }

    void SetJP(int jp)
    { SetInt32Value(UNIT_FIELD_JOBPOINT, jp); }

    uint32 GetJP() const
    { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }

    uint32 GetTotalJP() const
    { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }

    void SetEXP(uint exp)
    { SetUInt64Value(UNIT_FIELD_EXP, exp); onExpChange(); }

    uint64 GetEXP() const
    { return GetUInt64Value(UNIT_FIELD_EXP); }

    uint GetCreatureGroup();

    void AddHealth(int hp)
    { SetHealth(GetHealth() + hp); }
    void AddMana(int mp)
    { SetMana(GetMana() + mp); }

    virtual void AddEXP(uint64 exp, uint jp, bool bApplyStanima);

    void SetSkill(int,int,int,int);

    void SetHealth(uint32);
    void SetMana(uint32);

    void SetMaxHealth(uint32 val)
    { SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val); };

    void SetMaxMana(uint32 val)
    { SetUInt32Value(UNIT_FIELD_MAX_MANA, val); };

    void SetFullHealth()
    { SetHealth(GetMaxHealth()); }

    void SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time);
    void SetMove(Position _to, uint8 _speed, uint _start_time);


    int CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem);
    float GetCastingMod(ElementalType type, bool bPhysical, bool bBad, uint nOriginalCoolTime)
    { return 1.0f; }

    void CancelSkill();
    void CancelAttack();

    // Event handler
    EventProcessor _Events;

    void CalculateStat();
    void applyStatByItem();
    virtual void applyJobLevelBonus() = 0;
    void getAmplifiedStatByAmplifier(CreatureStat &);
    void finalizeStat();
    void calcAttribute(CreatureAtributeServer &attribute);
    void applyItemEffect();
    void onItemWearEffect(Item* pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio);
    Item *GetWornItem(ItemWearType);
    virtual uint16_t putonItem(ItemWearType pos, Item *item) = 0;
    virtual uint16_t putoffItem(ItemWearType) = 0;

    ExpertMod                        m_Expert[10]{};
    CreatureStat                     m_cStat{ };
    CreatureStat                     m_cStatByState{ };
    CreatureAtributeServer           m_Attribute{ };
    CreatureAtributeServer           m_cAtributeByState{ };
    CreatureStatAmplifier            m_StatAmplifier{ };
    CreatureAttributeAmplifier       m_AttributeAmplifier{ };
    CreatureElementalResist          m_Resist{};
    CreatureElementalResistAmplifier m_ResistAmplifier{ };

// 	GameObject* GetGameObject(uint32 spellId) const;
// 	void AddGameObject(GameObject* gameObj);
// 	void RemoveGameObject(GameObject* gameObj, bool del);
// 	void RemoveGameObject(uint32 spellid, bool del);
// 	void RemoveAllGameObjects();
    Unit(bool isWorldObject);
    Item       *m_anWear[Item::MAX_ITEM_WEAR]{ };
    std::vector<Skill*> m_vSkillList;
    uint m_nMovableTime{0};

    Skill* m_castingSkill{nullptr};

    float m_nRegenHP{}, m_fRegenMP{};
protected:
    virtual void onRegisterSkill(int,int,int,int) { };
    virtual void onExpChange() { };
    virtual void onAttackAndSkillProcess();
    virtual void onCantAttack(uint target, uint t) { };
    // Overwritten in Monster
    virtual int onDamage(Unit* pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical)
    { return nDamage; }

    virtual void onDead(Unit* pFrom, bool decreaseEXPOnDead);
    void processAttack();
    void broadcastAttackMessage(Unit* pTarget, AttackInfo arDamage[], int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack);
    //UNORDERED_MAP<int, Item> m_anWear;
    uint m_nLastUpdatedTime{};
    DeathState _deathState;
    void processPendingMove();
    uint32 m_unitTypeMask;
//	typedef std::list<GameObject*> GameObjectList;
//	GameObjectList m_gameObj;
private:
    void incParameter(uint nBitset, float nValue, bool bStat);
    void incParameter2(uint nBitset, float fValue);
    void ampParameter2(uint nBitset, float fValue);
    void ampParameter(uint nBitset, float fValue, bool bStat);
};

#endif // !_UNIT_H_
