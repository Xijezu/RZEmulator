#include "Common.h"
#include "NGInit.h"
#include <iostream>

#include "XSocket.h"
#include "NetworkThread.h"
#include "MonitorSession.h"
#include "ServerMonitor.h"
#include "SingleSocketInstance.h"

int main(int argc, char **argv)
{
    auto [bSuccess, ioContext] = NGemity::InitFramework("servermonitor.conf", "ServerMonitor", argc, argv);
    if (!bSuccess)
    {
        return -1;
    }

    sServerMonitor.InitializeServerMonitor();
    NGemity::SingleSocketInstance::Instance().InitializeSingleSocketInstance();

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);
    sServerMonitor.InitializeMonitoring(ioContext);

    auto threadPool = NGemity::GetThreadPool(ioContext);

    ioContext->run_for(std::chrono::seconds(20));

    return 0;
}