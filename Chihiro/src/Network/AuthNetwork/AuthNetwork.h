#ifndef _AUTHSESSION_H_
#define _AUTHSESSION_H_

#include "Common.h"
#include "Configuration/Config.h"
#include "AuthSession.h"
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>
#include "Encryption/ByteBuffer.h"

/// Doing this inline, I really don't want to waste another file :p

class AuthNetwork : public ACE_Connector<XSocket, ACE_SOCK_Connector> {
public:
	AuthNetwork()
    {
        ACE_NEW(_socket, XSocket);
        ACE_NEW(_session, AuthSession(*_socket));
        _socket->set_session(_session);
    }

	~AuthNetwork()  = default;

	int InitializeNetwork(ACE_INET_Addr& auth_addr)
    {
        open(ACE_Reactor::instance(), ACE_NONBLOCK);
		if (connect(_socket, auth_addr) != 0)
		{
			return -1;
		}
		SendServerInfo();
		return 0;
	}

	void Stop()
	{
		this->close();
		delete _socket;
	}

    virtual int close() {
        ACE_Connector::close();
    }

	void SendAccountToAuth(WorldSession& session, std::string login_name, uint64 one_time_key)
	{
		_session->AccountToAuth(&session, login_name, one_time_key);
	}

	void SendPacketToAuth(XPacket& packet) {
		_socket->SendPacket(packet);
		_socket->handle_output();
	}

	void SendLogoutToAuth(std::string account) {
		XPacket packet(20012); // Client logout
		packet.fill(account, 61);
		packet << (uint32)0;
		packet.FinalizePacket();
		_socket->SendPacket(packet);
	}

	XSocket* GetSocket() { return _socket; }
private:
	void SendServerInfo() {
		XPacket packet(20001);
		packet << (uint16)sConfigMgr->GetIntDefault("GameServer.Index", 1);
		packet.fill(sConfigMgr->GetStringDefault("GameServer.Name", "Testserver").c_str(), 21);
		packet.fill(sConfigMgr->GetStringDefault("GameServer.SSU", "about:blank").c_str(), 256);
		packet << (uint8)sConfigMgr->GetIntDefault("GameServer.AdultServer", 0);
		packet.fill(sConfigMgr->GetStringDefault("GameServer.IP", "127.0.0.1").c_str(), 16);
		packet << (uint32)sConfigMgr->GetIntDefault("GameServer.Port", 4514);
		_socket->SendPacket(packet);
		return;
	}

	XSocket* _socket;
	AuthSession* _session;
};

#define sAuthNetwork ACE_Singleton<AuthNetwork, ACE_Thread_Mutex>::instance()
#endif // _AUTHSESSION_H_