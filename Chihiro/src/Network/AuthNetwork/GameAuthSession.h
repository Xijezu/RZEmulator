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

#ifndef NGEMITY_GAMEAUTHSESSION_H
#define NGEMITY_GAMEAUTHSESSION_H

#include "Common.h"

template<class T> class WorldSocket;
class XPacket;
class WorldSession;

// Handle login commands
class GameAuthSession
{
public:
	typedef WorldSocket<GameAuthSession> AuthSocket;
	typedef UNORDERED_MAP<std::string, WorldSession*> AuthQueue;
	explicit GameAuthSession(AuthSocket *pSocket);
	~GameAuthSession();

	// Network handlers
	void OnClose();
	void ProcessIncoming(XPacket *);

	// Packet handlers
	void HandleGameLoginResult(XPacket *);
	void HandleClientLoginResult(XPacket *);
	void HandleClientKick(XPacket *);

    void SendGameLogin();
	void AccountToAuth(WorldSession* pSession, const std::string& szLoginName, uint64 nOneTimeKey);
    void ClientLogoutToAuth(const std::string& account);


    int GetAccountId() const;
	std::string GetAccountName();

private:
	AuthQueue  m_queue;
	AuthSocket *m_pSocket;

	uint16 m_nGameIDX;
	std::string m_szGameName;
	std::string m_szGameSSU;
	bool m_bGameIsAdultServer;
	std::string m_szGameIP;
	int m_nGamePort;
};

#endif // NGEMITY_GAMEAUTHSESSION_H