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

#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "MySQLThreading.h"

#include "AuthClientSocketMgr.h"
#include "AuthGameSocketMgr.h"
#include "SystemConfigs.h"
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem/operations.hpp>
#include <csignal>

bool StartDB();
void StopDB();

bool stopEvent{false};                                     // Setting it to true stops the server
# define _MONONOKE_CORE_CONFIG  "authserver.conf"

namespace fs = boost::filesystem;

void SignalHandler(std::weak_ptr<NGemity::Asio::IoContext> ioContextRef, boost::system::error_code const &error, int /*signalNumber*/);
void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32 dbPingInterval, boost::system::error_code const &error);

extern int main(int argc, char **argv)
{
    //sLog->Initialize();
    auto configFile = fs::absolute((std::string)_MONONOKE_CORE_CONFIG);
    std::string configError;

    if (!sConfigMgr->LoadInitial(_MONONOKE_CORE_CONFIG,
                                 std::vector<std::string>(argv, argv + argc),
                                 configError))
    {
        printf("Error in config file or file not found: %s\n", configError.c_str());
        return 1;
    }

    std::shared_ptr<NGemity::Asio::IoContext> ioContext = std::make_shared<NGemity::Asio::IoContext>();
    // If logs are supposed to be handled async then we need to pass the IoContext into the Log singleton
    sLog->Initialize(sConfigMgr->GetBoolDefault("Log.Async.Enable", false) ? ioContext.get() : nullptr);

    NG_LOG_INFO("server.authserver", "%s (authserver)", _FULLVERSION);
    NG_LOG_INFO("server.authserver", "       _   _  _____                _ _");
    NG_LOG_INFO("server.authserver", "      | \\ | |/ ____|              (_) |");
    NG_LOG_INFO("server.authserver", "      |  \\| | |  __  ___ _ __ ___  _| |_ _   _");
    NG_LOG_INFO("server.authserver", "      | . ` | | |_ |/ _ \\ '_ ` _ \\| | __| | | |");
    NG_LOG_INFO("server.authserver", "      | |\\  | |__| |  __/ | | | | | | |_| |_| |");
    NG_LOG_INFO("server.authserver", "      |_| \\_|\\_____|\\___|_| |_| |_|_|\\__|\\__, |");
    NG_LOG_INFO("server.authserver", "                                          __/ |");
    NG_LOG_INFO("server.authserver", "                                         |___/");
    NG_LOG_INFO("server.authserver", "           NGemity (c) 2018 - For Rappelz");
    NG_LOG_INFO("server.authserver", "               <https://ngemity.org/>");

    auto                  authPort   = (uint16)sConfigMgr->GetIntDefault("Authserver.Port", 4500);
    std::string           authBindIp = sConfigMgr->GetStringDefault("Authserver.IP", "0.0.0.0");
    if (!sACSocketMgr.StartWorldNetwork(*ioContext, authBindIp, authPort, 1))
    {
        NG_LOG_ERROR("server.authserver", "Authnetwork startup failed: %s:%d", authBindIp.c_str(), authPort);
    }
    std::shared_ptr<void> sACNetwork(nullptr, [](void *) { sACSocketMgr.StopNetwork(); });

    auto                  gamePort = (uint16)sConfigMgr->GetIntDefault("Gameserver.Port", 4502);
    std::string           bindIp   = sConfigMgr->GetStringDefault("Gameserver.IP", "0.0.0.0");
    if (!sAGSocketMgr.StartWorldNetwork(*ioContext, bindIp, gamePort, 1))
    {
        NG_LOG_ERROR("server.authserver", "Gamenetwork startup failed: %s:%d", bindIp.c_str(), gamePort);
    }
    std::shared_ptr<void> sAGNetwork(nullptr, [](void *) { sAGSocketMgr.StopNetwork(); });

    // Initialize the database connection
    if (!StartDB())
        return 1;
    std::shared_ptr<void> sDBHandler(nullptr, [](void *) { StopDB(); });

    // Set signal handlers
    boost::asio::signal_set signals(*ioContext, SIGINT, SIGTERM);
#if PLATFORM == PLATFORM_WINDOWS
    signals.add(SIGBREAK);
#endif
    signals.async_wait(std::bind(&SignalHandler, std::weak_ptr<NGemity::Asio::IoContext>(ioContext), std::placeholders::_1, std::placeholders::_2));

    // Enabled a timed callback for handling the database keep alive ping
    int32                                        dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);
    std::shared_ptr<boost::asio::deadline_timer> dbPingTimer    = std::make_shared<boost::asio::deadline_timer>(*ioContext);
    dbPingTimer->expires_from_now(boost::posix_time::minutes(dbPingInterval));
    dbPingTimer->async_wait(std::bind(&KeepDatabaseAliveHandler, std::weak_ptr<boost::asio::deadline_timer>(dbPingTimer), dbPingInterval, std::placeholders::_1));

    // Start the io service worker loop
    ioContext->run();

    dbPingTimer->cancel();
    signals.cancel();

    NG_LOG_INFO("server.authserver", "Stopping Mononoke...");

    return 0;
}

void SignalHandler(std::weak_ptr<NGemity::Asio::IoContext> ioContextRef, boost::system::error_code const &error, int /*signalNumber*/)
{
    if (!error)
        if (std::shared_ptr<NGemity::Asio::IoContext> ioContext = ioContextRef.lock())
            ioContext->stop();
}

void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32 dbPingInterval, boost::system::error_code const &error)
{
    if (!error)
    {
        if (std::shared_ptr<boost::asio::deadline_timer> dbPingTimer = dbPingTimerRef.lock())
        {
            NG_LOG_INFO("server.authserver", "Ping MySQL to keep connection alive");
            LoginDatabase.KeepAlive();

            dbPingTimer->expires_from_now(boost::posix_time::minutes(dbPingInterval));
            dbPingTimer->async_wait(std::bind(&KeepDatabaseAliveHandler, dbPingTimerRef, dbPingInterval, std::placeholders::_1));
        }
    }
}

// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    DatabaseLoader loader("server.authserver", DatabaseLoader::DATABASE_NONE);
    loader.AddDatabase(LoginDatabase, "Auth");
    if (!loader.Load())
    {
        NG_LOG_ERROR("server.authserver", "Cannot connect to database");
        return false;
    }

    return true;
}

void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}