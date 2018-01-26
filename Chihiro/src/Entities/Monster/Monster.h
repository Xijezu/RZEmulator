#ifndef PROJECT_MONSTER_H
#define PROJECT_MONSTER_H

#include "Common.h"
#include "Unit.h"

class Monster;
struct MonsterDeleteHandler
{
    virtual void onMonsterDelete(Monster* mob) = 0;
};

struct takePriority {
    ItemPickupOrder PickupOrder{};
};

struct HateTag {
    HateTag(uint _uid, uint _time, int _hate)
    {
        uid   = _uid;
        nTime = _time;
        nHate = _hate;
    }

    uint uid;
    uint nTime;
    int  nHate;
    bool bIsActive;
    int  nBadAttackCount;
    int  nLastMaxHate;
};

struct HateModifierTag {
    HateModifierTag(uint _uid, int _hate)
    {
        uid   = _uid;
        nHate = _hate;
    }

    uint uid;
    int  nHate;
};


enum MonsterFlag : uint16 {
    MF_FIRST_ATTACK = 0,
    MF_GROUP_FIRST_ATTACK,
    MF_RESPONCE_CASTING,
    MF_RESPONCE_RACE,
    MF_RESPONCE_BATTLE
};

enum MonsterGenerateCode : int
{
    MGC_None = 0,
    MGC_ByRespawn = 1,
    MGC_ByScript = 2,
    MGC_ByShoveling = 3,
};

enum MonsterStatus : int
{
    MS_Normal = 0,
    MS_Tracking = 1,
    MS_FindAttackPos = 2,
    MS_Attack = 3,
    MS_Dead = 4,
};

struct MonsterRespawnInfo {
    uint interval;
    float left;
    float top;
    float right;
    float bottom;
    uint8 layer;
    uint monster_id;
    uint max_num;
    uint inc;
    uint id;
    bool is_wandering;
    int dungeon_id;
    int way_point_id;

    MonsterRespawnInfo() = default;

    MonsterRespawnInfo(uint _id, uint _interval, float _left, float _top, float _right, float _bottom, uint _monster_id, uint _max_num, uint _inc, bool _is_wandering, int _way_point_id)
    {
        id = _id;
        interval = _interval;
        left = _left;
        top = _top;
        right = _right;
        bottom = _bottom;
        layer = 0;
        dungeon_id = 0;
        monster_id = _monster_id;
        max_num = _max_num;
        inc = _inc;
        is_wandering = _is_wandering;
        way_point_id = _way_point_id;
    }
};

struct MonsterBase {
    int   id;
    int   monster_group;
    int   location_id;
    int   level;
    int   grp;
    float size;
    float scale;
    int   magic_type;
    int   race;
    int   visible_range;
    int   chase_range;
    int   flag[5];
    int   monster_type;
    int   stat_id;
    int   fight_type;
    int   weapon_type;
    int   attack_motion_speed;
    int   ability;
    int   standard_walk_speed;
    int   standard_run_speed;
    int   walk_speed;
    int   run_speed;
    float attack_range;
    int   hp;
    int   mp;
    int   attacK_point;
    int   magic_point;
    int   defence;
    int   magic_defence;
    int   attack_speed;
    int   magic_speed;
    int   accuracy;
    int   avoid;
    int   magic_accuracy;
    int   magic_avoid;
    int   taming_id;
    float taming_percentage;
    float taming_exp_mod;
    int   exp[2];
    int   jp[2];
    int   gold_drop_percentage;
    int   gold_min[2];
    int   gold_max[2];
    int   chaos_drop_percentage;
    int   chaos_min[2];
    int   chaos_max[2];
    int   drop_item_id[10];
    int   drop_percentage[10];
    int   drop_min_count[10];
    int   drop_max_count[10];
    int   drop_min_level[10];
    int   drop_max_level[10];
    int   skill_id[4];
    int   skill_lv[4];
    float skill_probability[4];
    int   local_flag;
};

struct VirtualParty {
    VirtualParty() = default;

    VirtualParty(int id, int d, int lv)
    {
        fContribute = 0.0f;
        hPlayer = 0;
        nPartyID = id;
        nDamage = d;
        nLevel = lv;
        bTamer = false;
    }

    VirtualParty(uint h, int d, int lv)
    {
        fContribute = 0.0f;
        hPlayer = h;
        nPartyID = 0;
        nDamage = d;
        nLevel = lv;
        bTamer = false;
    }

    static bool GreaterByDamage(VirtualParty lh, VirtualParty rh)
    {
        return lh.nDamage > rh.nDamage;
    }

    static bool GreaterByContribute(VirtualParty lh, VirtualParty rh)
    {
        return lh.fContribute > rh.fContribute;
    }

