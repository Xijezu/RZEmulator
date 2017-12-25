#ifndef __WORLD_H
#define __WORLD_H

#include "Common.h"
#include "Network/GameNetwork/GameHandler.h"
#include "Entities/Player/Player.h"
#include "Unit.h"
#include "Dynamic/UnorderedMap.h"

typedef UNORDERED_MAP<uint32, GameSession*> SessionMap;
const int g_nRegionSize = 180;

class World
{
public:
	World();
	~World();

	void InitWorld();

	bool SetMultipleMove(Unit* pUnit, Position curPos, std::vector<Position>newPos, uint8_t speed, bool bAbsoluteMove, uint t, bool bBroadcastMove);

	GameSession* FindSession(uint32 id) const;
	void AddSession(GameSession* s);
	bool RemoveSession(uint32 id);

	void AddObjectToWorld(WorldObject* obj);
	void AddSummonToWorld(Summon* pSummon);
    void RemoveObjectFromWorld(WorldObject* obj);

	// Warping
	void WarpBegin(Player*);
	void WarpEnd(Player*, Position, uint8_t);
	void WarpEndSummon(Player*, Position, uint8_t, Summon*, bool);

    void onRegionChange(WorldObject *obj, uint update_time, bool bIsStopMessage);
	/// Get the number of current active sessions
	const SessionMap& GetAllSessions() const { return m_sessions; }
	uint32 GetSessionCount() const { return m_sessions.size(); }

	void SendResult(Player&, uint16_t, uint16_t, uint16_t);
    void Broadcast(uint, uint, uint, uint, uint8, XPacket);
    void Broadcast(uint, uint, uint8, XPacket);
	uint GetArTime();

	void Update(uint);
	static bool IsStopped() { return false; }

    uint64_t getItemIndex();
    uint64_t getPlayerIndex();
    uint64_t getPetIndex();
    uint64_t getSummonIndex();
	uint64_t getSkillIndex();
private:
	SessionMap m_sessions;
	uint64_t s_nItemIndex{0};
	const uint startTime;
private:
	void onMoveObject(WorldObject* pUnit, Position oldPos, Position newPos);
    bool onSetMove(WorldObject* pObject, Position curPos, Position lastpos);
	void enterProc(WorldObject* pUint, uint prx, uint pry);
    void step(WorldObject *obj, uint tm);
	uint64_t s_nPlayerIndex{0};
	uint64_t s_nPetIndex{0};
	uint64_t s_nSummonIndex{0};
	uint64_t s_nSkillIndex{0};
};

#define sWorld ACE_Singleton<World, ACE_Thread_Mutex>::instance()
#endif // __WORLD_H