#ifndef MESSAGESERIALIZERBUFFER_H
#define MESSAGESERIALIZERBUFFER_H

#include "XPacket.h"
#include "StructSerializer.h"
#include <string.h>
#include <type_traits>
#include <vector>

class MessageSerializerBuffer : public StructSerializer {
private:
        XPacket *packet;

public:
        MessageSerializerBuffer(XPacket *packet);
	~MessageSerializerBuffer();

        const XPacket *getPacket() const { return packet; }

        XPacket *getPacket() { return packet; }

        XPacket *getFinalizedPacket()
        {
            packet->FinalizePacket();
		return packet;
	}

	// Write functions /////////////////////////

	void writeHeader(uint32_t size, uint16_t id) {
        packet->Initialize(id, size);
        packet->Reset();
	}

	// Write primitives types T
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type write(const char* fieldName, T val) {
        (*packet) << val;
	}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type write(const char* fieldName, const T& val) {
		(void)fieldName;
		val.serialize(this);
	}

	// String
	void writeString(const char* fieldName, const std::string& val, size_t maxSize);
	void writeDynString(const char* fieldName, const std::string& val, size_t count);

	// Fixed array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type writeArray(const char* fieldName,
	                                                                       const T* val,
	                                                                       size_t size) {
        packet->append<T>(val, size);
	}

	// Fixed array of primitive with cast from U to T
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeArray(const char* fieldName,
	                                                                                   const U* val,
	                                                                                   size_t size) {
		for(size_t i = 0; i < size; ++i) {
			write<T>(fieldName, (T)val[i]);
		}
	}

	// Fixed array of object
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type writeArray(const char* fieldName,
	                                                                        const T* val,
	                                                                        size_t size) {
		for(size_t i = 0; i < size; i++) {
			write<T>(fieldName, val[i]);
		}
	}

	// Dynamic array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type writeDynArray(const char* fieldName,
	                                                                          const std::vector<T>& val,
	                                                                          uint32_t count) {
        packet->append<T>(val.data(), count);
	}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeDynArray(const char* fieldName,
	                                                                                      const std::vector<U>& val,
	                                                                                      uint32_t count) {
		for(size_t i = 0; i < count; ++i) {
			write<T>(fieldName, (T)val[i]);
		}
	}

	// Dynamic array of object or primitive with cast
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type writeDynArray(const char* fieldName,
	                                                                           const std::vector<T>& val,
	                                                                           uint32_t count) {
		for(size_t i = 0; i < count; i++)
			write<T>(fieldName, val[i]);
	}

	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type writeSize(const char* fieldName, T size) {
		write<T>(fieldName, size);
	}

	// Padding, write dummy bytes
	void pad(const char* fieldName, size_t size) {
		for(size_t i = 0; i < size; i++)
            packet->append<uint8>(0);
	}

	// Read functions /////////////////////////

	void readHeader(uint16_t& id) {
        id = packet->GetPacketID();
	}

	// Primitives via arg
	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value, void>::type read(const char* fieldName, U& val) {
        val = packet->read<T>();
	}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type read(const char* fieldName, T& val) {
		val.deserialize(this);
	}

	// String
	void readString(const char* fieldName, std::string& val, size_t size);
	void readDynString(const char* fieldName, std::string& val, uint32_t sizeToRead, bool hasNullTerminator);
	void readEndString(const char* fieldName, std::string& val, bool hasNullTerminator);

	// Fixed array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {
        packet->read(static_cast<uint8 *>(val), size * sizeof(T));
	}

	// Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readArray(const char* fieldName,
	                                                                                  U* val,
	                                                                                  size_t size) {
		for(size_t i = 0; i < size; i++) {
			read<T, U>(fieldName, val[i]);
		}
	}

	// Fixed array of objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {
		for(size_t i = 0; i < size; i++) {
			read<T>(fieldName, val[i]);
		}
	}

	// Dynamic array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                         std::vector<T>& val,
	                                                                         uint32_t sizeToRead) {
		if(sizeToRead > 0) {
			val.resize(sizeToRead);
            packet->read(static_cast<uint8 *>(val.data()), val.size() * sizeof(T));
		} else {
			val.clear();
		}
	}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readDynArray(const char* fieldName,
	                                                                                     std::vector<U>& val,
	                                                                                     uint32_t sizeToRead) {
		val.clear();
		if(sizeToRead > 0) {
			val.reserve(sizeToRead);
			for(size_t i = 0; i < sizeToRead; i++) {
                val.push_back(packet->read<T>());
			}
		}
	}

	// Dynamic array of object
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                          std::vector<T>& val,
	                                                                          uint32_t sizeToRead) {
		val.resize(sizeToRead, T{});

		auto it = val.begin();
		auto itEnd = val.end();
		for(; it != itEnd; ++it)
			read<T>(fieldName, *it);
	}

	// End array, read to the end of stream
	template<typename T> void readEndArray(const char* fieldName, std::vector<T>& val) {
		// While there are non parsed bytes and the read actually read something, continue
		uint32_t lastParsedSize = UINT32_MAX;
        while (lastParsedSize != packet->rpos() && packet->rpos() < packet->size())
        {
            lastParsedSize = packet->rpos();
			auto it = val.insert(val.end(), T());
			T& newItem = *it;
			read<T>(fieldName, newItem);
		}
	}

	// read size for objects (std:: containers)
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readSize(const char* fieldName, uint32_t& val) {
		read<T>(fieldName, val);
	}

	void discard(const char* fieldName, size_t size) {
        packet->read_skip(size);
	}
};

#endif  // MESSAGEBUFFER_H
