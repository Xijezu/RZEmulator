#include "NetworkThread.h"

NetworkThread::NetworkThread() : _connections(0), _stopped(false), _thread(nullptr), _ioContext(1),
                                 _acceptSocket(_ioContext), _updateTimer(_ioContext)
{
}

NetworkThread::~NetworkThread()
{
    Stop();
    if (_thread)
    {
        Wait();
        delete _thread;
    }
}

bool NetworkThread::Start()
{
    if (_thread)
        return false;

    _thread = new std::thread(&NetworkThread::Run, this);
    return true;
}

void NetworkThread::Stop()
{
    _stopped = true;
    _ioContext.stop();
}

void NetworkThread::Wait()
{
    ASSERT(_thread);

    _thread->join();
    delete _thread;
    _thread = nullptr;
}

void NetworkThread::AddSocket(std::shared_ptr<XSocket> sock)
{
    std::lock_guard<std::mutex> lock(_newSocketsLock);

    ++_connections;
    _newSockets.push_back(sock);
    SocketAdded(sock);
}

void NetworkThread::AddNewSockets()
{
    std::lock_guard<std::mutex> lock(_newSocketsLock);

    if (_newSockets.empty())
        return;

    for (std::shared_ptr<XSocket> sock : _newSockets)
    {
        if (!sock->IsOpen())
        {
            SocketRemoved(sock);
            --_connections;
        }
        else
            _sockets.push_back(sock);
    }

    _newSockets.clear();
}

void NetworkThread::Run()
{
    NG_LOG_DEBUG("network", "Network Thread Starting");

    _updateTimer.expires_from_now(boost::posix_time::milliseconds(10));
    _updateTimer.async_wait(std::bind(&NetworkThread::Update, this));
    _ioContext.run();

    NG_LOG_DEBUG("network", "Network Thread exits");
    _newSockets.clear();
    _sockets.clear();
}

void NetworkThread::Update()
{
    if (_stopped)
        return;

    _updateTimer.expires_from_now(boost::posix_time::milliseconds(10));
    _updateTimer.async_wait(std::bind(&NetworkThread::Update, this));

    AddNewSockets();

    _sockets.erase(std::remove_if(_sockets.begin(), _sockets.end(), [this](std::shared_ptr<XSocket> sock) {
                       if (!sock->Update())
                       {
                           if (sock->IsOpen())
                               sock->CloseSocket();

                           this->SocketRemoved(sock);

                           --this->_connections;
                           return true;
                       }

                       return false;
                   }),
                   _sockets.end());
}