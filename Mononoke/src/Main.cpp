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
#include "WorldSocketMgr.h"
#include "SignalHandler.h"
#include "AuthGameSession.h"
#include "AuthClientSession.h"
#include "SystemConfigs.h"

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
#include <ace/Dev_Poll_Reactor.h>
#endif
#include <ace/TP_Reactor.h>

bool StartDB();
void StopDB();

bool stopEvent = false;                                     // Setting it to true stops the server
LoginDatabaseWorkerPool LoginDatabase;                      // Accessor to the auth server database


#ifndef _MONONOKE_CORE_CONFIG
# define _MONONOKE_CORE_CONFIG  "authserver.conf"
#endif //_MONONOKE_CORE_CONFIG

class AuthServerSignalHandler : public Skyfire::SignalHandler
{
public:
	virtual void HandleSignal(int SigNum)
	{
		switch (SigNum)
		{
			case SIGINT:
            case SIGTERM:
                stopEvent = true;
				break;
            default:
                break;
		}
	}
};



extern int main(int argc, char **argv)
{
	//sLog->Initialize();

	if (!sConfigMgr->LoadInitial(_MONONOKE_CORE_CONFIG))
    {
        NG_LOG_ERROR("server.authserver", "Invalid or missing configuration file : %s", _MONONOKE_CORE_CONFIG);
        NG_LOG_ERROR("server.authserver", "Verify that the file exists and has \'[authserver]' written in the top of the file!");
        return 1;
    }
	NG_LOG_INFO("server.authserver", "%s (authserver)", _FULLVERSION);

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
	ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(ACE::max_handles(), 1), 1), true);
#else
	ACE_Reactor::instance(new ACE_Reactor(new ACE_TP_Reactor(), true), true);
#endif

	///- Initialize the signal handlers
	AuthServerSignalHandler SignalINT, SignalTERM;
#ifdef _WIN32
	AuthServerSignalHandler SignalBREAK;
#endif /* _WIN32 */

	///- Register worldserver's signal handlers
	ACE_Sig_Handler Handler;
	Handler.register_handler(SIGINT, &SignalINT);
	Handler.register_handler(SIGTERM, &SignalTERM);


	auto authPort = (uint16)sConfigMgr->GetIntDefault("Authserver.Port", 4500);
	std::string authBindIp = sConfigMgr->GetStringDefault("Authserver.IP", "0.0.0.0");
	if (ACE_Singleton<WorldSocketMgr<AuthClientSession>, ACE_Thread_Mutex>::instance()->StartNetwork(authPort, authBindIp.c_str()) == -1)
	{
		printf("Error creating acceptor at %s:%d\n", authBindIp.c_str(), authPort);
		return 1;
	}

    auto gamePort = (uint16)sConfigMgr->GetIntDefault("Gameserver.Port", 4502);
    std::string bindIp = sConfigMgr->GetStringDefault("Gameserver.IP", "0.0.0.0");
    if (ACE_Singleton<WorldSocketMgr<AuthGameSession>, ACE_Thread_Mutex>::instance()->StartNetwork(gamePort, bindIp.c_str()) == -1)
    {
        printf("Error creating acceptor at %s:%d\n", bindIp.c_str(), gamePort);
        return 1;
    }

	// Initialize the database connection
	if (!StartDB())
		return 1;

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
			NG_LOG_INFO("server.authserver", "Ping MySQL to keep connection alive");
			LoginDatabase.KeepAlive();
		}
	}

	// Close the Database Pool and library
	StopDB();

	return 0;
}

// Initialize connection to the database
bool StartDB()
{
	MySQL::Library_Init();

	std::string dbstring = sConfigMgr->GetStringDefault("AuthDatabase.CString", "");
	if (dbstring.empty())
	{
		NG_LOG_ERROR("server.authserver","Database not specified");
		return false;
	}

	auto worker_threads = (uint8)sConfigMgr->GetIntDefault("AuthDatabase.WorkerThreads", 1);
	if (worker_threads < 1 || worker_threads > 32)
	{
		NG_LOG_ERROR("server.authserver","Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
		worker_threads = 1;
	}

	auto synch_threads = (uint8)sConfigMgr->GetIntDefault("AuthDatabase.SynchThreads", 1);;
	if (synch_threads < 1 || synch_threads > 32)
	{
		NG_LOG_ERROR("server.authserver","Improper value specified for LoginDatabase.SynchThreads, defaulting to 1.");
		synch_threads = 1;
	}

	// NOTE: While authserver is singlethreaded you should keep synch_threads == 1. Increasing it is just silly since only 1 will be used ever.
	if (!LoginDatabase.Open(dbstring, worker_threads, synch_threads))
	{
		NG_LOG_ERROR("server.authserver","Cannot connect to database");
		return false;
	}

	return true;
}

void StopDB()
{
	LoginDatabase.Close();
	MySQL::Library_End();
}