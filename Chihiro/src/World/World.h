#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "RespawnObject.h"
#include "Timer.h"
#include "LockedQueue.h"

enum ShutdownExitCode
{
    SHUTDOWN_EXIT_CODE = 0,
    ERROR_EXIT_CODE    = 1,
    RESTART_EXIT_CODE  = 2
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
    BOOL_CONFIG_VALUE_COUNT
};

enum WorldIntConfigs
{
    CONFIG_MAP_WIDTH = 0,
    CONFIG_MAP_HEIGHT,
    CONFIG_REGION_SIZE,
    CONFIG_TILE_SIZE,
    CONFIG_ITEM_HOLD_TIME, // Item disappears after X seconds when not looted
    CONFIG_MAX_LEVEL,
    CONFIG_LOCAL_FLAG,
    CONFIG_PINGDB,
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

typedef std::unordered_map<uint32, WorldSession *> SessionMap;
constexpr int                                      g_nRegionSize = 180;
constexpr int                                      g_nMapWidth   = 700000;
constexpr int                                      g_nMapHeight  = 1000000;
constexpr float                                    g_fMapLength  = 16128.0f;

class World
{
    public:
        static std::atomic<uint32>  m_worldLoopCounter;
        typedef std::atomic<uint64> AtomicIndex;
        ~World();

        static World &Instance()
        {
            static World instance;
            return instance;
        }

        void InitWorld();
        void LoadConfigSettings(bool reload);

        bool SetMultipleMove(Unit *pUnit, Position curPos, std::vector<Position> newPos, uint8_t speed, bool bAbsoluteMove, uint t, bool bBroadcastMove);
        bool SetMove(Unit *obj, Position curPos, Position newPos, uint8 speed, bool bAbsoluteMove, uint t, bool bBroadcastMove);
        void EnumMovableObject(Position pos, uint8 layer, float range, std::vector<uint> &pvResult, bool bIncludeClient, bool bIncludeNPC);

        void addEXP(Unit *pCorpse, Player *pPlayer, float exp, float jp);
        void addEXP(Unit *pCorpse, int nPartyID, float exp, float jp);

        WorldSession *FindSession(uint32 id) const;
        void AddSession(WorldSession *s);
        bool RemoveSession(uint32 id);
        void UpdateSessions(uint diff);
        void KickAll();

        void AddObjectToWorld(WorldObject *obj);
        void AddSummonToWorld(Summon *pSummon);
        void AddMonsterToWorld(Monster *pMonster);
        void AddItemToWorld(Item *pItem);
        bool RemoveItemFromWorld(Item *pItem);
        void RemoveObjectFromWorld(WorldObject *obj);
        void MonsterDropItemToWorld(Unit *pUnit, Item *pItem);
        bool checkDrop(Unit *pKiller, int code, int percentage, float fDropRatePenalty, float fPCBangDropRateBonus);
        bool ProcTame(Monster *pMonster);
        void ClearTamer(Monster *pMonster, bool bBroadcastMsg);
        bool SetTamer(Monster *pMonster, Player *pPlayer, int nSkillLevel);

        // skills
        void AddSkillResult(std::vector<SkillResult> &pvList, bool bIsSuccess, int nSuccessType, uint handle);
        void AddSkillDamageResult(std::vector<SkillResult> &pvList, uint8 type, uint8 damageType, DamageInfo damageInfo, uint handle);

        // Quest?
        int ShowQuestMenu(Player *pPlayer);

        // Item
        uint procAddItem(Player *pClient, Item *pItem, bool bIsPartyProcess);
        void procPartyShare(Player *pClient, Item *pItem);
        void addChaos(Unit *pCorpse, Player *pPlayer, float chaos);

        // Warping
        void WarpBegin(Player *);
        void WarpEnd(Player *, Position, uint8_t);
        void WarpEndSummon(Player *, Position, uint8_t, Summon *, bool);

        void onRegionChange(WorldObject *obj, uint update_time, bool bIsStopMessage);

        /// Get the number of current active sessions
        const SessionMap &GetAllSessions() const { return m_sessions; }

        uint32 GetSessionCount() const { return m_sessions.size(); }

        void Broadcast(uint, uint, uint, uint, uint8, XPacket);
        void Broadcast(uint, uint, uint8, XPacket);
        uint GetArTime();

        void Update(uint);

        static uint8 GetExitCode() { return m_ExitCode; }

        static void StopNow(uint8 exitcode)
        {
            m_stopEvent = true;
            m_ExitCode  = exitcode;
        }

        static bool IsStopped() { return m_stopEvent; }

        /// Gets and increments the identifier for DB insert statements
        uint64 GetItemIndex();
        uint64 GetPlayerIndex();
        uint64 GetPetIndex();
        uint64 GetStateIndex();
        uint64 GetSummonIndex();
        uint64 GetSkillIndex();

        /// Set a server rate
        void setRate(Rates rate, float value) { rate_values[rate] = value; }

        /// Get a server rate
        float getRate(Rates rate) const { return rate_values[rate]; }

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
        void setIntConfig(WorldIntConfigs index, uint32 value)
        {
            if (index < INT_CONFIG_VALUE_COUNT)
                m_int_configs[index] = value;
        }

        /// Get a server configuration element
        uint32 getIntConfig(WorldIntConfigs index) const
        {
            return index < INT_CONFIG_VALUE_COUNT ? m_int_configs[index] : 0;
        }

        std::vector<RespawnObject *> m_vRespawnList{ };
    private:
        static std::atomic<bool> m_stopEvent;

        static uint8 m_ExitCode;

        SessionMap                 m_sessions;
        const uint                 startTime;

        void AddSession_(WorldSession *s);
        LockedQueue<WorldSession*> addSessQueue;

        void onMoveObject(WorldObject *pUnit, Position oldPos, Position newPos);
        bool onSetMove(WorldObject *pObject, Position curPos, Position lastpos);
        void enterProc(WorldObject *pUint, uint prx, uint pry);
        void step(WorldObject *obj, uint tm);

        AtomicIndex s_nPlayerIndex{0};
        AtomicIndex s_nPetIndex{0};
        AtomicIndex s_nSummonIndex{0};
        AtomicIndex s_nSkillIndex{0};
        AtomicIndex s_nStateIndex{0};
        AtomicIndex s_nItemIndex{0};

        float  rate_values[MAX_RATES];
        uint32 m_int_configs[INT_CONFIG_VALUE_COUNT];
        bool   m_bool_configs[BOOL_CONFIG_VALUE_COUNT];

        IntervalTimer m_timers[WUPDATE_COUNT];
    protected:
        World();
};

#define sWorld World::Instance()