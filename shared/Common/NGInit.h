#include "Common.h"
#include "SystemConfigs.h"
#include "IoContext.h"
#include <thread>
#include "Stacktrace.h"
#include "Config.h"
#include "Log.h"

namespace NGemity
{
using NGInitTuple = std::tuple<bool, std::shared_ptr<NGemity::Asio::IoContext>>;

static std::shared_ptr<std::vector<std::thread>> GetThreadPool(std::shared_ptr<NGemity::Asio::IoContext> ioContext)
{
    int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 2);
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

    return threadPool;
}

static NGInitTuple InitFramework(const std::string &szLogfile, const std::string &szLibraryName, int argc, char **argv)
{
    Stacktrace::enableStacktracing();

    std::string configError;
    if (!sConfigMgr->LoadInitial(szLogfile,
                                 std::vector<std::string>(argv, argv + argc),
                                 configError))
    {
        printf("Error in config file or file not found: %s\n", configError.c_str());
        return NGInitTuple(false, nullptr);
    }

    auto ioContext = std::make_shared<NGemity::Asio::IoContext>();
    // If logs are supposed to be handled async then we need to pass the IoContext into the Log singleton
    sLog->Initialize(sConfigMgr->GetBoolDefault("Log.Async.Enable", false) ? ioContext.get() : nullptr);

    NG_LOG_INFO("server.authserver", "%s (%s)", _FULLVERSION, szLibraryName.c_str());
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

    return NGInitTuple(true, ioContext);
}
} // namespace NGemity