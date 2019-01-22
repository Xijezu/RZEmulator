#include "Common.h"
#include <iostream>
#include "XPacket.h"
#include "XSocket.h"
#include "XRc4Cipher.h"
#include "IoContext.h"
#include "NetworkThread.h"
#include "Log.h"
#include "ShizukeSession.h"
#include "Config.h"
#include "SingleSocketInstance.h"
#include "NGInit.h"

int main(int argc, char **argv)
{
    // Making sure the args are there
    if (argc < 4)
    {
        std::cout << "You're missing the IP and port:" << std::endl;
        std::cout << "XRZPlayers [IP] [PORT] [TYPE]" << std::endl;
        return 0;
    }

    auto [bSuccess, ioContext] = NGemity::InitFramework("luna.conf", "Luna", argc, argv);
    if (!bSuccess)
    {
        return -1;
    }

    NGemity::SingleSocketInstance::Instance().InitializeSingleSocketInstance();
    auto socket = NGemity::GetXSocket<ShizukeSession>(*(ioContext), argv[1], std::atoi(argv[2]));
    NGemity::SingleSocketInstance::Instance().AddSocket(socket);

    auto threadPool = NGemity::GetThreadPool(ioContext);

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);
    TS_CS_VERSION versionPct{};
    versionPct.szVersion = argv[3];
    socket->SendPacket(versionPct);

    ioContext->run_for(std::chrono::seconds(10));
    return 0;
}