/*
*  Copyright (C) 2016-2016 Xijezu <http://xijezu.com>
*  Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
*  Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
*  Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
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

#ifndef __XSOCKET_H_
#define __XSOCKET_H_

#include <ace/Synch_Traits.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Block.h>
#include <ace/Basic_Types.h>
#include "Common.h"
#include "XPacket.h"

struct TS_MESSAGE;

typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> Base;

class XSocket : public Base
{
public:
	class Session
	{
	public:
		Session(void);
		virtual ~Session(void);

		virtual void ProcessIncoming(XPacket*) = 0;
		virtual void OnAccept(void) = 0;
		virtual void OnClose(void) = 0;
		virtual void Encrypt(void*, size_t, bool = false) = 0;
		virtual void Decrypt(void*, size_t, bool = false) = 0;
	};

	XSocket(void);
	virtual ~XSocket(void);
	/// Mutex type used for various synchronizations.
	typedef ACE_Thread_Mutex LockType;
	typedef ACE_Guard<LockType> GuardType;

	//int SendPacket(TS_MESSAGE& packet, size_t len) { packet.SetChecksum();  return SendPacket(reinterpret_cast<char*>(&packet), len); }
	int SendPacket(XPacket& packet) { packet.FinalizePacket(); return SendPacket((char*)packet.contents(), packet.size()); }
	int SendPacket(const char *buf, size_t len);

    /// Add reference to this object.
    long AddReference(void);

    /// Remove reference to this object.
    long RemoveReference(void);


    /// returning informations for remote host
	const std::string& getRemoteAddress(void) const;
	uint16 getRemotePort(void) const;

	/// things called by ACE framework.

	/// Called on open, the void* is the acceptor.
	virtual int open(void *);
	/// Called on failures inside of the acceptor, don't call from your code.
	virtual int close(int);
	/// Called when we can read from the socket.
	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	/// Called when the socket can write.
	virtual int handle_output(ACE_HANDLE = ACE_INVALID_HANDLE);
	/// Called when connection is closed or error happens.
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE,
		ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

	void set_session(Session* session);

protected:
	/// Helper functions for processing incoming data.
	int handle_input_header(void);
	int handle_input_payload(void);
	int handle_input_missing_data(void);
	/// Drain the queue if its not empty.
	int handle_output_queue(GuardType& g);
	/// Help functions to mark/unmark the socket for output.
	/// @param g the guard is for m_OutBufferLock, the function will release it
	int cancel_wakeup_output(GuardType& g);
	int schedule_wakeup_output(GuardType& g);
	/// Called by ReactorRunnable.
	int Update(void);

private:
	ssize_t noblk_send(ACE_Message_Block &message_block);
	/// This block actually refers to m_RecvWPct contents,
	/// which allows easy and safe writing to it.
	/// It wont free memory when its deleted. m_RecvWPct takes care of freeing.
	ACE_Message_Block m_RecvPct;
	/// Fragment of the received header.
	ACE_Message_Block m_Header;
	/// here are stored the fragments of the received data
	XPacket *m_RecvWPct;
	/// Session to which received packets are routed
	Session* session_;
	/// Address of the remote peer
	std::string _remoteAddress;
	/// Port of the remote peer
	uint16 _remotePort;
	/// Mutex for protecting output related data.
	LockType m_OutBufferLock;
	/// Buffer used for writing output.
	ACE_Message_Block* m_OutBuffer;
	/// Size of the m_OutBuffer.
	size_t m_OutBufferSize;
	/// True if the socket is registered with the reactor for output
	bool m_OutActive;
};

#endif // __XSOCKET_H_

