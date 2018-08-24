#ifndef PACKETS_ENCODINGSCRAMBLED_H
#define PACKETS_ENCODINGSCRAMBLED_H

#include "EncodingRandomized.h"

class EncodingScrambled {
public:
	static uint32_t getSize(int version) { return EncodingRandomized::getSize(version); }

	template<class T> static void serialize(T* buffer, uint32_t value) {
		EncodingRandomized::serialize<T>(buffer, encode(value));
	}

	template<class T> static void deserialize(T* buffer, uint32_t& value) {
		uint32_t deserializedValue;
		EncodingRandomized::deserialize<T>(buffer, deserializedValue);

		value = decode(deserializedValue);
	}

private:
	EncodingScrambled();

	static void init();
	static uint32_t encode(uint32_t v);
	static uint32_t decode(uint32_t v);

	static bool mapInitialized;
	static uint8_t encodeMap[32];
	static uint8_t decodeMap[32];
};

#endif  // PACKETS_ENCODINGSCRAMBLED_H
