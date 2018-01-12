/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
  *  Copyright (C) 2011-2017 Project SkyFire <http://www.projectskyfire.org/>
  *  Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
  *  Copyright (C) 2005-2017 MaNGOS <https://www.getmangos.eu/>
  *
  *  This program is free software; you can redistribute it and/or modify it
  *  under the terms of the GNU General Public License as published by the
  *  Free Software Foundation; either version 3 of the License, or (at your
  *  option) any later version.
  *
  *  This program is distributed in the hope that it will be useful, but WITHOUT
  *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
  *  more details.
  *
  *  You should have received a copy of the GNU General Public License along
  *  with this program. If not, see <http://www.gnu.org/licenses/>.
  */
#include "WorldSocketMgr.h"

#include <ace/Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/os_include/netinet/os_tcp.h>
#include <ace/Dev_Poll_Reactor.h>

#include <set>

#include "Log.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "WorldSocket.h"
#include "WorldSocketAcceptor.h"

/**
* This is a helper class to WorldSocketMgr, that manages
* network threads, and assigning connections from acceptor thread
* to other network threads
*/
template<class T>
class ReactorRunnable : protected ACE_Task_Base {
public:

    ReactorRunnable() :
            m_Reactor(0),
            m_Connections(0),
            m_ThreadId(-1)
    {
        ACE_Reactor_Impl *imp;

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)

        imp = new ACE_Dev_Poll_Reactor();

            imp->max_notify_iterations (128);
            imp->restart (1);

#else

        imp = new ACE_TP_Reactor();
        imp->max_notify_iterations(128);

#endif

        m_Reactor = new ACE_Reactor(imp, 1);
    }

    virtual ~ReactorRunnable()
    {
        Stop();
        Wait();

        delete m_Reactor;
    }

    void Stop()
    {
        m_Reactor->end_reactor_event_loop();
    }

    int Start()
    {
        if (m_ThreadId != -1)
            return -1;

        return (m_ThreadId = activate());
    }

    void Wait()
    { ACE_Task_Base::wait(); }

    long Connections()
    {
        return static_cast<long> (m_Connections.value());
    }

    int AddSocket(WorldSocket<T> *sock)
    {
        std::lock_guard<std::mutex> guard(m_NewSockets_Lock);

        ++m_Connections;
        sock->AddReference();
        sock->reactor(m_Reactor);
        m_NewSockets.insert(sock);

        return 0;
    }

    ACE_Reactor *GetReactor()
    {
        return m_Reactor;
    }

protected:

    void AddNewSockets()
    {
        std::lock_guard<std::mutex> guard(m_NewSockets_Lock);

        if (m_NewSockets.empty())
            return;

        for (typename SocketSet::const_iterator i = m_NewSockets.begin(); i != m_NewSockets.end(); ++i) {
            WorldSocket<T> *sock = (*i);

            if (sock->IsClosed()) {
                sock->RemoveReference();
                --m_Connections;
            } else
                m_Sockets.insert(sock);
        }

        m_NewSockets.clear();
    }

    virtual int svc()
    {
        MX_LOG_DEBUG("misc", "Network Thread Starting");

        ACE_ASSERT (m_Reactor);

        typename SocketSet::iterator i, t;

        while (!m_Reactor->reactor_event_loop_done()) {
            // dont be too smart to move this outside the loop
            // the run_reactor_event_loop will modify interval
            ACE_Time_Value interval(0, 10000);

            if (m_Reactor->run_reactor_event_loop(interval) == -1)
                break;
            AddNewSockets();

            for (i = m_Sockets.begin(); i != m_Sockets.end();) {
                if ((*i)->Update() == -1) {
                    t = i;
                    ++i;

                    (*t)->CloseSocket();
                    (*t)->RemoveReference();
                    --m_Connections;
                    m_Sockets.erase(t);
                } else
                    ++i;
            }
        }

        MX_LOG_DEBUG("misc", "Network Thread exits");

        return 0;
    }

private:
    typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX, long> AtomicInt;
    typedef std::set<WorldSocket<T> *>              SocketSet;

    ACE_Reactor *m_Reactor;
    AtomicInt m_Connections;
    int       m_ThreadId;

    SocketSet m_Sockets;

    SocketSet  m_NewSockets;
    std::mutex m_NewSockets_Lock;
};

