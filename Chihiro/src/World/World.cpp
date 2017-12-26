#include "World.h"
#include "DatabaseEnv.h"
#include "Map/ArRegion.h"
#include "Globals/ObjectMgr.h"
#include "Timer.h"
#include "Scripting/XLua.h"
#include "Messages.h"

World::World() : startTime(getMSTime())
{
}


World::~World()
{
	CharacterDatabase.Close();
}

uint World::GetArTime()
{
	return GetMSTimeDiffToNow(startTime) / 10;
}

void World::InitWorld()
{
    MX_LOG_INFO("server.worldserver", "Initializing world...");

    uint32_t oldTime = getMSTime(), oldFullTime = getMSTime();
	MX_LOG_INFO("server.worldserver", "Initializing region system...");
	sArRegion->InitRegionSystem(sConfigMgr->GetIntDefault("Game.MapWidth", 700000), sConfigMgr->GetIntDefault("Game.MapHeight", 1000000));
	MX_LOG_INFO("server.worldserver", "Initialized region system in %u ms", GetMSTimeDiffToNow(oldTime));

    oldTime = getMSTime();
	MX_LOG_INFO("server.worldserver", "Initializing game content...");

    // Dörti häckz, plz ihgnoar
    s_nItemIndex = CharacterDatabase.Query("SELECT MAX(sid) FROM Item;").get()->Fetch()->GetUInt64();;
    s_nPlayerIndex = CharacterDatabase.Query("SELECT MAX(sid) FROM `Character`;").get()->Fetch()->GetUInt64();
	s_nSkillIndex = CharacterDatabase.Query("SELECT MAX(sid) FROM `Skill`;").get()->Fetch()->GetUInt64();
	s_nSummonIndex = CharacterDatabase.Query("SELECT MAX(sid) FROM `Summon`;").get()->Fetch()->GetUInt64();

	MX_LOG_INFO("server.worldserver", "Initializing scripting...");
	sScriptingMgr->InitializeLua();
	MX_LOG_INFO("server.worldserver", "Initialized scripting in %u ms", GetMSTimeDiffToNow(oldTime));

	oldTime = getMSTime();
	sObjectMgr->InitGameContent();
	MX_LOG_INFO("server.worldserver", "Initialized game content in %u ms", GetMSTimeDiffToNow(oldTime));

	MX_LOG_INFO("server.worldserver", "World fully initialized in %u ms!", GetMSTimeDiffToNow(oldFullTime));
}

/// Find a session by its id
GameSession* World::FindSession(uint32 id) const
{
	SessionMap::const_iterator itr = m_sessions.find(id);

	if (itr != m_sessions.end())
		return itr->second;                                 // also can return NULL for kicked session
	else
		return nullptr;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    if(m_sessions.count(id) == 1)
    {
        m_sessions.erase(id);
        return true;
    }
    return false;
}

void World::AddSession(GameSession* s)
{
	if(s == nullptr)
		return;
    if(m_sessions.count(s->GetAccountId()) == 0) {
        m_sessions.insert({ (uint32)s->GetAccountId(), s });
    }
}

uint64_t World::getItemIndex()
{
    return ++s_nItemIndex;
}

uint64_t World::getPlayerIndex()
{
    return ++s_nPlayerIndex;
}

uint64_t World::getPetIndex()
{
    return ++s_nPetIndex;
}

uint64_t World::getSummonIndex()
{
    return ++s_nSummonIndex;
}

uint64_t World::getSkillIndex()
{
	return ++s_nSkillIndex;
}

bool World::SetMultipleMove(Unit *pUnit, Position curPos, std::vector<Position> newPos, uint8_t speed, bool bAbsoluteMove, uint t, bool bBroadcastMove)
{
    Position oldPos{};
    Position lastpos = newPos.back();

	bool result = false;
	if(bAbsoluteMove || true/* onSetMove Quadtreepotato*/) {
		oldPos.m_positionX = pUnit->GetPositionX();
		oldPos.m_positionY = pUnit->GetPositionY();
		oldPos.m_positionZ = pUnit->GetPositionZ();
		oldPos._orientation = pUnit->GetOrientation();

		pUnit->SetCurrentXY(curPos.GetPositionX(), curPos.GetPositionY());
		curPos.m_positionX = pUnit->GetPositionX();
        curPos.m_positionY = pUnit->GetPositionY();
        curPos.m_positionZ = pUnit->GetPositionZ();
        curPos.SetOrientation(pUnit->GetOrientation());

		onMoveObject(pUnit, oldPos, curPos);
		enterProc(pUnit, (uint)(oldPos.GetPositionX() / g_nRegionSize), (uint)(oldPos.GetPositionY() / g_nRegionSize));
		pUnit->SetMultipleMove(newPos, speed, t);

		if(bBroadcastMove) {
			sArRegion->DoEachVisibleRegion((uint) (pUnit->GetPositionX() / g_nRegionSize), (uint) (pUnit->GetPositionY() / g_nRegionSize), pUnit->GetLayer(),
										   [=](ArRegion *region) {
											   region->DoEachClient([=](WorldObject *obj) {
												   Messages::SendMoveMessage(dynamic_cast<Player *>(obj), pUnit);
											   });
										   });
		}
        result = true;
	}
    return result;
}

