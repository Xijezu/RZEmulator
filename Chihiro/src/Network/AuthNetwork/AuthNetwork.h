#ifndef _AUTHSESSION_H_
#define _AUTHSESSION_H_

#include "Common.h"
#include "Declarations.h"
#include "Configuration/Config.h"
#include "GameAuthSession.h"
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>
#include "Encryption/ByteBuffer.h"
/// Doing this inline, I really don't want to waste another file :p

class AuthNetwork : public ACE_Connector<WorldSocket<GameAuthSession>, ACE_SOCK_Connector> {
public:
	AuthNetwork()
    {
		m_pSocket = new WorldSocket<GameAuthSession>();
    }

	~AuthNetwork() override = default;

	int InitializeNetwork(ACE_INET_Addr& auth_addr)
    {
        open(ACE_Reactor::instance(), ACE_NONBLOCK);
		if (connect(m_pSocket, auth_addr) != 0)
		{
			return -1;
		}
		m_pSocket->GetSession()->SendGameLogin();
		return 0;
	}

	void Stop()
	{
		this->close();
		delete m_pSocket;
	}

     int close() override
	 {
		 ACE_Connector::close();
	 }

	void SendAccountToAuth(WorldSession& session, const std::string& login_name, uint64 one_time_key)
	{
		m_pSocket->GetSession()->AccountToAuth(&session, login_name, one_time_key);
	}

	void SendClientLogoutToAuth(const std::string& account)
	{
		m_pSocket->GetSession()->ClientLogoutToAuth(account);
	}
private:
	WorldSocket<GameAuthSession>* m_pSocket;
};

#define sAuthNetwork ACE_Singleton<AuthNetwork, ACE_Thread_Mutex>::instance()
#endif // _AUTHSESSION_H_