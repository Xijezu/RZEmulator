#include "Common.h"
#include "AuthSession.h"
#include "AuthNetwork.h"

AuthSession::AuthSession(XSocket& socket) : socket_(socket)
{
    _rc4decode.SetKey("}h79q~B%al;k'y $E");
    _rc4encode.SetKey("}h79q~B%al;k'y $E");

}


AuthSession::~AuthSession()
{
    _rc4encode.Clear();
    _rc4decode.Clear();
}

void AuthSession::ProcessIncoming(XPacket* _packet) {
	switch (_packet->GetPacketID())
	{
	case 20011:		// Clientlogin result
	{
        _packet->read_skip(7);
        auto szAccount = _packet->ReadString(61);
        _packet->rpos(0);
		if (m_Queue.count(szAccount) == 1)
		{
			m_Queue[szAccount]->ProcessIncoming(_packet);
			m_Queue.erase(szAccount);
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
	packet << (int64) one_time_key;
	sAuthNetwork->SendPacketToAuth(packet);
}

void AuthSession::Decrypt(void *dest, size_t size, bool isPeek)
{
    _rc4decode.Decode(dest, dest, size, isPeek);
}

void AuthSession::Encrypt(void *dest, size_t size, bool isPeek)
{
    _rc4encode.Encode(dest, dest, size, isPeek);
}
