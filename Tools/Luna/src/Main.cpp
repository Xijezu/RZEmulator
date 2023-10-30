#include <iostream>

#include "Common.h"
#include "LunaSession.h"
#include "NGInit.h"
#include "NetworkThread.h"
#include "SingleSocketInstance.h"
#include "XSocket.h"
#include "cipher/XStrZlibWithSimpleCipherUtil.h"

int main(int argc, char **argv)
{
    // Making sure the args are there
    if (argc < 4) {
        std::cout << "You're missing the IP and port:" << std::endl;
        std::cout << "Luna [IP] [PORT] [USERNAME] [PASSWORD]" << std::endl;
        return 0;
    }

    std::string szIP = argv[1];
    uint16_t nPort = atoi(argv[2]);
    std::string szUser = argv[3];
    std::string szPass = argv[4];

    auto [bSuccess, ioContext] = NGemity::InitFramework("luna.conf", "Luna", argc, argv);
    if (!bSuccess) {
        return -1;
    }

    NGemity::SingleSocketInstance::Instance().InitializeSingleSocketInstance();
    auto socket = NGemity::GetXSocket<LunaSession>(*(ioContext), szIP, nPort);
    NGemity::SingleSocketInstance::Instance().AddSocket(socket);

    auto threadPool = NGemity::GetThreadPool(ioContext);

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);
    socket->InitConnection(szUser, szPass);
    szPass.clear();

    ioContext->run();

    return 0;
}