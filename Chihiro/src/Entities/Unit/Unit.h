#ifndef _UNIT_H_
#define _UNIT_H_

#include "Common.h"
#include "XPacket.h"
#include "Entities/Object/Object.h"
#include "Utilities/Util.h"
#include "Item.h"
#include "Utilities/EventProcessor.h"
#include "CreatureAttribute.h"

class World;

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

struct Skill {
    int sid;
    int owner_id;
    int summon_id;
    int skill_id;
    int skill_level;
    int cool_time;
};

class Unit : public WorldObject {
public:
    virtual ~Unit();

    static void EnterPacket(XPacket&, Unit*);

    void AddToWorld();
    void RemoveFromWorld();
    void CleanupBeforeRemoveFromMap(bool finalCleanup);

    void CleanupsBeforeDelete(bool finalCleanup = true);                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

    virtual void Update(uint32 time);

    uint32 HasUnitTypeMask(uint32 mask) const
    { return mask & m_unitTypeMask; }

    void AddUnitTypeMask(uint32 mask)
    { m_unitTypeMask |= mask; }
// 	bool isSummon() const { return m_unitTypeMask & UNIT_MASK_SUMMON; }
// 	bool isPet() const { return m_unitTypeMask & UNIT_MASK_PET; }

    uint32_t getLevel() const
    { return GetUInt32Value(UNIT_FIELD_LEVEL); }

    uint8 getLevelForTarget(WorldObject const * /*target*/) const
    { return getLevel(); }

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

    void SetCurrentJob(int job)
    { SetInt32Value(UNIT_FIELD_JOB, job); }

    int32 GetCurrentJob() const
    { return GetInt32Value(UNIT_FIELD_JOB); };

    void SetCurrentJLv(int jlv)
    { SetInt32Value(UNIT_FIELD_JLV, jlv); }

    int32 GetCurrentJLv() const
    { return GetInt32Value(UNIT_FIELD_JLV); }

    uint32 GetStamina() const
    { return GetUInt32Value(UNIT_FIELD_STAMINA); }

    void SetJP(int jp)
    { SetInt32Value(UNIT_FIELD_JOBPOINT, jp); }

    uint32 GetJP() const
    { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }

    uint32 GetTotalJP() const
    { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }

    void SetEXP(uint exp)
    { SetUInt32Value(UNIT_FIELD_EXP, exp); }

    uint32 GetEXP() const
    { return uint32(GetUInt32Value(UNIT_FIELD_EXP)); }

    bool IsFullHealth() const
    { return GetHealth() == GetMaxHealth(); }

    bool HealthBelowPct(int32 pct) const
    { return GetHealth() * uint64(100) < GetMaxHealth() * uint64(pct); }

    bool HealthAbovePct(int32 pct) const
    { return GetHealth() * uint64(100) > GetMaxHealth() * uint64(pct); }

    bool HealthAbovePctHealed(int32 pct, uint32 heal) const
    { return (GetHealth() + heal) * uint64(100) > GetMaxHealth() * uint64(pct); }

    float GetHealthPct() const
    { return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f; }

    void SetHealth(uint32);
    void SetMana(uint32);

    void SetMaxHealth(uint32 val)
    { SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val); };

    void SetMaxMana(uint32 val)
    { SetUInt32Value(UNIT_FIELD_MAX_MANA, val); };

    inline void SetFullHealth()
    { SetHealth(GetMaxHealth()); }

    void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);

    virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false);

    // returns true if unit's position really changed
    bool UpdatePosition(const Position &pos, bool teleport = false)
    { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }

    void UpdateOrientation(float orientation);

    bool isAlive() const
    { return (_deathState == ALIVE); };

    bool isDying() const
    { return (_deathState == JUST_DIED); };

    bool isDead() const
    { return (_deathState == DEAD); };

    DeathState getDeathState()
    { return _deathState; };

    virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

    void SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time);

    // Event handler
    EventProcessor _Events;

    void CalculateStat();
    void applyStatByItem();
    virtual void applyJobLevelBonus() = 0;
    void getAmplifiedStatByAmplifier(CreatureStat &);
    void finalizeStat();
    void calcAttribute(CreatureAtributeServer &attribute);
    void applyItemEffect();
    void onItemWearEffect(Item pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio);
    Item *GetWornItem(ItemWearType);
    virtual uint16_t putonItem(ItemWearType pos, Item *item) = 0;
    virtual uint16_t putoffItem(ItemWearType) = 0;

    CreatureStat                     m_cStat{ };
    CreatureStat                     m_cStatByState{ };
    CreatureAtributeServer           m_cAtribute{ };
    CreatureAtributeServer           m_cAtributeByState{ };
    CreatureStatAmplifier            m_StatAmplifier{ };
    CreatureAttributeAmplifier       m_AttributeAmplifier{ };
    CreatureElementalResistAmplifier m_ResistAmplifier{ };

// 	GameObject* GetGameObject(uint32 spellId) const;
// 	void AddGameObject(GameObject* gameObj);
// 	void RemoveGameObject(GameObject* gameObj, bool del);
// 	void RemoveGameObject(uint32 spellid, bool del);
// 	void RemoveAllGameObjects();
    Unit(bool isWorldObject);
    Item       *m_anWear[Item::MAX_ITEM_WEAR]{ };
    std::vector<Skill> m_vSkillList;
    uint m_nMovableTime{0};
protected:
    //UNORDERED_MAP<int, Item> m_anWear;
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
    bool m_bIsFirstEnter{true};
};

#endif // !_UNIT_H_