void World::onMoveObject(WorldObject *pUnit, Position oldPos, Position newPos)
{
	auto prev_rx = (uint)(oldPos.m_positionX / g_nRegionSize);
	auto prev_ry = (uint)(oldPos.m_positionY / g_nRegionSize);
	if(prev_rx != (uint)(newPos.GetPositionX() / g_nRegionSize) || prev_ry != (uint)(newPos.GetPositionY() / g_nRegionSize)) {
		sArRegion->GetRegion(prev_rx, prev_ry, (uint32)pUnit->GetLayer())->RemoveObject(pUnit);
		sArRegion->GetRegion(*pUnit)->AddObject(pUnit);
	}
}

void World::enterProc(WorldObject *pUnit, uint prx, uint pry)
{
	auto rx = (uint)(pUnit->GetPositionX() / g_nRegionSize);
	auto ry = (uint)(pUnit->GetPositionY() / g_nRegionSize);
	if(rx != prx || ry != pry) {
		sArRegion->DoEachNewRegion(rx, ry, prx, pry, pUnit->GetLayer(), [=](ArRegion* rgn) {
			rgn->DoEachClient([=](Unit* client) { // enterProc
				// BEGIN Send Enter Message to each other
				if(client->GetHandle() != pUnit->GetHandle()) {
					Messages::sendEnterMessage(dynamic_cast<Player*>(pUnit), client, false);
					if(client->GetSubType() == ST_Player) {
						Messages::sendEnterMessage(dynamic_cast<Player *>(client), pUnit, false);
					}
				}
			});	// END Send Enter Message to each other
			auto func = [=](WorldObject* obj) {
				Messages::sendEnterMessage(dynamic_cast<Player*>(pUnit), obj, false);
			};
			rgn->DoEachMovableObject(func);
			rgn->DoEachStaticObject(func);
		});
	}
}

void World::AddObjectToWorld(WorldObject *obj)
{
    auto region = sArRegion->GetRegion(*obj);
    if (region == nullptr)
        return;

    sArRegion->DoEachVisibleRegion((uint) (obj->GetPositionX() / g_nRegionSize), (uint) (obj->GetPositionY() / g_nRegionSize), obj->GetLayer(),
                                   [=](ArRegion *rgn) {
                                       rgn->DoEachClient([=](Unit *client) { // enterProc
                                           // BEGIN Send Enter Message to each other
                                           if (client->GetHandle() != obj->GetHandle()) {
                                               // Enter message FROM doEachRegion-Client TO obj
                                               if (client->IsInWorld() && obj->GetSubType() == ST_Player)
                                                   Messages::sendEnterMessage(dynamic_cast<Player *>(obj), client, false);
                                               // Enter message FROM obj TO doEachRegion-Client
                                               if (client->GetSubType() == ST_Player)
                                                   Messages::sendEnterMessage(dynamic_cast<Player *>(client), dynamic_cast<Unit *>(obj), false);
                                           }
                                       });    // END Send Enter Message to each other
                                       rgn->DoEachMovableObject([=](WorldObject *lbObj) {
                                           if (lbObj->IsInWorld() && obj->GetSubType() == ST_Player)
                                               Messages::sendEnterMessage(dynamic_cast<Player *>(obj), dynamic_cast<Unit *>(lbObj), false);
                                       });
                                       rgn->DoEachStaticObject([=](WorldObject *lbObj) {
                                           if (lbObj->IsInWorld() && obj->GetSubType() == ST_Player)
                                               Messages::sendEnterMessage(dynamic_cast<Player *>(obj), dynamic_cast<Unit *>(lbObj), false);
                                       });
                                   });
    region->AddObject(obj);
}

