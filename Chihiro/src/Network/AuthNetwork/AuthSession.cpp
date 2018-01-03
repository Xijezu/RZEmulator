#include "Common.h"
#include "AuthSession.h"
#include "AuthNetwork.h"
#include "../../../Mononoke/src/Server/AuthGame/AuthGamePackets.h"

AuthSession::AuthSession(XSocket& socket) : socket_(socket)
{
}


AuthSession::~AuthSession()
{

}

void AuthSession::ProcessIncoming(XPacket* _packet) {
	switch (_packet->GetPacketID())
	{
	case 20011:		// Clientlogin result
	{
		AG_CLIENT_LOGIN* result = ((AG_CLIENT_LOGIN*)_packet->contents());
		if (m_Queue.count(result->account) == 1)
		{
			m_Queue[result->account]->ProcessIncoming(_packet);
			m_Queue.erase(result->account);
		}
	}
		break;
	default:
		break;
	}
}

void AuthSession::AccountToAuth(WorldSession* session, std::string login_name, uint64 one_time_key)
{
	m_Queue[login_name] = session;
	XPacket packet(20010);
	packet.fill(login_name, 61);
	packet << (int64_t) one_time_key;
	sAuthNetwork->SendPacketToAuth(packet);
}
