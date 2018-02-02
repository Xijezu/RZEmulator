#ifndef PROJECT_WORLDSOCKETMGR_H
#define PROJECT_WORLDSOCKETMGR_H

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

#include <ace/Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/os_include/netinet/os_tcp.h>
#include <ace/Dev_Poll_Reactor.h>

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include <set>

#include "Log.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "WorldSocket.h"
#include "WorldSocketAcceptor.h"

template<class T> class WorldSocket;
template<class T> class WorldSocketAcceptor;

class ACE_Event_Handler;

template<class T>
class ReactorRunnable : protected ACE_Task_Base
{
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

        void Wait() { ACE_Task_Base::wait(); }

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

            for (typename SocketSet::const_iterator i = m_NewSockets.begin(); i != m_NewSockets.end(); ++i)
            {
                WorldSocket<T> *sock = (*i);

                if (sock->IsClosed())
                {
                    sock->RemoveReference();
                    --m_Connections;
                }
                else
                    m_Sockets.insert(sock);
            }

            m_NewSockets.clear();
        }

        virtual int svc()
        {
            MX_LOG_DEBUG("misc", "Network Thread Starting");

            ACE_ASSERT (m_Reactor);

            typename SocketSet::iterator i, t;

            while (!m_Reactor->reactor_event_loop_done())
            {
                // dont be too smart to move this outside the loop
                // the run_reactor_event_loop will modify interval
                ACE_Time_Value interval(0, 10000);

                if (m_Reactor->run_reactor_event_loop(interval) == -1)
                    break;
                AddNewSockets();

                for (i = m_Sockets.begin(); i != m_Sockets.end();)
                {
                    if ((*i)->Update() == -1)
                    {
                        t = i;
                        ++i;

                        (*t)->CloseSocket();
                        (*t)->RemoveReference();
                        --m_Connections;
                        m_Sockets.erase(t);
                    }
                    else
                        ++i;
                }
            }

            MX_LOG_DEBUG("misc", "Network Thread exits");

            return 0;
        }

    private:
        typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX, long> AtomicInt;
        typedef std::set<WorldSocket<T> *>           SocketSet;

        ACE_Reactor *m_Reactor;
        AtomicInt   m_Connections;
        int         m_ThreadId;

        SocketSet m_Sockets;

        SocketSet  m_NewSockets;
        std::mutex m_NewSockets_Lock;
};

/// Manages all sockets connected to peers and network threads
template<class T>
class WorldSocketMgr
{
    public:
        friend class WorldSocket<T>;
        friend class ACE_Singleton<WorldSocketMgr<T>, ACE_Thread_Mutex>;

        /// Start network, listen at address:port .
        int StartNetwork(ACE_UINT16 port, const char *address)
        {
            if (!sLog->ShouldLog("misc", LOG_LEVEL_DEBUG))
                ACE_Log_Msg::instance()->priority_mask(LM_ERROR, ACE_Log_Msg::PROCESS);

            if (StartReactiveIO(port, address) == -1)
                return -1;

            return 0;
        }

        /// Stops all network threads, It will wait for all running threads .
        void StopNetwork()
        {
            if (m_Acceptor)
            {
                m_Acceptor->close();
            }

            if (m_NetThreadsCount != 0)
            {
                for (size_t i = 0; i < m_NetThreadsCount; ++i)
                    m_NetThreads[i].Stop();
            }

            Wait();
        }

        /// Wait untill all network threads have "joined" .
        void Wait()
        {
            if (m_NetThreadsCount != 0)
            {
                for (size_t i = 0; i < m_NetThreadsCount; ++i)
                    m_NetThreads[i].Wait();
            }
        }

    private:
        int OnSocketOpen(WorldSocket<T> *sock)
        {
            // set some options here
            if (m_SockOutKBuff >= 0)
            {
                if (sock->peer().set_option(SOL_SOCKET,
                                            SO_SNDBUF,
                                            (void *)&m_SockOutKBuff,
                                            sizeof(int)) == -1 && errno != ENOTSUP)
                {
                    MX_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen set_option SO_SNDBUF");
                    return -1;
                }
            }

            static const int ndoption = 1;

            // Set TCP_NODELAY.
            if (m_UseNoDelay)
            {
                if (sock->peer().set_option(ACE_IPPROTO_TCP,
                                            TCP_NODELAY,
                                            (void *)&ndoption,
                                            sizeof(int)) == -1)
                {
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

        int StartReactiveIO(ACE_UINT16 port, const char *address)
        {
            m_UseNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

            int num_threads = sConfigMgr->GetIntDefault("Network.Threads", 1);

            if (num_threads <= 0)
            {
                MX_LOG_ERROR("misc", "Network.Threads is wrong in your config file");
                return -1;
            }

            m_NetThreadsCount = static_cast<size_t> (num_threads + 1);

            m_NetThreads = new ReactorRunnable<T>[m_NetThreadsCount];

            MX_LOG_DEBUG("misc", "Max allowed socket connections %d", ACE::max_handles());

            // -1 means use default
            m_SockOutKBuff = sConfigMgr->GetIntDefault("Network.OutKBuff", -1);

            m_SockOutUBuff = sConfigMgr->GetIntDefault("Network.OutUBuff", 65536);

            if (m_SockOutUBuff <= 0)
            {
                MX_LOG_ERROR("misc", "Network.OutUBuff is wrong in your config file");
                return -1;
            }

            m_Acceptor = new WorldSocketAcceptor<T>;

            if (port != 0)
            {
                ACE_INET_Addr listen_addr(port, address);

                if (m_Acceptor->open(listen_addr, m_NetThreads[0].GetReactor(), ACE_NONBLOCK) == -1)
                {
                    MX_LOG_ERROR("misc", "Failed to open acceptor, check if the port is free");
                    return -1;
                }
            }

            for (size_t i = 0; i < m_NetThreadsCount; ++i)
                m_NetThreads[i].Start();

            return 0;
        }

    private:
        WorldSocketMgr() :
                m_NetThreads(0),
                m_NetThreadsCount(0),
                m_SockOutKBuff(-1),
                m_SockOutUBuff(65536),
                m_UseNoDelay(true),
                m_Acceptor(0) {}

        virtual ~WorldSocketMgr()
        {
            delete[] m_NetThreads;
            delete m_Acceptor;
        }

        ReactorRunnable<T> *m_NetThreads;
        size_t m_NetThreadsCount;

        int  m_SockOutKBuff;
        int  m_SockOutUBuff;
        bool m_UseNoDelay;

        WorldSocketAcceptor<T> *m_Acceptor;
};
#endif