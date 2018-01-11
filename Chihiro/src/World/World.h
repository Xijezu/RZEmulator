#ifndef __WORLD_H
#define __WORLD_H

#include "Common.h"
#include "Dynamic/UnorderedMap.h"
#include "RespawnObject.h"

enum ShutdownExitCode
{
	SHUTDOWN_EXIT_CODE = 0,
	ERROR_EXIT_CODE    = 1,
	RESTART_EXIT_CODE  = 2
};

class WorldSession;
class Unit;
class Player;
class XPacket;

typedef UNORDERED_MAP<uint32, WorldSession*> SessionMap;
const int g_nRegionSize = 180;
const int g_nMapWidth = 700000;
const int g_nMapHeight = 1000000;

class World
{
public:
    static ACE_Atomic_Op<ACE_Thread_Mutex, uint32> m_worldLoopCounter;

	World();
	~World();

	void InitWorld();

	bool SetMultipleMove(Unit* pUnit, Position curPos, std::vector<Position>newPos, uint8_t speed, bool bAbsoluteMove, uint t, bool bBroadcastMove);
	bool SetMove(Unit* obj, Position curPos, Position newPos, uint8 speed, bool bAbsoluteMove, uint t, bool bBroadcastMove);

	void addEXP(Unit* pCorpse, Player* pPlayer, float exp, float jp);

	WorldSession* FindSession(uint32 id) const;
	void AddSession(WorldSession* s);
	bool RemoveSession(uint32 id);
	void KickAll();

	void AddObjectToWorld(WorldObject* obj);
	void AddSummonToWorld(Summon* pSummon);
	void AddMonsterToWorld(Monster* pMonster);
    void AddItemToWorld(Item* pItem);
    bool RemoveItemFromWorld(Item* pItem);
    void RemoveObjectFromWorld(WorldObject* obj);
    void MonsterDropItemToWorld(Unit* pUnit, Item* pItem);
	bool checkDrop(Unit* pKiller, int code, int percentage, float fDropRatePenalty, float fPCBangDropRateBonus);
	bool ProcTame(Monster* pMonster);
	void ClearTamer(Monster* pMonster, bool bBroadcastMsg);
	bool SetTamer(Monster* pMonster, Player* pPlayer, int nSkillLevel);

	// skills
	void AddSkillDamageResult(std::vector<SkillResult>& pvList, bool bIsSuccess, int nSuccessType, uint handle);
	void AddSkillDamageResult(std::vector<SkillResult>& pvList, uint8 type, uint8 damageType, DamageInfo damageInfo, uint handle);

	// Quest?
	int ShowQuestMenu(Player* pPlayer);

    // Item
    uint procAddItem(Player* pClient, Item* pItem, bool bIsPartyProcess);
	void addChaos(Unit* pCorpse, Player* pPlayer, float chaos);

	// Warping
	void WarpBegin(Player*);
	void WarpEnd(Player*, Position, uint8_t);
	void WarpEndSummon(Player*, Position, uint8_t, Summon*, bool);

    void onRegionChange(WorldObject *obj, uint update_time, bool bIsStopMessage);
	/// Get the number of current active sessions
	const SessionMap& GetAllSessions() const { return m_sessions; }
	uint32 GetSessionCount() const { return m_sessions.size(); }

    void Broadcast(uint, uint, uint, uint, uint8, XPacket);
    void Broadcast(uint, uint, uint8, XPacket);
	uint GetArTime();

	void Update(uint);
    static uint8 GetExitCode() { return m_ExitCode; }
    static void StopNow(uint8 exitcode) { m_stopEvent = true; m_ExitCode = exitcode; }
	static bool IsStopped() { return m_stopEvent.value(); }

    /// Gets and increments the identifier for DB insert statements
    uint64 GetItemIndex();
    uint64 GetPlayerIndex();
    uint64 GetPetIndex();
    uint64 GetSummonIndex();
	uint64 GetSkillIndex();

	std::vector<RespawnObject*> m_vRespawnList{};
private:
	static ACE_Atomic_Op<ACE_Thread_Mutex, bool> m_stopEvent;
	static uint8 m_ExitCode;

	SessionMap m_sessions;
	uint64_t s_nItemIndex{0};
	const uint startTime;

	void AddSession_(WorldSession* s);
	ACE_Based::LockedQueue<WorldSession*, ACE_Thread_Mutex> addSessQueue{};

	void onMoveObject(WorldObject* pUnit, Position oldPos, Position newPos);
    bool onSetMove(WorldObject* pObject, Position curPos, Position lastpos);
	void enterProc(WorldObject* pUint, uint prx, uint pry);
    void step(WorldObject *obj, uint tm);
	uint64_t s_nPlayerIndex{0};
	uint64_t s_nPetIndex{0};
	uint64_t s_nSummonIndex{0};
	uint64_t s_nSkillIndex{0};
};

#define sWorld ACE_Singleton<World, ACE_Null_Mutex>::instance()
#endif // __WORLD_H