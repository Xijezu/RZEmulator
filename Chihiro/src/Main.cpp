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
#include "Database/DatabaseEnv.h"
#include "Utilities/SignalHandler.h"
#include "AuthNetwork.h"
#include "World.h"
#include "SystemConfigs.h"
#include "WorldRunnable.h"

#include <fstream>

#ifndef _CHIHIRO_CORE_CONFIG
# define _CHIHIRO_CORE_CONFIG  "chihiro.conf"
#endif //_CHIHIRO_CORE_CONFIG

CharacterDatabaseWorkerPool CharacterDatabase;	                      // Accessor to the character server database
GameDatabaseWorkerPool GameDatabase;								  // Accessor to the game server database


bool StartDB();
void StopDB();

/// Handle worldservers's termination signals
class WorldServerSignalHandler : public Skyfire::SignalHandler
{
public:
	void HandleSignal(int sigNum) override
	{
		NG_LOG_INFO("server.worldserver", "Received signal: %d", sigNum);
		switch (sigNum)
		{
			case SIGINT:
			case SIGTERM:
#ifdef _WIN32
				case SIGBREAK:
#endif
				World::StopNow(SHUTDOWN_EXIT_CODE);
				break;
			case SIGSEGV:
			{
				/* Dont mind this line of code here
				 * This small thing gets us a stacktrace
				 * no matter what happened as long as it
				 * was a segmentation fault
				 */
				ACE_Stack_Trace st;
				NG_LOG_FATAL("server.worldserver", "%s", st.c_str());
				exit(sigNum);
			}
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
		NG_LOG_ERROR("server.worldserver", "Invalid or missing configuration file : %s", _CHIHIRO_CORE_CONFIG);
		NG_LOG_ERROR("server.worldserver", "Verify that the file exists and has \'[chihiro]' written in the top of the file!");
		return 1;
	}

    NG_LOG_INFO("server.worldserver", "%s (worldserver)", _FULLVERSION);

    NG_LOG_INFO("server.worldserver", "       _   _  _____                _ _");
    NG_LOG_INFO("server.worldserver", "      | \\ | |/ ____|              (_) |");
    NG_LOG_INFO("server.worldserver", "      |  \\| | |  __  ___ _ __ ___  _| |_ _   _");
    NG_LOG_INFO("server.worldserver", "      | . ` | | |_ |/ _ \\ '_ ` _ \\| | __| | | |");
    NG_LOG_INFO("server.worldserver", "      | |\\  | |__| |  __/ | | | | | | |_| |_| |");
    NG_LOG_INFO("server.worldserver", "      |_| \\_|\\_____|\\___|_| |_| |_|_|\\__|\\__, |");
    NG_LOG_INFO("server.worldserver", "                                          __/ |");
    NG_LOG_INFO("server.worldserver", "                                         |___/");
    NG_LOG_INFO("server.worldserver", "           NGemity (c) 2018 - For Rappelz");
    NG_LOG_INFO("server.worldserver", "               <https://ngemity.org/>");


#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
	ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(ACE::max_handles(), true), true), true);
#else
	ACE_Reactor::instance(new ACE_Reactor(new ACE_TP_Reactor(), true), true);
#endif

	///- Initialize the signal handlers
	WorldServerSignalHandler SignalINT, SignalTERM, SignalSEGV;
#ifdef _WIN32
	WorldServerSignalHandler SignalBREAK;
#endif /* _WIN32 */

	///- Register worldserver's signal handlers
	ACE_Sig_Handler Handler;
	Handler.register_handler(SIGINT, &SignalINT);
	Handler.register_handler(SIGTERM, &SignalTERM);
    Handler.register_handler(SIGSEGV, &SignalSEGV);
#ifdef _WIN32
	Handler.register_handler(SIGBREAK, &SignalBREAK);
#endif /* _WIN32 */

	if (!StartDB())
	{
		NG_LOG_ERROR("server.worldserver","Cannot connect to database.");
		return 1;
	}

    CharacterDatabase.PExecute("UPDATE `Character` SET logout_time = NOW() WHERE login_time > logout_time;");

	sWorld->InitWorld();

	ACE_Singleton<WorldSocketMgr<GameAuthSession>, ACE_Thread_Mutex>::instance()->StartNetwork(0, "0.0.0.0");
	ACE_INET_Addr auth_addr((uint16_t)sConfigMgr->GetIntDefault("AuthServer.Port", 4502), sConfigMgr->GetStringDefault("AuthServer.IP", "127.0.0.1").c_str());
	if (sAuthNetwork->InitializeNetwork(auth_addr) != 0)
	{
		NG_LOG_ERROR("server.worldserver","Cannot connect to the auth server!");
		return 1;
	}

	auto worldPort = (uint16)sConfigMgr->GetIntDefault("GameServer.Port", 4514);
	std::string bindIp = sConfigMgr->GetStringDefault("GameServer.IP", "0.0.0.0");
	if (sWorldSocketMgr->StartNetwork(worldPort, bindIp.c_str()) == -1)
	{
		NG_LOG_ERROR("server.worldserver", "Failed to start network");
		return -1;
		// go down and shutdown the server
	}

    ///-Launch WorldRunnable thread
    ACE_Based::Thread worldThread(new WorldRunnable);
    worldThread.setPriority(ACE_Based::Highest);


    worldThread.wait();
    StopDB();

	int exitCode = World::GetExitCode();
	NG_LOG_INFO("server.worldserver", "Exiting with code %d", exitCode);

	return exitCode;
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
		NG_LOG_ERROR("server.worldserver","Character database not specified in configuration file");
		return false;
	}

	async_threads = (uint8)sConfigMgr->GetIntDefault("CharacterDB.WorkerThreads", 2);
	if (async_threads < 1 || async_threads > 32)
	{
		NG_LOG_ERROR("server.worldserver","Character database: invalid number of worker threads specified. "
			"Please pick a value between 1 and 32.");
		return false;
	}

	synch_threads = (uint8)sConfigMgr->GetIntDefault("CharacterDB.SynchThreads", 2);
	///- Initialize the world database
	if (!CharacterDatabase.Open(dbstring, 1, synch_threads))
	{
		NG_LOG_ERROR("server.worldserver", "Cannot connect to character database %s", dbstring.c_str());
		return false;
	}

	///- Get game database info from configuration file
	dbstring = sConfigMgr->GetStringDefault("GameDB.CString", "");
	if (dbstring.empty())
	{
		NG_LOG_ERROR("server.worldserver","Arcadia database not specified in configuration file");
		return false;
	}

	///- Initialize the game database, no need to set async/sync threads
	if (!GameDatabase.Open(dbstring, 1, 1))
	{
		NG_LOG_ERROR("server.worldserver", "Cannot connect to Arcadia database %s", dbstring.c_str());
		return false;
	}
	return true;
}

void StopDB()
{
    GameDatabase.Close();
    CharacterDatabase.Close();
    MySQL::Library_End();
}