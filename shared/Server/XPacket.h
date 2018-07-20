#pragma once
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
#include "Common.h"
#include "Encryption/ByteBuffer.h"
#include "Server/TS_MESSAGE.h"

class MessageBuffer;

class XPacket : public ByteBuffer
{
public:
	// just container for later use
	XPacket() : ByteBuffer(0), m_nPacketID(0)
	{
	}

	explicit XPacket(uint16 packID) : ByteBuffer(0), m_nPacketID(packID)
	{
		resize(7);
		put(4, packID);
	}

	explicit XPacket(uint16 packID, int32 res, char* encrypted ) : ByteBuffer(res), m_nPacketID(packID) { 
		append(encrypted, 7);
	}
	// copy constructor
	XPacket(const XPacket &packet) : ByteBuffer(packet), m_nPacketID(packet.m_nPacketID)
	{

	}

	explicit XPacket(uint16 packID, MessageBuffer&& buffer) : m_nPacketID(packID), ByteBuffer(std::move(buffer))
	{
	}

	void FinalizePacket() {
		put(0, (uint32)size());
		put(6, (uint8)TS_MESSAGE::GetChecksum(m_nPacketID, size()));
	}
	void Reset() {
		clear();
		resize(7);
		put(4, m_nPacketID);
	}
	void Initialize(uint16 packID, size_t newres = 200)
	{
		clear();
		_storage.reserve(newres);
		m_nPacketID = packID;
	}

	uint16 GetPacketID() const { return m_nPacketID; }
	void SetPacketID(uint16 packID) { m_nPacketID = packID; }
protected:
	uint16 m_nPacketID;
};