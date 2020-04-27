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

#include <boost/asio/signal_set.hpp>

#include "AuthNetwork.h"
#include "CliThread.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "Maploader.h"
#include "MemPool.h"
#include "MySQLThreading.h"
#include "ObjectMgr.h"
#include "Stacktrace.h"
#include "SystemConfigs.h"
#include "WorldSession.h"
#include "XSocketMgr.h"

#ifndef _CHIHIRO_CORE_CONFIG
#define _CHIHIRO_CORE_CONFIG "chihiro.conf"
#endif //_CHIHIRO_CORE_CONFIG

bool StartDB();
void StopDB();
void WorldUpdateLoop();
void ShutdownCLIThread(std::thread *cliThread);
void SignalHandler(boost::system::error_code const &error, int32_t signalNumber);
void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32_t dbPingInterval, boost::system::error_code const &error);

constexpr int32_t WORLD_SLEEP_CONST = 50;

int32_t main(int32_t argc, char **argv)
{
    Stacktrace::enableStacktracing();

    std::string configError;
    if (!sConfigMgr->LoadInitial(_CHIHIRO_CORE_CONFIG, std::vector<std::string>(argv, argv + argc), configError))
    {
        printf("Error in config file or file not found: %s\n", configError.c_str());
        return 1;
    }

    std::shared_ptr<NGemity::Asio::IoContext> ioContext = std::make_shared<NGemity::Asio::IoContext>();
    // If logs are supposed to be handled async then we need to pass the IoContext into the Log singleton
    sLog->Initialize(sConfigMgr->GetBoolDefault("Log.Async.Enable", false) ? ioContext.get() : nullptr);

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

    if (!StartDB())
    {
        NG_LOG_ERROR("server.worldserver", "Cannot connect to database.");
        return 1;
    }
    std::shared_ptr<void> sDBHandle(nullptr, [](void *) { StopDB(); });
    // Set signal handlers
    boost::asio::signal_set signals(*ioContext, SIGINT, SIGTERM);
#if PLATFORM == PLATFORM_WINDOWS
    signals.add(SIGBREAK);
#endif
    signals.async_wait(SignalHandler);

    // Enabled a timed callback for handling the database keep alive ping
    int32_t dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);
    std::shared_ptr<boost::asio::deadline_timer> dbPingTimer = std::make_shared<boost::asio::deadline_timer>(*ioContext);
    dbPingTimer->expires_from_now(boost::posix_time::minutes(dbPingInterval));
    dbPingTimer->async_wait(std::bind(&KeepDatabaseAliveHandler, std::weak_ptr<boost::asio::deadline_timer>(dbPingTimer), dbPingInterval, std::placeholders::_1));

    sWorld.InitWorld();
    if (!sAuthNetwork.InitializeNetwork(*ioContext, sConfigMgr->GetStringDefault("AuthServer.IP", "127.0.0.1"), sConfigMgr->GetIntDefault("AuthServer.Port", 4502)))
    {
        NG_LOG_ERROR("server.worldserver", "Cannot connect to the auth server!");
        return 1;
    }

    std::shared_ptr<void> sAuthHandle(nullptr, [](void *) { sAuthNetwork.Stop(); });
    auto worldPort = (uint16_t)sConfigMgr->GetIntDefault("GameServer.Port", 4514);
    std::string bindIp = sConfigMgr->GetStringDefault("GameServer.IP", "0.0.0.0");

    auto pWorldNetwork = std::make_unique<XSocketMgr<WorldSession>>();
    if (!pWorldNetwork->StartWorldNetwork(*ioContext, bindIp.c_str(), worldPort, 2))
    {
        NG_LOG_ERROR("server.worldserver", "Failed to start network");
        return -1;
        // go down and shutdown the server
    }
    std::shared_ptr<void> sWorldHandle(nullptr, [](void *) {
        sWorld.KickAll();
        sMemoryPool.Destroy();

        sObjectMgr.UnloadAll();
        sMapContent.UnloadAll();
    });

#if NG_USE_CLITHREAD
    // Launch CliRunnable thread
    std::shared_ptr<std::thread> cliThread;
    if (sConfigMgr->GetBoolDefault("Console.Enable", true))
    {
        cliThread.reset(new std::thread(CliThread), &ShutdownCLIThread);
    }
#endif

    // Start the Boost based thread pool
    int32_t numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
    std::shared_ptr<std::vector<std::thread>> threadPool(new std::vector<std::thread>(), [ioContext](std::vector<std::thread> *del) {
        ioContext->stop();
        for (std::thread &thr : *del)
            thr.join();

        delete del;
    });

    if (numThreads < 1)
        numThreads = 1;

    for (int32_t i = 0; i < numThreads; ++i)
        threadPool->push_back(std::thread([ioContext]() { ioContext->run(); }));

    WorldUpdateLoop();

    // Shutdown starts here
    threadPool.reset();
    sLog->SetSynchronous();

    pWorldNetwork->StopNetwork();
    dbPingTimer->cancel();
    signals.cancel();

    int32_t exitCode = World::GetExitCode();
    NG_LOG_INFO("server.worldserver", "Exiting with code %d", exitCode);

    return exitCode;
}

