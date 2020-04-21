#include "Common.h"
#include "Config.h"
#include "IoContext.h"
#include "Log.h"
#include "NGInit.h"
#include "NetworkThread.h"
#include "ShizukeSession.h"
#include "SingleSocketInstance.h"
#include "XPacket.h"
#include "XRc4Cipher.h"
#include "XSocket.h"
#include <iostream>

int main(int argc, char **argv) {
    // Making sure the args are there

    auto [bSuccess, ioContext] =
        NGemity::InitFramework("luna.conf", "Luna", argc, argv);
    if (!bSuccess) {
        return -1;
    }

    ConfigMgr::instance()->SetPacketVersion(EPIC_5_1);
    TS_CS_VERSION versionPct{};
    versionPct.szVersion = "ASER";

    XPacket output;
    MessageSerializerBuffer serializer(&output);
    versionPct.serialize(&serializer);
    auto t = serializer.getFinalizedPacket();

    XRC4Cipher cipher{};
    cipher.SetKey("}h79q~B%al;k'y $E");
    cipher.Encode(t->contents(), t->contents(), t->size());

    std::stringstream str{};
    for (int i = 0; i < t->size(); i++)
        str << std::to_string(t->contents()[i]) << ", ";
    std::cout << str.str();

    NGemity::SingleSocketInstance::Instance().InitializeSingleSocketInstance();
    auto socket = NGemity::GetXSocket<ShizukeSession>(*(ioContext), argv[1],
                                                      std::atoi(argv[2]));
    NGemity::SingleSocketInstance::Instance().AddSocket(socket);

    auto threadPool = NGemity::GetThreadPool(ioContext);

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);

    ioContext->run_for(std::chrono::seconds(10));
    return 0;
}