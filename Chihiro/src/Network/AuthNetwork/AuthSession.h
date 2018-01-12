#ifndef _AUTHNETWORK_H_
#define _AUTHNETWORK_H_
#include "Server/XSocket.h"
#include "Network/GameNetwork/WorldSession.h"

class AuthSession : public XSocket::Session
{
public:
	explicit AuthSession(XSocket& socket);
	~AuthSession();

	/// Leaving the following empty because there's no usage for them in the auth connection
	virtual void OnAccept(void) { }
	virtual void OnClose(void) { }
	void Decrypt(void*, size_t, bool/* =false */) override;
	void Encrypt(void*, size_t, bool/* =false */) override;
	void ProcessIncoming(XPacket*) override;
	void AccountToAuth(WorldSession* session, std::string login_name, uint64 one_time_key);
private:
	XRC4Cipher	_rc4encode;
	XRC4Cipher	_rc4decode;

	XSocket& socket_;
	XSocket& socket(void) { return socket_; }
	std::map<std::string, WorldSession*> m_Queue;
};


#endif // _AUTHNETWORK_H_