void SignalHandler(boost::system::error_code const &error, int32_t /*signalNumber*/)
{
    if (!error)
        World::StopNow(SHUTDOWN_EXIT_CODE);
}

void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32_t dbPingInterval, boost::system::error_code const &error)
{
    if (!error)
    {
        if (std::shared_ptr<boost::asio::deadline_timer> dbPingTimer = dbPingTimerRef.lock())
        {
            NG_LOG_INFO("server.worldserver", "Ping MySQL to keep connection alive");
            GameDatabase.KeepAlive();
            CharacterDatabase.KeepAlive();

            dbPingTimer->expires_from_now(boost::posix_time::minutes(dbPingInterval));
            dbPingTimer->async_wait(std::bind(&KeepDatabaseAliveHandler, dbPingTimerRef, dbPingInterval, std::placeholders::_1));
        }
    }
}

///- Initialize connection to the databases
bool StartDB()
{
    MySQL::Library_Init();

    DatabaseLoader loader("server.worldserver", DatabaseLoader::DATABASE_NONE);
    loader.AddDatabase(GameDatabase, "Game").AddDatabase(CharacterDatabase, "Character");

    if (!loader.Load())
    {
        NG_LOG_ERROR("server.worldserver", "Cannot connect to database");
        return false;
    }

    CharacterDatabase.PExecute("UPDATE `Character` SET logout_time = NOW() WHERE login_time > logout_time;");
    return true;
}

void StopDB()
{
    GameDatabase.Close();
    CharacterDatabase.Close();
    MySQL::Library_End();
}

void WorldUpdateLoop()
{
    uint32_t realCurrTime = 0;
    uint32_t realPrevTime = getMSTime();

    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;
        realCurrTime = getMSTime();

        uint32_t diff = getMSTimeDiff(realPrevTime, realCurrTime);

        sWorld.Update(diff);
        realPrevTime = realCurrTime;

        uint32_t executionTimeDiff = getMSTimeDiff(realCurrTime, getMSTime());

        // we know exactly how long it took to update the world, if the update took less than WORLD_SLEEP_CONST, sleep for WORLD_SLEEP_CONST - world update time
        if (executionTimeDiff < WORLD_SLEEP_CONST)
            std::this_thread::sleep_for(std::chrono::milliseconds(WORLD_SLEEP_CONST - executionTimeDiff));
    }
}

void ShutdownCLIThread(std::thread *cliThread)
{
    if (cliThread != nullptr)
    {
#ifdef _WIN32
        // First try to cancel any I/O in the CLI thread
        if (!CancelSynchronousIo(cliThread->native_handle()))
        {
            // if CancelSynchronousIo() fails, print32_t the error and try with old way
            DWORD errorCode = GetLastError();

            // if CancelSynchronousIo fails with ERROR_NOT_FOUND then there was nothing to cancel, proceed with shutdown
            if (errorCode != ERROR_NOT_FOUND)
            {
                LPSTR errorBuffer;
                DWORD numCharsWritten =
                    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorCode, 0, (LPTSTR)&errorBuffer, 0, nullptr);
                if (!numCharsWritten)
                    errorBuffer = "Unknown error";

                NG_LOG_DEBUG("server.worldserver", "Error cancelling I/O of CliThread, error code %u, detail: %s", uint32_t(errorCode), errorBuffer);

                if (numCharsWritten)
                    LocalFree(errorBuffer);

                // send keyboard input to safely unblock the CLI thread
                INPUT_RECORD b[4];
                HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
                b[0].EventType = KEY_EVENT;
                b[0].Event.KeyEvent.bKeyDown = TRUE;
                b[0].Event.KeyEvent.uChar.AsciiChar = 'X';
                b[0].Event.KeyEvent.wVirtualKeyCode = 'X';
                b[0].Event.KeyEvent.wRepeatCount = 1;

                b[1].EventType = KEY_EVENT;
                b[1].Event.KeyEvent.bKeyDown = FALSE;
                b[1].Event.KeyEvent.uChar.AsciiChar = 'X';
                b[1].Event.KeyEvent.wVirtualKeyCode = 'X';
                b[1].Event.KeyEvent.wRepeatCount = 1;

                b[2].EventType = KEY_EVENT;
                b[2].Event.KeyEvent.bKeyDown = TRUE;
                b[2].Event.KeyEvent.dwControlKeyState = 0;
                b[2].Event.KeyEvent.uChar.AsciiChar = '\r';
                b[2].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
                b[2].Event.KeyEvent.wRepeatCount = 1;
                b[2].Event.KeyEvent.wVirtualScanCode = 0x1c;

                b[3].EventType = KEY_EVENT;
                b[3].Event.KeyEvent.bKeyDown = FALSE;
                b[3].Event.KeyEvent.dwControlKeyState = 0;
                b[3].Event.KeyEvent.uChar.AsciiChar = '\r';
                b[3].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
                b[3].Event.KeyEvent.wVirtualScanCode = 0x1c;
                b[3].Event.KeyEvent.wRepeatCount = 1;
                DWORD numb;
                WriteConsoleInput(hStdIn, b, 4, &numb);
            }
        }
#endif
        cliThread->join();
        delete cliThread;
    }
}