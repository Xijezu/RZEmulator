#include "Common.h"
#include <iostream>
#include "XPacket.h"
#include "XSocket.h"
#include "XRc4Cipher.h"
#include "IoContext.h"
#include "NetworkThread.h"
#include "Log.h"
#include "ShizukeSession.h"
#include "Stacktrace.h"
#include "Config.h"
#include "cipher/XStrZlibWithSimpleCipherUtil.h"

template <class TS_SERIALIZABLE_PACKET>
void SendPacket(TS_SERIALIZABLE_PACKET const &packet, XSocket *Socket)
{
    XPacket output;
    MessageSerializerBuffer serializer(&output);
    packet.serialize(&serializer);
    Socket->SendPacket(*serializer.getFinalizedPacket());
}

int main(int argc, char **argv)
{
    // Making sure the args are there
    if (argc < 4)
    {
        std::cout << "You're missing the IP and port:" << std::endl;
        std::cout << "XRZPlayers [IP] [PORT] [TYPE]" << std::endl;
        return 0;
    }

    Stacktrace::enableStacktracing();

    std::string configError;
    if (!sConfigMgr->LoadInitial("mononoke.conf",
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
    boost::asio::ip::tcp_endpoint endpoint(boost::asio::ip::make_address_v4(argv[1]), (uint16_t)atoi(argv[2]));
    boost::asio::ip::tcp::socket socket(*(ioContext.get()));
    m_pNetworkThread = std::make_unique<NetworkThread<XSocket>>();

    try
    {
        socket.connect(endpoint);
        socket.set_option(boost::asio::ip::tcp::no_delay(true));
    }
    catch (std::exception &)
    {
        NG_LOG_ERROR("server.network", "Cannot connect to game server at %s:%d", argv[1], (uint16_t)atoi(argv[2]));
        return false;
    }

    std::shared_ptr<XSocket> pSocket = std::make_shared<XSocket>(std::move(socket));

    pSocket->SetSendBufferSize(65536);
    pSocket->SetSession(new ShizukeSession{pSocket.get()});

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

    //ConfigMgr::instance()->SetPacketVersion(EPIC_9_1);

    TS_CS_REQUEST requestTest{};
    requestTest.t = 's';
    requestTest.command = XStrZlibWithSimpleCipherUtil::Encrypt("cprint('Hello glandu!')");
    //SendPacket(requestTest, pSocket.get());
    TS_CS_SET_PROPERTY property{};

    ioContext->run_for(std::chrono::seconds(5));

    return 0;
}