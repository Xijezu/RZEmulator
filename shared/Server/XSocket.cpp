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

#include "XSocket.h"
#include "Logging/Log.h"

#include <ace/Message_Block.h>
#include <ace/OS_NS_string.h>
#include <ace/INET_Addr.h>
#include <ace/SString.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

XSocket::Session::Session(void) { }

XSocket::Session::~Session(void) { }

XSocket::XSocket(void)
	: session_(NULL), _remoteAddress(), m_Header(sizeof(TS_MESSAGE)), m_RecvWPct(0), m_RecvPct(),
	  m_OutBuffer(0), m_OutBufferSize(65536), m_OutActive(false)
{
	reference_counting_policy().value(ACE_Event_Handler::Reference_Counting_Policy::ENABLED);

	msg_queue()->high_water_mark(8 * 1024 * 1024);
	msg_queue()->low_water_mark(8 * 1024 * 1024);
}

XSocket::~XSocket(void)
{
	delete m_RecvWPct;

	if (m_OutBuffer)
		m_OutBuffer->release();

	closing_ = true;

	peer().close();
}

int XSocket::open(void * a)
{
	ACE_UNUSED_ARG(a);

	// Prevent double call to this func.
	if (m_OutBuffer)
		return -1;

	// This will also prevent the socket from being Updated
	// while we are initializing it.
	m_OutActive = true;

	// Allocate the buffer.
	ACE_NEW_RETURN(m_OutBuffer, ACE_Message_Block(m_OutBufferSize), -1);

	// Store peer address.
	ACE_INET_Addr remote_addr;

	if (peer().get_remote_addr(remote_addr) == -1)
	{
		MX_LOG_ERROR("network", "XSocket::open: peer().get_remote_addr errno = %s", ACE_OS::strerror(errno));
		return -1;
	}

	_remoteAddress = remote_addr.get_host_addr();
	_remotePort = remote_addr.get_port_number();

	// Register with ACE Reactor
	if (reactor()->register_handler(this, ACE_Event_Handler::READ_MASK | ACE_Event_Handler::WRITE_MASK) == -1)
	{
		MX_LOG_ERROR("network", "XSocket::open: unable to register client handler errno = %s", ACE_OS::strerror(errno));
		return -1;
	}

	// reactor takes care of the socket from now on
	remove_reference();

	return 0;
}

int XSocket::close(int)
{
	shutdown();

	closing_ = true;

	remove_reference();

	return 0;
}

const std::string& XSocket::getRemoteAddress(void) const
{
	return _remoteAddress;
}

uint16 XSocket::getRemotePort(void) const
{
	return _remotePort;
}

ssize_t XSocket::noblk_send(ACE_Message_Block &message_block)
{
	const size_t len = message_block.length();

	if (len == 0)
		return -1;

	// Try to send the message directly.
	ssize_t n = peer().send(message_block.rd_ptr(), len, MSG_NOSIGNAL);

	if (n < 0)
	{
		if (errno == EWOULDBLOCK) // Blocking signal
			return 0;
		else // Error happened
			return -1;
	}
	else if (n == 0)
	{
		// Can this happen ?
		return -1;
	}

	// return bytes transmitted
	return n;
}

