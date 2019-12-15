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
#include "MonsterBase.h"
#include "Unit.h"

struct VirtualParty
{
    VirtualParty() = default;

    VirtualParty(int32_t id, int32_t d, int32_t lv)
    {
        fContribute = 0.0f;
        hPlayer = 0;
        nPartyID = id;
        nDamage = d;
        nLevel = lv;
        bTamer = false;
    }

    VirtualParty(uint32_t h, int32_t d, int32_t lv)
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

    int32_t nPartyID;
    uint32_t hPlayer;
    int32_t nDamage;
    float fContribute;
    bool bTamer;
    int32_t nLevel;
};

struct DamageTag
{
    DamageTag(uint32_t _uid, uint32_t _time, int32_t _damage)
    {
        uid = _uid;
        nTime = _time;
        nDamage = _damage;
    }

    uint32_t uid;
    uint32_t nTime;
    int32_t nDamage;
};

struct WayPointInfo;
class Monster : public Unit
{
    friend class World;
    friend class GameContent;

  public:
    static void EnterPacket(XPacket &pEnterPct, Monster *monster, Player *pPlayer);
    explicit Monster(uint32_t handle, MonsterBase *mb);
    ~Monster() = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    Monster(const Monster &) = delete;
    Monster &operator=(const Monster &) = delete;

    void Update(uint32_t) override;
    void OnUpdate() override;
    bool StartAttack(uint32_t target, bool bNeedFastReaction) override;
    void SetRespawnPosition(Position pos) { m_pRespawn = pos; }
    MonsterBase *GetBase() const { return m_Base; }
    void applyJobLevelBonus() override{};
    float GetSize() const override { return m_Base->size; }
    float GetScale() const override { return m_Base->scale; }

    const std::string &GetNameAsString() override;

    MONSTER_STATUS GetStatus() const { return m_nStatus; }

    void SetStatus(MONSTER_STATUS status);
    void SetTamer(uint32_t handle, int32_t nTamingSkillLevel);

    uint32_t GetCreatureGroup() const override;
    bool IsEnvironmentMonster() const;
    bool IsBattleMode() const; // override;
    bool IsBossMonster() const;
    bool IsDungeonConnector() const;
    bool IsDungeonCore() const;
    bool IsAgent() const;
    bool IsAutoTrap() const;

    bool IsAttackable() override;
    bool IsMovable() override;
    //bool IsKnockbackable() override;

    bool IsMonster() const override { return true; }

    float GetChaseRange() const;
    float GetFirstAttackRange();
    bool IsFirstAttacker() const;
    bool IsGroupFirstAttacker() const;
    uint32_t GetTamer() const;
    bool IsCastRevenger() const;
    bool IsBattleRevenger() const;
    int32_t GetMonsterGroup() const;
    float GetTameExpAdjust() const { return m_Base != nullptr ? m_Base->taming_exp_mod : 1.0f; }
    int32_t GetTameItemCode() const;
    int32_t GetTameCode() const;
    float GetTamePercentage() const;
    int32_t GetMonsterID() const;
    CreatureStat *GetBaseStat() const override;
    int32_t GetRace() const override;

    int32_t GetHate(uint32_t handle);
    int32_t RemoveHate(uint32_t handle, int32_t pt);

    int32_t AddHate(uint32_t handle, int32_t pt, bool bBroadcast = true, bool bProcRoamingMonster = true);
    bool IsAlly(const Unit *pTarget) override;
    void TriggerForceKill(Player *pPlayer);

    MonsterDeleteHandler *m_pDeleteHandler{nullptr};
    bool m_bNearClient;

  protected:
    HateTag *getHateTag(uint32_t handle, uint32_t t);
    HateTag *addHate(uint32_t handle, int32_t nHate);
    bool removeFromHateList(uint32_t handle);

    void processWalk(uint32_t t);
    void processMove(uint32_t t);
    void ForceKill(Player *byPlayer);
    void processFirstAttack(uint32_t t);
    void FindAttackablePosition(Position &myPosition, Position &enemyPosition, float distance, float gap);
    void getMovePosition(Position &newPos);
    Position getNonDuplicateAttackPos(Unit *pEnemy);

    void onBeforeCalculateStat() override;
    void onApplyAttributeAdjustment() override;
    void comeBackHome(bool bInvincible);
    int32_t onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int32_t nDamage, bool bCritical) override;
    void onDead(Unit *pFrom, bool decreaseEXPOnDead) override;
    void processDead(uint32_t t); //override;
  private:
    void findNextEnemy();
    void AI_processAttack(uint32_t t);
    void AI_processAttack(Unit *pEnemy, uint32_t t);

    DamageTag *addDamage(uint32_t handle, int32_t nDamage);
    DamageTag *getDamageTag(uint32_t handle, uint32_t t);
    void calcPartyContribute(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute);
    void procEXP(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute);
    void procDropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
    void procQuest(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute);
    void procDropGold(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
    void procDropChaos(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty);
    void dropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int32_t code, long count, int32_t level, bool bIsEventItem, int32_t nFlagIndex);
    void dropItemGroup(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int32_t nDropGroupID, long count, int32_t level, int32_t nFlagIndex);

    std::vector<DamageTag> m_vDamageList{};
    std::vector<HateTag> m_vHateList{};
    std::vector<HateModifierTag> m_vHateModifierByState{};

    Position m_pRespawn{};
    MonsterBase *m_Base{nullptr};

    float m_nLastEnemyDistance;
    uint32_t m_nLastTrackTime;

    bool m_bComeBackHome;
    uint32_t m_nLastHateUpdateTime;
    // Attack related
    bool m_bNeedToFindEnemy{false};
    uint32_t m_hFirstAttacker;
    uint32_t m_nFirstAttackTime;
    uint32_t m_nTotalDamage;
    int32_t m_nMaxHate;
    // Taming related
    int32_t m_nTamingSkillLevel;
    uint32_t m_hTamer;
    int32_t m_nTamedTime;
    Player *pFCClient{nullptr};
    bool bForceKill{false};
    bool m_bTamedSuccess;
    int32_t m_nWayPointIdx;
    bool m_bIsWandering;
    WayPointInfo *m_pWayPointInfo;

    MONSTER_STATUS m_nStatus;
};