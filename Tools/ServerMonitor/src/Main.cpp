#include "Common.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "MonitorSession.h"
#include "MySQLThreading.h"
#include "NGInit.h"
#include "NetworkThread.h"
#include "ServerMonitor.h"
#include "SingleSocketInstance.h"
#include "XSocket.h"
#include <iostream>

bool StartDB();
void StopDB();
void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32_t dbPingInterval, boost::system::error_code const &error);

int main(int argc, char **argv)
{
    auto [bSuccess, ioContext] = NGemity::InitFramework("servermonitor.conf", "ServerMonitor", argc, argv);
    if (!bSuccess)
    {
        return -1;
    }

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);
    sServerMonitor.InitializeServerMonitor();
    NGemity::SingleSocketInstance::Instance().InitializeSingleSocketInstance();

    // Initialize the database connection
    if (!StartDB())
        return 1;
    std::shared_ptr<void> sDBHandler(nullptr, [](void *) { StopDB(); });

    // Enabled a timed callback for handling the database keep alive ping
    int32_t dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);
    std::shared_ptr<boost::asio::deadline_timer> dbPingTimer = std::make_shared<boost::asio::deadline_timer>(*ioContext);
    dbPingTimer->expires_from_now(boost::posix_time::minutes(dbPingInterval));
    dbPingTimer->async_wait(std::bind(&KeepDatabaseAliveHandler, std::weak_ptr<boost::asio::deadline_timer>(dbPingTimer), dbPingInterval, std::placeholders::_1));

    sServerMonitor.InitializeMonitoring(ioContext);
    auto threadPool = NGemity::GetThreadPool(ioContext);

    ioContext->run();

    StopDB();

    return 0;
}

// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    DatabaseLoader loader("server.authserver", DatabaseLoader::DATABASE_NONE);
    loader.AddDatabase(LogDatabase, "Log");
    if (!loader.Load())
    {
        NG_LOG_ERROR("sql.sql", "Cannot connect to database");
        return false;
    }

    return true;
}

void StopDB()
{
    LogDatabase.Close();
    MySQL::Library_End();
}

void KeepDatabaseAliveHandler(std::weak_ptr<boost::asio::deadline_timer> dbPingTimerRef, int32_t dbPingInterval, boost::system::error_code const &error)
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