int XSocket::SendPacket(const char *buf, size_t len)
{
	ACE_GUARD_RETURN(LockType, Guard, m_OutBufferLock, -1);

	if (closing_)
		return -1;

	// Encrypt the packet before sending
	session_->Encrypt((void*)buf, len);

	if (m_OutBuffer->space() >= len && msg_queue()->is_empty())
	{
		// Put the packet on the buffer.

		if (len != 0)
			if (m_OutBuffer->copy(buf, len) == -1)
				ACE_ASSERT(false);
	}
	else
	{
		// Enqueue the packet.
		ACE_Message_Block* mb;

		ACE_NEW_RETURN(mb, ACE_Message_Block(len), -1);

		if (len != 0)
			mb->copy(buf, len);

		if (msg_queue()->enqueue_tail(mb, (ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
		{
			MX_LOG_ERROR("network", "XSocket::SendPacket enqueue_tail failed");
			mb->release();
			return -1;
		}
	}

	return 0;
}

int XSocket::handle_output(ACE_HANDLE)
{
	ACE_GUARD_RETURN(LockType, Guard, m_OutBufferLock, -1);

	if (closing_)
		return -1;

	size_t send_len = m_OutBuffer->length();

	if (send_len == 0)
		return handle_output_queue(Guard);

#ifdef MSG_NOSIGNAL
	ssize_t n = peer().send(m_OutBuffer->rd_ptr(), send_len, MSG_NOSIGNAL);
#else
	ssize_t n = peer().send(m_OutBuffer->rd_ptr(), send_len);
#endif // MSG_NOSIGNAL

	if (n == 0)
		return -1;
	else if (n == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return schedule_wakeup_output(Guard);

		return -1;
	}
	else if (n < (ssize_t)send_len) //now n > 0
	{
		m_OutBuffer->rd_ptr(static_cast<size_t> (n));

		// move the data to the base of the buffer
		m_OutBuffer->crunch();

		return schedule_wakeup_output(Guard);
	}
	else //now n == send_len
	{
		m_OutBuffer->reset();

		return handle_output_queue(Guard);
	}

	ACE_NOTREACHED(return 0);
}

int XSocket::handle_output_queue(GuardType& g)
{
	if (msg_queue()->is_empty())
		return cancel_wakeup_output(g);

	ACE_Message_Block *mblk;

	if (msg_queue()->dequeue_head(mblk, (ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
	{
		MX_LOG_ERROR("network", "XSocket::handle_output_queue dequeue_head");
		return -1;
	}

	const size_t send_len = mblk->length();

#ifdef MSG_NOSIGNAL
	ssize_t n = peer().send(mblk->rd_ptr(), send_len, MSG_NOSIGNAL);
#else
	ssize_t n = peer().send(mblk->rd_ptr(), send_len);
#endif // MSG_NOSIGNAL

	if (n == 0)
	{
		mblk->release();

		return -1;
	}
	else if (n == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
		{
			msg_queue()->enqueue_head(mblk, (ACE_Time_Value*)&ACE_Time_Value::zero);
			return schedule_wakeup_output(g);
		}

		mblk->release();
		return -1;
	}
	else if (n < (ssize_t)send_len) //now n > 0
	{
		mblk->rd_ptr(static_cast<size_t> (n));

		if (msg_queue()->enqueue_head(mblk, (ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
		{
			MX_LOG_ERROR("network", "XSocket::handle_output_queue enqueue_head");
			mblk->release();
			return -1;
		}

		return schedule_wakeup_output(g);
	}
	else //now n == send_len
	{
		mblk->release();

		return msg_queue()->is_empty() ? cancel_wakeup_output(g) : ACE_Event_Handler::WRITE_MASK;
	}

	ACE_NOTREACHED(return -1);
}

int XSocket::handle_close(ACE_HANDLE h, ACE_Reactor_Mask)
{
	// Critical section
	{
		ACE_GUARD_RETURN(LockType, Guard, m_OutBufferLock, -1);

		closing_ = true;

		if (h == ACE_INVALID_HANDLE)
			peer().close_writer();
	}

	if (session_) {
		session_->OnClose();
		delete session_;
	}

	reactor()->remove_handler(this, ACE_Event_Handler::DONT_CALL | ACE_Event_Handler::ALL_EVENTS_MASK);
	return 0;
}

int XSocket::handle_input(ACE_HANDLE)
{
	if (closing_)
		return -1;

	switch (handle_input_missing_data())
	{
	case -1:
	{
		if ((errno == EWOULDBLOCK) ||
			(errno == EAGAIN))
		{
			return Update();                           // interesting line, isn't it ?
		}

		MX_LOG_DEBUG("network", "XSocket::handle_input: Peer error closing connection errno = %s", ACE_OS::strerror(errno));

		errno = ECONNRESET;
		return -1;
	}
	case 0:
	{
		MX_LOG_DEBUG("network", "XSocket::handle_input: Peer has closed connection");

		errno = ECONNRESET;
		return -1;
	}
	case 1:
		return 1;
	default:
		return Update();                               // another interesting line ;)
	}

	ACE_NOTREACHED(return -1);
}

void XSocket::set_session(Session* session)
{
// 	if (session_ != NULL)
// 		delete session_;

	session_ = session;
}

int XSocket::handle_input_header(void)
{
	ACE_ASSERT(m_RecvWPct == NULL);

	ACE_ASSERT(m_Header.length() == sizeof(TS_MESSAGE));

	session_->Decrypt((uint8*)m_Header.rd_ptr(), sizeof(TS_MESSAGE), true);

	TS_MESSAGE& header = *((TS_MESSAGE*)m_Header.rd_ptr());

	if ((header.size < 7) || (header.size > 0xFFFF) || (header.id > 0xFFFF))
	{
		MX_LOG_ERROR("network", "XSocket::handle_input_header() - Malfunctioned header");
		errno = EINVAL;
		return -1;
	}

	ACE_NEW_RETURN(m_RecvWPct, XPacket((uint16_t)header.id, header.size, m_Header.rd_ptr()), -1);

	if (header.size > 0)
	{
		m_RecvWPct->resize(header.size);
		m_RecvPct.base((char*)m_RecvWPct->contents(), m_RecvWPct->size());
	}
	else
	{
		ACE_ASSERT(m_RecvPct.space() == 0);
	}

	return 0;
}

int XSocket::handle_input_missing_data(void)
{
	char buf[4096];

	ACE_Data_Block db(sizeof(buf), ACE_Message_Block::MB_DATA, buf, 0, 0, ACE_Message_Block::DONT_DELETE, 0);

	ACE_Message_Block message_block(&db, ACE_Message_Block::DONT_DELETE, 0);

	const size_t recv_size = message_block.space();

	const ssize_t n = peer().recv(message_block.wr_ptr(), recv_size);

	if (n <= 0)
		return (int)n;

	message_block.wr_ptr(n);

	while (message_block.length() > 0)
	{
		if (m_Header.space() > 0)
		{
			//need to receive the header
			const size_t to_header = (message_block.length() > m_Header.space() ? m_Header.space() : message_block.length());
			m_Header.copy(message_block.rd_ptr(), to_header);

			if (m_Header.space() > 0)
			{
				// Couldn't receive the whole header this time.
				ACE_ASSERT(message_block.length() == 0);
				errno = EWOULDBLOCK;
				return -1;
			}

			// We just received nice new header
			if (handle_input_header() == -1)
			{
				ACE_ASSERT((errno != EWOULDBLOCK) && (errno != EAGAIN));
				return -1;
			}
		}

		// Its possible on some error situations that this happens
		// for example on closing when epoll receives more chunked data and stuff
		// hope this is not hack, as proper m_RecvWPct is asserted around
		if (!m_RecvWPct)
		{
			MX_LOG_ERROR("network", "Forcing close on input m_RecvWPct = NULL");
			errno = EINVAL;
			return -1;
		}

		// We have full read header, now check the data payload
		if (m_RecvPct.space() > 0)
		{
			//need more data in the payload
			const size_t to_data = (message_block.length() > m_RecvPct.space() ? m_RecvPct.space() : message_block.length());
			m_RecvPct.copy(message_block.rd_ptr(), to_data);
			message_block.rd_ptr(to_data);

			if (m_RecvPct.space() > 0)
			{
				// Couldn't receive the whole data this time.
				ACE_ASSERT(message_block.length() == 0);
				errno = EWOULDBLOCK;
				return -1;
			}
		}

		//just received fresh new payload
		if (handle_input_payload() == -1)
		{
			ACE_ASSERT((errno != EWOULDBLOCK) && (errno != EAGAIN));
			return -1;
		}
	}

	return (size_t)n == recv_size ? 1 : 2;
}

int XSocket::handle_input_payload(void)
{
	// set errno properly here on error !!!
	// now have a header and payload

	ACE_ASSERT(m_RecvPct.space() == 0);
	ACE_ASSERT(m_Header.space() == 0);
	ACE_ASSERT(m_RecvWPct != nullptr);

	session_->Decrypt(&m_RecvWPct->contents()[0], m_RecvWPct->size());
	session_->ProcessIncoming(m_RecvWPct);

	m_RecvPct.base(nullptr, 0);
	m_RecvPct.reset();

	m_RecvWPct->clear();
	m_RecvWPct = nullptr;

	m_Header.reset();

	return 0;
}

int XSocket::Update(void)
{
	if (closing_)
		return -1;

	if (m_OutActive || (m_OutBuffer->length() == 0 && msg_queue()->is_empty()))
		return 0;

	int ret;
	do
		ret = handle_output(get_handle());
	while (ret > 0);

	return ret;
}

int XSocket::cancel_wakeup_output(GuardType& g)
{
	if (!m_OutActive)
		return 0;

	m_OutActive = false;

	g.release();

	if (reactor()->cancel_wakeup
		(this, ACE_Event_Handler::WRITE_MASK) == -1)
	{
		// would be good to store errno from reactor with errno guard
		MX_LOG_ERROR("network", "XSocket::cancel_wakeup_output");
		return -1;
	}

	return 0;
}

int XSocket::schedule_wakeup_output(GuardType& g)
{
	if (m_OutActive)
		return 0;

	m_OutActive = true;

	g.release();

	if (reactor()->schedule_wakeup
		(this, ACE_Event_Handler::WRITE_MASK) == -1)
	{
		MX_LOG_ERROR("network", "XSocket::schedule_wakeup_output");
		return -1;
	}

	return 0;
}