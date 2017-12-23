#ifndef SKYFIRE_WORLDPACKET_H
#define SKYFIRE_WORLDPACKET_H

#include "Common.h"
#include "Encryption/ByteBuffer.h"
#include "Server/TS_MESSAGE.h"

class XPacket : public ByteBuffer
{
public:
	// just container for later use
	XPacket() : ByteBuffer(0), m_nPacketID(0)
	{
	}

	XPacket(uint16 packID) : ByteBuffer(0), m_nPacketID(packID)
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
#endif