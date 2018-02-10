#ifndef _UNIT_H_
#define _UNIT_H_

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
    STATUS_LOGIN_COMPLETE         = 0x01,
    STATUS_FIRST_ENTER            = 0x02,
    STATUS_ATTACK_STARTED         = 0x04,
    STATUS_FIRST_ATTACK           = 0x08,
    STATUS_MOVE_PENDED            = 0x10,
    STATUS_NEED_TO_UPDATE_STATE   = 0x20,
    STATUS_MOVING_BY_FEAR         = 0x40,
    STATUS_NEED_TO_CALCULATE_STAT = 0x80,
    STATUS_PROCESSING_REFELCT     = 0x100,
    STATUS_INVISIBLE              = 0x200,
    STATUS_INVINCIBLE             = 0x400,
    STATUS_HIDING                 = 0x800,
    STATUS_MOVABLE                = 0x2000,
    STATUS_ATTACKABLE             = 0x4000,
    STATUS_SKILL_CASTABLE         = 0x8000,
    STATUS_MAGIC_CASTABLE         = 0x10000,
    STATUS_ITEM_USABLE            = 0x20000,
    STATUS_MORTAL                 = 0x40000,
    STATUS_HAVOC_BURST            = 0x80000,
    STATUS_FEARED                 = 0x100000,
    STATUS_FORM_CHANGED           = 0x200000,
    STATUS_MOVE_SPEED_FIXED       = 0x400000,
    STATUS_HP_REGEN_STOPPED       = 0x800000,
    STATUS_MP_REGEN_STOPPED       = 0x1000000,
    STATUS_USING_DOUBLE_WEAPON    = 0x2000000,
};

enum DamageType : int
{
    DT_NORMAL_PHYSICAL_DAMAGE           = 0,
    DT_NORMAL_MAGICAL_DAMAGE            = 1,
    DT_NORMAL_PHYSICAL_SKILL_DAMAGE     = 2,
    DT_ADDITIONAL_DAMAGE                = 3,
    DT_NORMAL_PHYSICAL_LEFT_HAND_DAMAGE = 4,
    DT_ADDITIONAL_LEFT_HAND_DAMAGE      = 5,
    DT_ADDITIONAL_MAGICAL_DAMAGE        = 6,
    DT_STATE_MAGICAL_DAMAGE             = 7,
    DT_STATE_PHYSICAL_DAMAGE            = 8,
};

enum DamageFlag : int
{
    IGNORE_AVOID = 2,
    IGNORE_DEFENCE = 4,
    IGNORE_BLOCK = 8,
    IGNORE_CRITICAL = 16,
};

