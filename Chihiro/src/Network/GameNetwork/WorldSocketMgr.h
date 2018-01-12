#ifndef PROJECT_WORLDSOCKETMGR_H
#define PROJECT_WORLDSOCKETMGR_H



#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>

template<class T>
class WorldSocket;
class WorldSession;
template <class T>
class ReactorRunnable;
class ACE_Event_Handler;

/// Manages all sockets connected to peers and network threads
template<class T>
class WorldSocketMgr
{
public:
    friend class WorldSocket<T>;
    friend class ACE_Singleton<WorldSocketMgr<T>, ACE_Thread_Mutex>;

    /// Start network, listen at address:port .
    int StartNetwork(ACE_UINT16 port, const char* address);

    /// Stops all network threads, It will wait for all running threads .
    void StopNetwork();

    /// Wait untill all network threads have "joined" .
    void Wait();

private:
    int OnSocketOpen(WorldSocket<T>* sock);

    int StartReactiveIO(ACE_UINT16 port, const char* address);

private:
    WorldSocketMgr();
    virtual ~WorldSocketMgr();

    ReactorRunnable<T>* m_NetThreads;
    size_t m_NetThreadsCount;

    int m_SockOutKBuff;
    int m_SockOutUBuff;
    bool m_UseNoDelay;

    class WorldSocketAcceptor* m_Acceptor;
};

#define sWorldSocketMgr ACE_Singleton<WorldSocketMgr<WorldSession>, ACE_Thread_Mutex>::instance()

#endif