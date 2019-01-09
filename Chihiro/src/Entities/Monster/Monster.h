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
#include "Unit.h"
#include "MonsterBase.h"

struct VirtualParty
{
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

struct DamageTag
{
    DamageTag(uint _uid, uint _time, int _damage)
    {
        uid = _uid;
        nTime = _time;
        nDamage = _damage;
    }

    uint uid;
    uint nTime;
    int nDamage;
};

struct WayPointInfo;
class Monster : public Unit
{
    friend class World;
    friend class GameContent;

  public:
    static void EnterPacket(XPacket &pEnterPct, Monster *monster, Player *pPlayer);
    explicit Monster(uint handle, MonsterBase *mb);
    ~Monster() = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    Monster(const Monster &) = delete;
    Monster &operator=(const Monster &) = delete;

    void Update(uint) override;
    void OnUpdate() override;
    bool StartAttack(uint target, bool bNeedFastReaction) override;

    void SetRespawnPosition(Position pos) { m_pRespawn = pos; }

    MonsterBase *GetBase() const { return m_Base; }

    void applyJobLevelBonus() override{};

    float GetSize() const override { return m_Base->size; }

    float GetScale() const override { return m_Base->scale; }

    const std::string &GetNameAsString() override;

    MONSTER_STATUS GetStatus() const { return m_nStatus; }

    void SetStatus(MONSTER_STATUS status);
    void SetTamer(uint handle, int nTamingSkillLevel);

    uint GetCreatureGroup() const override;
    bool IsEnvironmentMonster() const;
    bool IsBattleMode() const; // override;
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

    int GetHate(uint32_t handle);
    int RemoveHate(uint32_t handle, int pt);

    int AddHate(uint handle, int pt, bool bBroadcast = true, bool bProcRoamingMonster = true);
    bool IsAlly(const Unit *pTarget) override;
    void TriggerForceKill(Player *pPlayer);

    MonsterDeleteHandler *m_pDeleteHandler{nullptr};
    bool m_bNearClient;

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
    void processDead(uint t); //override;
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

    std::vector<DamageTag> m_vDamageList{};
    std::vector<HateTag> m_vHateList{};
    std::vector<HateModifierTag> m_vHateModifierByState{};

    Position m_pRespawn{};
    MonsterBase *m_Base{nullptr};

    float m_nLastEnemyDistance;
    uint m_nLastTrackTime;

    bool m_bComeBackHome;
    uint m_nLastHateUpdateTime;
    // Attack related
    bool m_bNeedToFindEnemy{false};
    uint m_hFirstAttacker;
    uint m_nFirstAttackTime;
    uint m_nTotalDamage;
    int m_nMaxHate;
    // Taming related
    int m_nTamingSkillLevel;
    uint m_hTamer;
    int m_nTamedTime;
    Player *pFCClient{nullptr};
    bool bForceKill{false};
    bool m_bTamedSuccess;
    int m_nWayPointIdx;
    bool m_bIsWandering;
    WayPointInfo *m_pWayPointInfo;

    MONSTER_STATUS m_nStatus;
};