class Unit : public WorldObject
{
    public:
        friend class Messages;
        friend class Skill;
        friend class Player;
        friend class XLua;
        friend class WorldSession;

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
        virtual int GetMoveSpeed();
        uint GetNextAttackableTime() const { return GetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME); }

        float GetRealAttackRange() const { return (12 * m_Attribute.nAttackRange) / 100.0f; }

        uint GetAttackInterval() const { return (uint)(100.0f / m_Attribute.nAttackSpeed * 115.0f); };

        /* FloatCreatureMastery */
        virtual float GetFCM() const { return 1.0f; }
        State *GetState(StateCode code);
        Damage CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag);
        DamageInfo DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
        DamageInfo DealPhysicalNormalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
        Damage DealDamage(Unit *pFrom, float nDamage, ElementalType type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
        Damage DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
        Damage DealPhysicalLeftHandDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
        Damage DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
        Damage DealPhysicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);
        Damage DealMagicalStateDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage);

        DamageInfo DealMagicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
        DamageInfo DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag);
        int damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead);

        float GetManaCostRatio(ElementalType type, bool bPhysical, bool bBad);

        int Heal(int hp);
        int MPHeal(int mp);
        int HealByItem(int hp);
        int MPHealByItem(int mp);
        virtual bool IsMovable();

        /// BATTLE END
        void RemoveState(StateCode code, int state_level);
        void RemoveState(int uid);

        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }

        // Setters
        void SetLevel(uint8 lvl) { SetInt32Value(UNIT_FIELD_LEVEL, lvl); }

        void SetCurrentJob(uint job) { SetUInt32Value(UNIT_FIELD_JOB, job); }

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
        virtual bool IsActable() const;
        virtual bool IsSitdown() const { return false; }

        void SetMaxHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_MAX_HEALTH, val); };

        void SetMaxMana(uint32 val) { SetUInt32Value(UNIT_FIELD_MAX_MANA, val); };

        void SetFullHealth() { SetHealth(GetMaxHealth()); }

        void SetEXP(int64 exp);

        // eh
        int GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad) const;

        float GetCoolTimeSpeed() const { return m_Attribute.nCoolTimeSpeed / 100.0f; }

        float GetCoolTimeMod(ElementalType type, bool bPhysical, bool bBad) const { return 1.0f; }

        uint GetRemainCoolTime(int skill_id) const;
        uint GetTotalCoolTime(int skill_id) const;

        uint GetBowAttackInterval() { return (uint)((float)GetAttackInterval() * (1.0f - m_fBowInterval)); }

        uint GetBowInterval() { return (uint)((float)GetAttackInterval() * m_fBowInterval); }

        // Getters
        uint32_t GetLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        virtual int GetRace() const { return GetInt32Value(UNIT_FIELD_RACE); }
        uint32 GetHealth() const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAX_HEALTH); }
        uint32 GetMana() const { return GetUInt32Value(UNIT_FIELD_MANA); }
        uint32 GetMaxMana() const { return GetUInt32Value(UNIT_FIELD_MAX_MANA); }
        uint32 GetCurrentJob() const { return GetUInt32Value(UNIT_FIELD_JOB); };
        uint32 GetCurrentJLv() const { return GetUInt32Value(UNIT_FIELD_JLV); }
        int GetStamina() const { return GetInt32Value(UNIT_FIELD_STAMINA); }
        uint32 GetJP() const { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }
        uint32 GetTotalJP() const { return GetUInt32Value(UNIT_FIELD_JOBPOINT); }
        float GetCastingMod(ElementalType type, bool bPhysical, bool bBad, uint nOriginalCoolTime) { return 1.0f; }
        float GetItemChance() const;
        uint64 GetEXP() const { return GetUInt64Value(UNIT_FIELD_EXP); }
        virtual uint GetCreatureGroup() const;

        void AddHealth(int hp) { SetHealth(GetHealth() + hp); }

        void AddMana(int mp) { SetMana(GetMana() + mp); }

        virtual void AddEXP(int64 exp, uint jp, bool bApplyStanima);
        void CancelSkill();
        void CancelAttack();
        int CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem);
        bool OnCompleteSkill();
        void SetMultipleMove(std::vector<Position>& _to, uint8_t _speed, uint _start_time);
        void SetMove(Position _to, uint8 _speed, uint _start_time);
        void CalculateStat();
        int GetArmorClass() const;
        // Event handler
        bool IsWornByCode(int code) const;
        virtual bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList);
        Item *GetWornItem(ItemWearType);
        ushort Puton(ItemWearType pos, Item *item);
        ushort Putoff(ItemWearType pos);
        ItemWearType GetAbsoluteWearPos(ItemWearType pos);
        ItemClass GetWeaponClass();
        bool IsWearShield();
        std::pair<float, int> GetHateMod(int nHateModType, bool bIsHarmful);

        virtual CreatureStat *GetBaseStat() const { return nullptr; }

        virtual bool IsUsingBow() const { return false; }

        virtual bool IsUsingCrossBow() const { return false; }

        bool TurnOnAura(Skill *pSkill);
        bool TurnOffAura(Skill *pSkill);
        void ToggleAura(Skill *pSkill);
        bool IsActiveAura(Skill *pSkill) const;

        uint16 AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura, int nStateValue, std::string szStateValue);
        explicit Unit(bool isWorldObject);
    protected:
        uint16 onItemUseEffect(Unit *pCaster, Item *pItem, int type, float var1, float var2, const std::string &szParameter);
        void applyStatByItem();
        virtual void procStateDamage(uint t);

        virtual void applyJobLevelBonus() {};

        virtual void onModifyStatAndAttribute() {};

        virtual void onBeforeCalculateStat() {};
        virtual void applyPassiveSkillEffect();
        void applyStatByState();
        void getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute);
        void amplifyStatByState();
        virtual void applyState(State &state);
        void applyStateEffect();
        void applyStateAmplifyEffect();
        void applyPassiveSkillAmplifyEffect();

        virtual void applyPassiveSkillAmplifyEffect(Skill *) {}

        void applyStateAmplify(State &state);
        void applyDoubeWeaponEffect();

        virtual void onApplyAttributeAdjustment() {};
        virtual void applyPassiveSkillEffect(Skill *skill);
        void getAmplifiedStatByAmplifier(CreatureStat &);
        void finalizeStat();
        void calcAttribute(CreatureAtributeServer &attribute);
        void applyItemEffect();

        virtual void onCompleteCalculateStat() {}; /* overwritten in player class */
        int64 GetBulletCount() const;
        ///- Gets overwritten in player for Beltslots and Max Chaos
        virtual void onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio);
        State *GetStateByEffectType(int effectType) const;

        virtual void onRegisterSkill(int64, int, int, int) {};

        virtual void onExpChange() {};

        virtual void onJobLevelUp() {};
        virtual void onAttackAndSkillProcess();

        virtual void onCantAttack(uint target, uint t) {};
        virtual uint16_t putonItem(ItemWearType pos, Item *item);
        virtual uint16_t putoffItem(ItemWearType);

        // Overwritten in Monster
        virtual int onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical) { return nDamage; }

        virtual void onDead(Unit *pFrom, bool decreaseEXPOnDead);
        void processAttack();
        void broadcastAttackMessage(Unit *pTarget, AttackInfo arDamage[], int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack);
        void onAfterAddState(State);
        void onUpdateState(State state, bool bIsExpire);
        void procMoveSpeedChange();
        void processPendingMove();
        void _InitTimerFieldsAndStatus();
        std::vector<State>          m_vStateList{ };
        uint32                      m_unitTypeMask;
        //	typedef std::list<GameObject*> GameObjectList;
        //	GameObjectList m_gameObj;

        ExpertMod                        m_Expert[10]{ };
        CreatureStat                     m_cStat{ };
        CreatureStat                     m_cStatByState{ };
        CreatureAtributeServer           m_Attribute{ };
        CreatureAtributeServer           m_AttributeByState{ };
        CreatureStatAmplifier            m_StatAmplifier{ };
        CreatureAttributeAmplifier       m_AttributeAmplifier{ };
        CreatureElementalResist          m_Resist{ };
        CreatureElementalResistAmplifier m_ResistAmplifier{ };
        std::vector<Skill *>             m_vSkillList;
        std::vector<HateModifier>        m_vHateMod{ };

        ///- Additional Damage
        std::vector<AdditionalDamageInfo> m_vNormalAdditionalDamage{ };
        std::vector<AdditionalDamageInfo> m_vRangeAdditionalDamage{ };
        std::vector<AdditionalDamageInfo> m_vPhysicalSkillAdditionalDamage{ };
        std::vector<AdditionalDamageInfo> m_vMagicialSkillAdditionalDamage{ };

        Item  *m_anWear[MAX_ITEM_WEAR]{nullptr};
        uint  m_nMovableTime{0};
        int   m_nUnitExpertLevel{0};
        int   m_nNextAttackMode{0};
        Skill *m_castingSkill{nullptr};
        float m_nRegenHP{ }, m_fRegenMP{ };
    private:
        UNORDERED_MAP<int, Skill *> m_vAura;

        float m_fBowInterval{0};
        bool ClearExpiredState(uint t);
        uint  m_nCurrentStateUID{0};
        int   m_nDoubleWeaponMasteryLevel{0};
        void incParameter(uint nBitset, float nValue, bool bStat);
        void incParameter2(uint nBitset, float fValue);
        void ampParameter2(uint nBitset, float fValue);
        void ampParameter(uint nBitset, float fValue, bool bStat);
};
#endif // !_UNIT_H_
