#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"
#include "Utilities/SignalHandler.h"
#include "Network/GameNetwork/GameAcceptor.h"
#include "Network/AuthSession.h"
#include "World.h"
#include "WorldRunnable.h"

#include <ace/Dev_Poll_Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/ACE.h>
#include <ace/Sig_Handler.h>
#include <ace/Event_Handler.h>

#ifndef _CHIHIRO_CORE_CONFIG
# define _CHIHIRO_CORE_CONFIG  "chihiro.conf"
#endif //_CHIHIRO_CORE_CONFIG

CharacterDatabaseWorkerPool CharacterDatabase;	                      // Accessor to the character server database
GameDatabaseWorkerPool GameDatabase;								  // Accessor to the game server database


bool StartDB();
void StopDB();
bool stopEvent = false;
///- Handle worldserver's termination signals
/// Handle worldservers's termination signals
class WorldServerSignalHandler : public Skyfire::SignalHandler
{
public:
	void HandleSignal(int sigNum) override
	{
		switch (sigNum)
		{
			case SIGINT:
			case SIGTERM:
#ifdef _WIN32
				case SIGBREAK:
                    if (m_ServiceStatus != 1)
#endif
                stopEvent = true;
				break;
			default:
				break;
		}
	}
};


int main(int argc, char **argv)
{
    if (!sConfigMgr->LoadInitial(_CHIHIRO_CORE_CONFIG))
	{
		MX_LOG_ERROR("server.worldserver", "Invalid or missing configuration file : %s", _CHIHIRO_CORE_CONFIG);
		MX_LOG_ERROR("server.worldserver","Verify that the file exists and has \'[chihiro]' written in the top of the file!");
		return 1;
	}

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
	ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(ACE::max_handles(), 1), 1), true);
#else
	ACE_Reactor::instance(new ACE_Reactor(new ACE_TP_Reactor(), true), true);
#endif

	///- Initialize the signal handlers
	WorldServerSignalHandler SignalINT, SignalTERM;
#ifdef _WIN32
	WorldServerSignalHandler SignalBREAK;
#endif /* _WIN32 */

	///- Register worldserver's signal handlers
	ACE_Sig_Handler Handler;
	Handler.register_handler(SIGINT, &SignalINT);
	Handler.register_handler(SIGTERM, &SignalTERM);
#ifdef _WIN32
	Handler.register_handler(SIGBREAK, &SignalBREAK);
#endif /* _WIN32 */

	if (!StartDB())
	{
		MX_LOG_ERROR("server.worldserver","Cannot connect to database.");
		return 1;
	}

	sWorld->InitWorld();

	///-Launch WorldRunnable thread
	ACE_Based::Thread worldThread(new WorldRunnable);
	worldThread.setPriority(ACE_Based::Highest);


	ACE_INET_Addr auth_addr(sConfigMgr->GetIntDefault("AuthServer.Port", 4502), sConfigMgr->GetStringDefault("AuthServer.IP", "127.0.0.1").c_str());
	if (sAuthNetwork->InitializeNetwork(auth_addr) != 0)
	{
		MX_LOG_ERROR("server.worldserver","Cannot connect to the auth server!");
		return 1;
	}

	ACE_INET_Addr game_addr(sConfigMgr->GetIntDefault("GameServer.Port", 4514), sConfigMgr->GetStringDefault("GameServer.IP", "0.0.0.0").c_str());
	GameAcceptor acceptor;
	if (acceptor.open(game_addr, ACE_Reactor::instance(), ACE_NONBLOCK) == -1) {
		printf("Error creating acceptor at %s:%d\n", game_addr.get_host_addr(), game_addr.get_port_number());
		return 1;
	}
	// maximum counter for next ping
	uint32 numLoops = 30 * (MINUTE * 1000000 / 100000);
	uint32 loopCounter = 0;

	// Wait for termination signal
	while (!stopEvent)
	{
		// dont move this outside the loop, the reactor will modify it
		ACE_Time_Value interval(0, 100000);

		if (ACE_Reactor::instance()->run_reactor_event_loop(interval) == -1)
			break;

		if ((++loopCounter) == numLoops)
		{
			loopCounter = 0;

			MX_LOG_INFO("misc", "Ping MySQL to keep connection alive");
			CharacterDatabase.KeepAlive();
		}
	}

    StopDB();

	return 0;
}

///- Initialize connection to the databases
bool StartDB()
{
	MySQL::Library_Init();

	std::string dbstring;
	uint8 async_threads, synch_threads;

	dbstring = sConfigMgr->GetStringDefault("CharacterDB.CString", "");
	if (dbstring.empty())
	{
		MX_LOG_ERROR("server.worldserver","Character database not specified in configuration file");
		return false;
	}

	async_threads = sConfigMgr->GetIntDefault("CharacterDB.WorkerThreads", 1);
	if (async_threads < 1 || async_threads > 32)
	{
		MX_LOG_ERROR("server.worldserver","Character database: invalid number of worker threads specified. "
			"Please pick a value between 1 and 32.");
		return false;
	}

	synch_threads = sConfigMgr->GetIntDefault("CharacterDB.SynchThreads", 1);

	///- Initialize the world database
	if (!CharacterDatabase.Open(dbstring, 1, synch_threads))
	{
		sLog->outError("Cannot connect to character database %s", dbstring.c_str());
		return false;
	}

	///- Get game database info from configuration file
	dbstring = sConfigMgr->GetStringDefault("GameDB.CString", "");
	if (dbstring.empty())
	{
		MX_LOG_ERROR("server.worldserver","Arcadia database not specified in configuration file");
		return false;
	}

	///- Initialize the game database, no need to set async/sync threads
	if (!GameDatabase.Open(dbstring, 1, 1))
	{
		sLog->outError("Cannot connect to Arcadia database %s", dbstring.c_str());
		return false;
	}
	return true;
}

void StopDB()
{
    sAuthNetwork->close();
    GameDatabase.Close();
    CharacterDatabase.Close();
    MySQL::Library_End();
}