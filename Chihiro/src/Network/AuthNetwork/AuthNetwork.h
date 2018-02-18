/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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

#ifndef NGEMITY_AUTHSESSION_H_
#define NGEMITY_AUTHSESSION_H_

#include "Common.h"
#include "Declarations.h"
#include "Configuration/Config.h"
#include "GameAuthSession.h"
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>
#include "Encryption/ByteBuffer.h"

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
		 return ACE_Connector::close();
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
#endif // NGEMITY_AUTHSESSION_H_