#ifdef IS_MONONOKE
    template class WorldSocketMgr<AuthClientSession>;
    template class WorldSocketMgr<AuthGameSession>;
#else
    template class WorldSocketMgr<WorldSession>;
#endif

template<class T>
WorldSocketMgr<T>::WorldSocketMgr() :
        m_NetThreads(0),
        m_NetThreadsCount(0),
        m_SockOutKBuff(-1),
        m_SockOutUBuff(65536),
        m_UseNoDelay(true),
        m_Acceptor(0)
{}

template<class T>
WorldSocketMgr<T>::~WorldSocketMgr()
{
    delete[] m_NetThreads;
    delete m_Acceptor;
}

template<class T>
int WorldSocketMgr<T>::StartReactiveIO(ACE_UINT16 port, const char *address)
{
    m_UseNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

    int num_threads = sConfigMgr->GetIntDefault("Network.Threads", 1);

    if (num_threads <= 0) {
        MX_LOG_ERROR("misc", "Network.Threads is wrong in your config file");
        return -1;
    }

    m_NetThreadsCount = static_cast<size_t> (num_threads + 1);

    m_NetThreads = new ReactorRunnable<T>[m_NetThreadsCount];

    MX_LOG_DEBUG("misc", "Max allowed socket connections %d", ACE::max_handles());

    // -1 means use default
    m_SockOutKBuff = sConfigMgr->GetIntDefault("Network.OutKBuff", -1);

    m_SockOutUBuff = sConfigMgr->GetIntDefault("Network.OutUBuff", 65536);

    if (m_SockOutUBuff <= 0) {
        MX_LOG_ERROR("misc", "Network.OutUBuff is wrong in your config file");
        return -1;
    }

    m_Acceptor = new WorldSocketAcceptor<T>;

    ACE_INET_Addr listen_addr(port, address);

    if (m_Acceptor->open(listen_addr, m_NetThreads[0].GetReactor(), ACE_NONBLOCK) == -1) {
        MX_LOG_ERROR("misc", "Failed to open acceptor, check if the port is free");
        return -1;
    }

    for (size_t i = 0; i < m_NetThreadsCount; ++i)
        m_NetThreads[i].Start();

    return 0;
}

template<class T>
int WorldSocketMgr<T>::StartNetwork(ACE_UINT16 port, const char *address)
{
    if (!sLog->ShouldLog("misc", LOG_LEVEL_DEBUG))
        ACE_Log_Msg::instance()->priority_mask(LM_ERROR, ACE_Log_Msg::PROCESS);

    if (StartReactiveIO(port, address) == -1)
        return -1;

    return 0;
}

template<class T>
void WorldSocketMgr<T>::StopNetwork()
{
    if (m_Acceptor) {
        m_Acceptor->close();
    }

    if (m_NetThreadsCount != 0) {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Stop();
    }

    Wait();
}

template<class T>
void WorldSocketMgr<T>::Wait()
{
    if (m_NetThreadsCount != 0) {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Wait();
    }
}

template<class T>
int WorldSocketMgr<T>::OnSocketOpen(WorldSocket<T> *sock)
{
    // set some options here
    if (m_SockOutKBuff >= 0) {
        if (sock->peer().set_option(SOL_SOCKET,
                                    SO_SNDBUF,
                                    (void *) &m_SockOutKBuff,
                                    sizeof(int)) == -1 && errno != ENOTSUP) {
            MX_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen set_option SO_SNDBUF");
            return -1;
        }
    }

    static const int ndoption = 1;

    // Set TCP_NODELAY.
    if (m_UseNoDelay) {
        if (sock->peer().set_option(ACE_IPPROTO_TCP,
                                    TCP_NODELAY,
                                    (void *) &ndoption,
                                    sizeof(int)) == -1) {
            MX_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen: peer().set_option TCP_NODELAY errno = %s", ACE_OS::strerror(errno));
            return -1;
        }
    }

    sock->m_OutBufferSize = static_cast<size_t> (m_SockOutUBuff);

    // we skip the Acceptor Thread
    size_t min = 1;

    ACE_ASSERT (m_NetThreadsCount >= 1);

    for (size_t i = 1; i < m_NetThreadsCount; ++i)
        if (m_NetThreads[i].Connections() < m_NetThreads[min].Connections())
            min = i;

    return m_NetThreads[min].AddSocket(sock);
}
