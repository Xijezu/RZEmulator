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
#include "LockedQueue.h"
#include "RegionContainer.h"
#include "RespawnObject.h"
#include "Timer.h"
#include <atomic>

enum ShutdownExitCode
{
    SHUTDOWN_EXIT_CODE = 0,
    ERROR_EXIT_CODE = 1,
    RESTART_EXIT_CODE = 2
};

enum WorldTimers : int
{
    WUPDATE_WORLDLOCATION,
    WUPDATE_PINGDB,
    WUPDATE_COUNT
};

enum WorldBoolConfigs : int
{
    CONFIG_PK_SERVER = 0,
    CONFIG_DISABLE_TRADE,
    CONFIG_MONSTER_WANDERING,
    CONFIG_MONSTER_COLLISION,
    CONFIG_IGNORE_RANDOM_DAMAGE,
    CONFIG_NO_COLLISION_CHECK,
    CONFIG_NO_SKILL_COOLTIME,
    CONFIG_SERVICE_SERVER,
    CONFIG_MONSTER_PATHFINDING,
    BOOL_CONFIG_VALUE_COUNT
};

enum WorldFloatConfigs
{
    CONFIG_MAP_LENGTH = 0,
    FLOAT_CONFIG_VALUE_COUNT
};

enum WorldIntConfigs
{
    CONFIG_MAP_WIDTH = 0,
    CONFIG_MAP_HEIGHT,
    CONFIG_MAP_REGION_SIZE,
    CONFIG_CELL_SIZE,
    CONFIG_REGION_SIZE,
    CONFIG_TILE_SIZE,
    CONFIG_ITEM_HOLD_TIME, // Item disappears after X seconds when not looted
    CONFIG_MAX_LEVEL,
    CONFIG_LOCAL_FLAG,
    CONFIG_SERVER_INDEX,
    INT_CONFIG_VALUE_COUNT
};

enum Rates
{
    RATES_EXP = 0,
    RATES_ITEM_DROP,
    RATES_CREATURE_DROP,
    RATES_CHAOS_DROP,
    RATES_GOLD_DROP,
    RATES_PVP_DAMAGE_FOR_PLAYER,
    RATES_PVP_DAMAGE_FOR_SUMMON,
    RATES_STAMINA_BONUS,
    MAX_RATES
};

class WorldSession;
class Unit;
class Player;
class Summon;
class XPacket;

typedef std::unordered_map<uint32_t, WorldSession *> SessionMap;

class World
{
  public:
    static std::atomic<uint32_t> m_worldLoopCounter;
    typedef std::atomic<uint64_t> AtomicIndex;
    ~World();
    // Deleting the copy & assignment operators
    // Better safe than sorry
    World(const World &) = delete;
    World &operator=(const World &) = delete;

    static World &Instance()
    {
        static World instance;
        return instance;
    }

    void InitWorld();
    void LoadConfigSettings(bool reload);

    bool SetMultipleMove(Unit *pUnit, Position curPos, std::vector<Position> newPos, uint8_t speed, bool bAbsoluteMove, uint32_t t, bool bBroadcastMove);
    bool SetMove(Unit *obj, Position curPos, Position newPos, uint8_t speed, bool bAbsoluteMove, uint32_t t, bool bBroadcastMove = true);
    void MoveObject(Unit *pObject, Position &newPos, float face);
    void EnumMovableObject(Position pos, uint8_t layer, float range, std::vector<uint32_t> &pvResult, bool bIncludeClient = true, bool bIncludeNPC = true);

    void addEXP(Unit *pCorpse, Player *pPlayer, int32_t exp, float jp);
    void addEXP(Unit *pCorpse, int32_t nPartyID, int32_t exp, float jp);

    WorldSession *FindSession(uint32_t id) const;
    void AddSession(WorldSession *s);
    bool RemoveSession(uint32_t id);
    void UpdateSessions(uint32_t diff);
    void KickAll();

    void AddObjectToWorld(WorldObject *obj);
    void AddSummonToWorld(Summon *pSummon);
    void AddMonsterToWorld(Monster *pMonster);
    void AddItemToWorld(Item *pItem);
    bool RemoveItemFromWorld(Item *pItem);
    void RemoveObjectFromWorld(WorldObject *obj);
    void MonsterDropItemToWorld(Unit *pUnit, Item *pItem);
    bool checkDrop(Unit *pKiller, int32_t code, int32_t percentage, float fDropRatePenalty, float fPCBangDropRateBonus);
    bool ProcTame(Monster *pMonster);
    void ClearTamer(Monster *pMonster, bool bBroadcastMsg);
    bool SetTamer(Monster *pMonster, Player *pPlayer, int32_t nSkillLevel);

    // Quest?
    int32_t ShowQuestMenu(Player *pPlayer);

    // Item
    uint32_t procAddItem(Player *pClient, Item *pItem, bool bIsPartyProcess);
    void procPartyShare(Player *pClient, Item *pItem);
    void addChaos(Unit *pCorpse, Player *pPlayer, float chaos);

