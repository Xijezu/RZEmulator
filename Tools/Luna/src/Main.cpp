#include "Common.h"
#include <iostream>
#include "XPacket.h"
#include "XSocket.h"
#include "XRc4Cipher.h"
#include "IoContext.h"
#include "NetworkThread.h"
#include "Log.h"
#include "LunaSession.h"
#include "Stacktrace.h"
#include "Config.h"

int main(int argc, char **argv)
{
    Stacktrace::enableStacktracing();

    // Making sure the args are there
    if (argc < 4)
    {
        std::cout << "You're missing the IP and port:" << std::endl;
        std::cout << "Luna [IP] [PORT] [USERNAME] [PASSWORD]" << std::endl;
        return 0;
    }

    std::string szIP = argv[1];
    uint16_t nPort = atoi(argv[2]);
    std::string szUser = argv[3];
    std::string szPass = argv[4];

    std::string configError;
    if (!sConfigMgr->LoadInitial("luna.conf",
                                 std::vector<std::string>(argv, argv + argc),
                                 configError))
    {
        printf("Error in config file or file not found: %s\n", configError.c_str());
        return 1;
    }

    std::shared_ptr<NGemity::Asio::IoContext> ioContext = std::make_shared<NGemity::Asio::IoContext>();
    // If logs are supposed to be handled async then we need to pass the IoContext into the Log singleton
    sLog->Initialize(sConfigMgr->GetBoolDefault("Log.Async.Enable", false) ? ioContext.get() : nullptr);

    NG_LOG_INFO("server.authserver", "%s (shizue)", "1.0.0");
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

    std::unique_ptr<NetworkThread<XSocket>> m_pNetworkThread;
    boost::asio::ip::tcp_endpoint endpoint(boost::asio::ip::make_address_v4(szIP), nPort);
    boost::asio::ip::tcp::socket socket(*(ioContext.get()));
    m_pNetworkThread = std::make_unique<NetworkThread<XSocket>>();

    try
    {
        socket.connect(endpoint);
        socket.set_option(boost::asio::ip::tcp::no_delay(true));
    }
    catch (std::exception &)
    {
        NG_LOG_ERROR("server.network", "Cannot connect to game server at %s:%d", szIP.c_str(), nPort);
        return false;
    }

    std::shared_ptr<XSocket> pSocket = std::make_shared<XSocket>(std::move(socket));

    pSocket->SetSendBufferSize(65536);
    pSocket->SetSession(new LunaSession{pSocket.get()});

    m_pNetworkThread->AddSocket(pSocket);
    pSocket->Start();
    m_pNetworkThread->Start();

    int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
    std::shared_ptr<std::vector<std::thread>> threadPool(new std::vector<std::thread>(), [ioContext](std::vector<std::thread> *del) {
        ioContext->stop();
        for (std::thread &thr : *del)
            thr.join();

        delete del;
    });

    if (numThreads < 1)
        numThreads = 1;

    for (int i = 0; i < numThreads; ++i)
        threadPool->push_back(std::thread([ioContext]() { ioContext->run(); }));

    ConfigMgr::instance()->SetPacketVersion(EPIC_9_5_2);
    reinterpret_cast<LunaSession *>(pSocket->GetSession())->InitConnection(szUser, szPass);
    szPass.clear();

    ioContext->run_for(std::chrono::seconds(10));

    return 0;
}