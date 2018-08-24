#include "MessageSerializerBuffer.h"
#include <algorithm>
#include "Config.h"

MessageSerializerBuffer::MessageSerializerBuffer() : StructSerializer(sConfigMgr->getCachedConfig().packetVersion) {
}

MessageSerializerBuffer::~MessageSerializerBuffer() {
}

void MessageSerializerBuffer::writeString(const char* fieldName, const std::string& val, size_t maxSize) {
	// keep room for nul terminator (truncate val if too long)
	size_t stringSize = std::min(val.size(), maxSize - 1);
	packet.append(val.c_str(), stringSize);
	for(size_t i = stringSize; i < maxSize; i++)
		packet.append<uint8>(0);
}

void MessageSerializerBuffer::writeDynString(const char* fieldName, const std::string& val, size_t count) {
	packet.append(val.c_str(), count);
}

void MessageSerializerBuffer::readString(const char* fieldName, std::string& val, size_t maxSize) {
	val = packet.ReadString(maxSize);
}

void MessageSerializerBuffer::readDynString(const char* fieldName,
                                  std::string& val,
                                  uint32_t sizeToRead,
                                  bool hasNullTerminator) {
	if(sizeToRead > 0) {
		// don't include the null terminator in std::string, else ::size() will be wrong
		val.resize(sizeToRead - hasNullTerminator);
		packet.read((uint8*)val.data(), val.size());
		if(hasNullTerminator)
			packet.read_skip(1);
	} else {
		val.clear();
	}
}

void MessageSerializerBuffer::readEndString(const char* fieldName, std::string& val, bool hasNullTerminator) {
	size_t remainingSize = packet.size() - packet.rpos();
	if(remainingSize > 0) {
		val.resize(remainingSize - hasNullTerminator);
		packet.read((uint8*)val.data(), val.size());
		if(hasNullTerminator)
			packet.read_skip(1);
	} else
		val.clear();
}