    // Warping
    void WarpBegin(Player *);
    void WarpEnd(Player *, Position, uint8_t);
    void WarpEndSummon(Player *, Position, uint8_t, Summon *, bool);

    void onRegionChange(WorldObject *obj, uint32_t update_time, bool bIsStopMessage);

    /// Get the number of current active sessions
    const SessionMap &GetAllSessions() const { return m_sessions; }

    uint32_t GetSessionCount() const { return m_sessions.size(); }

    template <typename TS_PACKET>
    void Broadcast(uint32_t rx1, uint32_t ry1, uint32_t rx2, uint32_t ry2, uint8_t layer, TS_PACKET packet)
    {
        BroadcastFunctor<TS_PACKET> broadcastFunctor;
        broadcastFunctor.packet = packet;

        sRegion.DoEachVisibleRegion(rx1, ry1,
                                    rx2, ry2,
                                    layer,
                                    NG_REGION_FUNCTOR(broadcastFunctor),
                                    (uint8_t)RegionVisitor::ClientVisitor);
    }

    template <typename TS_PACKET>
    void Broadcast(uint32_t rx, uint32_t ry, uint8_t layer, TS_PACKET packet)
    {
        BroadcastFunctor<TS_PACKET> broadcastFunctor;
        broadcastFunctor.packet = packet;

        sRegion.DoEachVisibleRegion(rx, ry,
                                    layer,
                                    NG_REGION_FUNCTOR(broadcastFunctor),
                                    (uint8_t)RegionVisitor::ClientVisitor);
    }

    uint32_t GetArTime();

    void Update(uint32_t);

    static uint8_t GetExitCode() { return m_ExitCode; }

    static void StopNow(uint8_t exitcode)
    {
        m_stopEvent = true;
        m_ExitCode = exitcode;
    }

    static bool IsStopped() { return m_stopEvent; }

    /// Gets and increments the identifier for DB insert statements
    uint64_t GetItemIndex();
    uint64_t GetPlayerIndex();
    uint64_t GetPetIndex();
    uint64_t GetStateIndex();
    uint64_t GetSummonIndex();
    uint64_t GetSkillIndex();

    /// Set a server rate
    void setRate(Rates rate, float value) { rate_values[rate] = value; }

    /// Get a server rate
    float getRate(Rates rate) const { return rate_values[rate]; }

    /// Set a float configuration element
    void setFloatConfig(WorldFloatConfigs index, float value)
    {
        if (index < FLOAT_CONFIG_VALUE_COUNT)
            m_float_configs[index] = value;
    }

    /// Get a float configuration element
    float getFloatConfig(WorldFloatConfigs index)
    {
        return index < FLOAT_CONFIG_VALUE_COUNT ? m_float_configs[index] : 0.0f;
    }

    /// Set a server configuration element
    void setBoolConfig(WorldBoolConfigs index, bool value)
    {
        if (index < BOOL_CONFIG_VALUE_COUNT)
            m_bool_configs[index] = value;
    }

    /// Get a server configuration element
    bool getBoolConfig(WorldBoolConfigs index) const
    {
        return index < BOOL_CONFIG_VALUE_COUNT ? m_bool_configs[index] : false;
    }

    /// Set a server configuration element
    void setIntConfig(WorldIntConfigs index, int32_t value)
    {
        if (index < INT_CONFIG_VALUE_COUNT)
            m_int_configs[index] = value;
    }

    /// Get a server configuration element
    int32_t getIntConfig(WorldIntConfigs index) const
    {
        return index < INT_CONFIG_VALUE_COUNT ? m_int_configs[index] : 0;
    }

    std::vector<RespawnObject *> m_vRespawnList{};

  private:
    static std::atomic<bool> m_stopEvent;

    static uint8_t m_ExitCode;

    SessionMap m_sessions;
    const uint32_t startTime;

    void AddSession_(WorldSession *s);
    LockedQueue<WorldSession *> addSessQueue;

    void onMoveObject(WorldObject *pUnit, Position oldPos, Position newPos);
    bool onSetMove(WorldObject *pObject, Position curPos, Position lastpos);
    void enterProc(WorldObject *pUint, uint32_t prx, uint32_t pry);
    void step(WorldObject *obj, uint32_t tm);

    AtomicIndex s_nPlayerIndex{0};
    AtomicIndex s_nPetIndex{0};
    AtomicIndex s_nSummonIndex{0};
    AtomicIndex s_nSkillIndex{0};
    AtomicIndex s_nStateIndex{0};
    AtomicIndex s_nItemIndex{0};

    float m_float_configs[FLOAT_CONFIG_VALUE_COUNT];
    float rate_values[MAX_RATES];
    int32_t m_int_configs[INT_CONFIG_VALUE_COUNT];
    bool m_bool_configs[BOOL_CONFIG_VALUE_COUNT];

    IntervalTimer m_timers[WUPDATE_COUNT];

  protected:
    World();
};

#define sWorld World::Instance()
