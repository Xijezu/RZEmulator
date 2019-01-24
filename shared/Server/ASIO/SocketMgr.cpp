#include "SocketMgr.h"

SocketMgr::SocketMgr() : _acceptor(nullptr), _threads(nullptr), _threadCount(0)
{
}

SocketMgr::~SocketMgr()
{
    ASSERT(!_threads && !_acceptor && !_threadCount, "StopNetwork must be called prior to SocketMgr destruction");
}

bool SocketMgr::StartNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16_t port, int threadCount)
{
    ASSERT(threadCount > 0);

    AsyncAcceptor *acceptor = nullptr;
    try
    {
        acceptor = new AsyncAcceptor(ioContext, bindIp, port);
    }
    catch (boost::system::system_error const &err)
    {
        NG_LOG_ERROR("network", "Exception caught in SocketMgr.StartNetwork (%s:%u): %s", bindIp.c_str(), port, err.what());
        return false;
    }

    if (!acceptor->Bind())
    {
        NG_LOG_ERROR("network", "StartNetwork failed to bind socket acceptor");
        return false;
    }

    _acceptor = acceptor;
    _threadCount = threadCount;
    _threads = CreateThreads();

    ASSERT(_threads);

    for (int32_t i = 0; i < _threadCount; ++i)
        _threads[i].Start();

    return true;
}

void SocketMgr::StopNetwork()
{
    _acceptor->Close();

    if (_threadCount != 0)
        for (int32_t i = 0; i < _threadCount; ++i)
            _threads[i].Stop();

    Wait();

    delete _acceptor;
    _acceptor = nullptr;
    delete[] _threads;
    _threads = nullptr;
    _threadCount = 0;
}

void SocketMgr::Wait()
{
    if (_threadCount != 0)
        for (int32_t i = 0; i < _threadCount; ++i)
            _threads[i].Wait();
}

void SocketMgr::OnSocketOpen(tcp::socket &&sock, uint32_t nThreadIndex)
{
    try
    {
        std::shared_ptr<XSocket> newSocket = std::make_shared<XSocket>(std::move(sock));
        _threads[nThreadIndex].AddSocket(newSocket);
    }
    catch (boost::system::system_error const &err)
    {
        NG_LOG_WARN("network", "Failed to retrieve client's remote address %s", err.what());
    }
}

uint32_t SocketMgr::SelectThreadWithMinConnections() const
{
    uint32_t min{0};

    for (int32_t i = 1; i < _threadCount; ++i)
        if (_threads[i].GetConnectionCount() < _threads[min].GetConnectionCount())
            min = i;

    return min;
}

std::pair<tcp::socket *, uint32_t> SocketMgr::GetSocketForAccept()
{
    uint32_t threadIndex = SelectThreadWithMinConnections();
    return std::make_pair(_threads[threadIndex].GetSocketForAccept(), threadIndex);
}