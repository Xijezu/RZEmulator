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
#include "AuthNetwork.h"
#include "World.h"
#include "SystemConfigs.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "Maploader.h"
#include "XSocketMgr.h"

#include <fstream>

#ifndef _CHIHIRO_CORE_CONFIG
# define _CHIHIRO_CORE_CONFIG  "chihiro.conf"
#endif //_CHIHIRO_CORE_CONFIG

bool StartDB();
void StopDB();
void WorldUpdateLoop();
constexpr int WORLD_SLEEP_CONST = 50;

int main(int argc, char **argv)
{
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
    std::shared_ptr<void> sDBHandle(nullptr, [](void*) { StopDB(); });

    sWorld.InitWorld();
    if (!sAuthNetwork.InitializeNetwork(*ioContext, sConfigMgr->GetStringDefault("AuthServer.IP", "127.0.0.1"), sConfigMgr->GetIntDefault("AuthServer.Port", 4502)))
    {
        NG_LOG_ERROR("server.worldserver", "Cannot connect to the auth server!");
        return 1;
    }
    std::shared_ptr<void> sAuthHandle(nullptr, [](void*) { sAuthNetwork.Stop(); });

    auto        worldPort = (uint16)sConfigMgr->GetIntDefault("GameServer.Port", 4514);
    std::string bindIp    = sConfigMgr->GetStringDefault("GameServer.IP", "0.0.0.0");
    if (!XSocketMgr<WorldSession>::Instance().StartWorldNetwork(*ioContext, bindIp.c_str(), worldPort, 2))
    {
        NG_LOG_ERROR("server.worldserver", "Failed to start network");
        return -1;
        // go down and shutdown the server
    }
    std::shared_ptr<void> sWorldHandle(nullptr, [](void*) {
        sWorld.KickAll();
        XSocketMgr<WorldSession>::Instance().StopNetwork();
        sMemoryPool.Destroy();

        sObjectMgr.UnloadAll();
        sMapContent.UnloadAll();
    });


    // Start the Boost based thread pool
    int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
    std::shared_ptr<std::vector<std::thread>> threadPool(new std::vector<std::thread>(), [ioContext](std::vector<std::thread>* del)
    {
        ioContext->stop();
        for (std::thread& thr : *del)
            thr.join();

        delete del;
    });

    if (numThreads < 1)
        numThreads = 1;

    for (int i = 0; i < numThreads; ++i)
        threadPool->push_back(std::thread([ioContext]() { ioContext->run(); }));

    WorldUpdateLoop();

    int exitCode = World::GetExitCode();
    NG_LOG_INFO("server.worldserver", "Exiting with code %d", exitCode);

    return exitCode;
}

///- Initialize connection to the databases
bool StartDB()
{
    MySQL::Library_Init();

    DatabaseLoader loader("server.worldserver", DatabaseLoader::DATABASE_NONE);
    loader.AddDatabase(GameDatabase, "Game")
          .AddDatabase(CharacterDatabase, "Character");

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
    uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();

    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;
        realCurrTime = getMSTime();

        uint32 diff = getMSTimeDiff(realPrevTime, realCurrTime);

        sWorld.Update(diff);
        realPrevTime = realCurrTime;

        uint32 executionTimeDiff = getMSTimeDiff(realCurrTime, getMSTime());

        // we know exactly how long it took to update the world, if the update took less than WORLD_SLEEP_CONST, sleep for WORLD_SLEEP_CONST - world update time
        if (executionTimeDiff < WORLD_SLEEP_CONST)
            std::this_thread::sleep_for(std::chrono::milliseconds(WORLD_SLEEP_CONST - executionTimeDiff));
    }
}