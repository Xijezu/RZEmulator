#ifndef _AUTHNETWORK_H_
#define _AUTHNETWORK_H_
#include "Server/XSocket.h"
#include "Network/GameNetwork/WorldSession.h"

class AuthSession : public XSocket::Session
{
public:
	AuthSession(XSocket& socket);
	virtual ~AuthSession(void);

	/// Leaving the following empty because there's no usage for them in the auth connection
	virtual void OnAccept(void) { }
	virtual void OnClose(void) { }
	virtual void Decrypt(void*, size_t, bool/* =false */) { }
	virtual void Encrypt(void*, size_t, bool/* =false */) { }
	virtual void ProcessIncoming(XPacket*);
	void AccountToAuth(WorldSession* session, std::string login_name, uint64 one_time_key);
private:
	XSocket& socket_;
	XSocket& socket(void) { return socket_; }
	std::map<std::string, WorldSession*> m_Queue;
};


#endif // _AUTHNETWORK_H_
