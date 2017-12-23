#ifndef _AUTHNETWORK_H_
#define _AUTHNETWORK_H_
#include "Server/XSocket.h"
#include "Network/GameNetwork/GameHandler.h"

class AuthNetwork : public XSocket::Session
{
public:
	AuthNetwork(XSocket& socket);
	virtual ~AuthNetwork(void);

	/// Leaving the following empty because there's no usage for them in the auth connection
	virtual void OnAccept(void) { }
	virtual void OnClose(void) { }
	virtual void Decrypt(void*, size_t, bool/* =false */) { }
	virtual void Encrypt(void*, size_t, bool/* =false */) { }
	virtual void ProcessIncoming(XPacket*);
	void AccountToAuth(GameSession* session, std::string login_name, uint64 one_time_key);
private:
	XSocket& socket_;
	XSocket& socket(void) { return socket_; }
	std::map<std::string, GameSession*> m_Queue;
};


#endif // _AUTHNETWORK_H_