    int nPartyID;
    uint hPlayer;
    int nDamage;
    float fContribute;
    bool bTamer;
    int nLevel;
};

struct DamageTag {
    DamageTag(uint _uid, uint _time, int _damage)
    {
        uid     = _uid;
        nTime   = _time;
        nDamage = _damage;
    }

    uint uid;
    uint nTime;
    int  nDamage;
};

class Monster : public Unit
{
        friend class World;
    public:

        static void EnterPacket(XPacket &pEnterPct, Monster *monster, Player *pPlayer);
        explicit Monster(uint handle, MonsterBase *mb);
        ~Monster() = default;

        void Update(uint) override;
        void OnUpdate() override;
        bool StartAttack(uint target, bool bNeedFastReaction) override;

        void SetRespawnPosition(Position pos) { m_pRespawn = pos; }

        MonsterBase *GetBase() const { return m_Base; }

        void applyJobLevelBonus() override {};

        float GetSize() const override { return m_Base->size; }

        float GetScale() const override { return m_Base->scale; }

        MonsterStatus GetStatus() const { return m_nStatus; }

        void SetStatus(MonsterStatus status);
        void SetTamer(uint handle, int nTamingSkillLevel);

        uint GetCreatureGroup() const override;
        bool IsEnvironmentMonster() const;
        bool IsBattleMode() const;// override;
        bool IsBossMonster() const;
        bool IsDungeonConnector() const;
        bool IsAgent() const;
        bool IsAutoTrap() const;
        bool IsNonAttacker() const;

        bool IsMonster() const override { return true; }

        float GetChaseRange() const;
        float GetFirstAttackRange();
        bool IsFirstAttacker() const;
        bool IsGroupFirstAttacker() const;
        uint GetTamer() const;
        bool IsCastRevenger() const;
        bool IsBattleRevenger() const;
        int GetMonsterGroup() const;
        int GetTameItemCode() const;
        int GetTameCode() const;
        float GetTamePercentage() const;
        int GetMonsterID() const;
        CreatureStat *GetBaseStat() const override;
        int GetRace() const override;

        int AddHate(uint handle, int pt, bool bBroadcast, bool bProcRoamingMonster);
        bool IsAlly(const Unit *pTarget) override;
        void TriggerForceKill(Player *pPlayer);

        MonsterDeleteHandler *m_pDeleteHandler{nullptr};
    protected:
        HateTag *getHateTag(uint handle, uint t);
        HateTag *addHate(uint handle, int nHate);
        bool removeFromHateList(uint handle);

        void processWalk(uint t);
        void processMove(uint t);
        void ForceKill(Player *byPlayer);
        void processFirstAttack(uint t);
        void FindAttackablePosition(Position &myPosition, Position &enemyPosition, float distance, float gap);
        void getMovePosition(Position &newPos);
        Position getNonDuplicateAttackPos(Unit *pEnemy);

        void onBeforeCalculateStat() override;
        void onApplyAttributeAdjustment() override;
        void comeBackHome(bool bInvincible);
        int onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical) override;
        void onDead(Unit *pFrom, bool decreaseEXPOnDead) override;
        void processDead(uint t);//override;
    private:
        void findNextEnemy();
        void AI_processAttack(uint t);
        void AI_processAttack(Unit *pEnemy, uint t);

        DamageTag *addDamage(uint handle, int nDamage);
        DamageTag *getDamageTag(uint handle, uint t);
        void calcPartyContribute(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute);
        void procEXP(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute);
        void procDropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
        void procQuest(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute);
        void procDropGold(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
        void procDropChaos(Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
        void dropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int code, long count, int level, bool bIsEventItem, int nFlagIndex);
        void dropItemGroup(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int nDropGroupID, long count, int level, int nFlagIndex);

        std::vector<DamageTag>       m_vDamageList{ };
        std::vector<HateTag>         m_vHateList{ };
        std::vector<HateModifierTag> m_vHateModifierByState{ };

        Position m_pRespawn{ };
        MonsterBase *m_Base{nullptr};

        float m_nLastEnemyDistance;
        int   m_nLastTrackTime;

        bool m_bComeBackHome;
        uint m_nLastHateUpdateTime;
        // Attack related
        bool m_bNeedToFindEnemy{false};
        uint m_hFirstAttacker;
        uint m_nFirstAttackTime;
        uint m_nTotalDamage;
        int  m_nMaxHate;
        uint m_hEnemy;
        // Taming related
        int  m_nTamingSkillLevel;
        uint m_hTamer;
        int  m_nTamedTime;
        Player *pFCClient{nullptr};
        bool bForceKill{false};
        bool m_bTamedSuccess;

        MonsterStatus m_nStatus;
};


#endif // PROJECT_MONSTER_H