void World::onRegionChange(WorldObject *obj, uint update_time, bool bIsStopMessage)
{
    auto oldx = (uint)(obj->GetPositionX() / g_nRegionSize);
    auto oldy = (uint)(obj->GetPositionY() / g_nRegionSize);
    step(obj, (uint)(update_time + obj->lastStepTime + (bIsStopMessage ? 0xA : 0)));

    if((uint)(obj->GetPositionX() / g_nRegionSize) != oldx || (uint)(obj->GetPositionY() / g_nRegionSize) != oldy) {
        enterProc(obj, oldx, oldy);
        obj->prevX = oldx;
        obj->prevY = oldy;
    }
}

void World::RemoveObjectFromWorld(WorldObject *obj)
{
    auto region = sArRegion->GetRegion(*obj);
	if(region != nullptr)
		region->RemoveObject(obj);
}

void World::step(WorldObject *obj, uint tm)
{
    Position oldPos = obj->GetPosition();
    obj->Step(tm);
    Position newPos = obj->GetPosition();

    onMoveObject(obj, oldPos, newPos);
    obj->lastStepTime = tm;
}

bool World::onSetMove(WorldObject *pObject, Position curPos, Position lastpos)
{
    return true;
}

void World::Update(uint diff)
{
    for (auto& session : m_sessions) {
        if (session.second != nullptr && session.second->GetPlayer() != nullptr) {
            session.second->GetPlayer()->Update(diff);
        } else {
            m_sessions.erase(session.first);
        }
    }
}

void World::Broadcast(uint rx1, uint ry1, uint rx2, uint ry2, uint8 layer, XPacket packet)
{
    sArRegion->DoEachVisibleRegion(rx1, ry1, rx2, ry2, layer, [&packet](ArRegion* rgn) {
        rgn->DoEachClient([&packet](WorldObject* obj) {
            dynamic_cast<Player*>(obj)->SendPacket(packet);
        });
    });
}

void World::Broadcast(uint rx, uint ry, uint8 layer, XPacket packet)
{
    sArRegion->DoEachVisibleRegion(rx, ry, layer, [&packet](ArRegion* rgn) {
       rgn->DoEachClient([&packet](WorldObject* obj) {
           dynamic_cast<Player*>(obj)->SendPacket(packet);
       });
    });
}

void World::AddSummonToWorld(Summon *pSummon)
{
    pSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
    //pSummon->AddToWorld();
    AddObjectToWorld(pSummon);
    //pSummon->m_bIsSummoned = true;
    pSummon->RemoveFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
}

void World::WarpBegin(Player * pPlayer)
{
	if(pPlayer->IsInWorld())
		RemoveObjectFromWorld(pPlayer);
	if(pPlayer->m_pMainSummon != nullptr)
		RemoveObjectFromWorld(pPlayer->m_pMainSummon);
	// Same for sub summon
	// same for pet
}

void World::WarpEnd(Player *pPlayer, Position pPosition, uint8_t layer)
{
	if(pPlayer == nullptr)
		return;

	uint ct = GetArTime();

	if(layer != pPlayer->GetLayer()) {
		// TODO Layer management
	}
	pPlayer->SetCurrentXY(pPosition.GetPositionX(), pPosition.GetPositionY());
	pPlayer->StopMove();

	Messages::SendWarpMessage(pPlayer);
	if(pPlayer->m_pMainSummon != nullptr)
		WarpEndSummon(pPlayer, pPosition, layer, pPlayer->m_pMainSummon, 0);

	((Unit*)pPlayer)->SetFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
	AddObjectToWorld(pPlayer);
	pPlayer->RemoveFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
	Position pos = pPlayer->GetCurrentPosition(ct);
	// Set Move
	Messages::SendPropertyMessage(pPlayer,pPlayer, "channel", 0);
	pPlayer->ChangeLocation(pPlayer->GetPositionX(), pPlayer->GetPositionY(), false, true);
	pPlayer->Save(true);
}

void World::WarpEndSummon(Player *pPlayer , Position pos, uint8_t layer, Summon *pSummon, bool)
{
	uint ct = GetArTime();
	if(pSummon == nullptr)
		return;
	pSummon->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
	pSummon->SetLayer(layer);
	pSummon->StopMove();
	pSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
	pSummon->AddNoise(rand32(), rand32(), 35);
	AddObjectToWorld(pSummon);
	pSummon->RemoveFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter);
	auto position = pSummon->GetCurrentPosition(ct);
	// Set move
}
