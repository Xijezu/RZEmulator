#include "EncodingScrambled.h"

bool EncodingScrambled::mapInitialized = false;
uint8_t EncodingScrambled::encodeMap[32];
uint8_t EncodingScrambled::decodeMap[32];

void EncodingScrambled::init() {
	if(mapInitialized == false) {
		mapInitialized = true;

		for(uint8_t i = 0; i < 32; i++)
			encodeMap[i] = i;

		for(uint8_t i = 0, j = 3; i < 32; i++) {
			uint8_t oldValue = encodeMap[i];
			encodeMap[i] = encodeMap[j];
			encodeMap[j] = oldValue;
			j = (j + i + 3) % 32;
		}
		for(uint8_t i = 0; i < 32; i++)
			decodeMap[encodeMap[i]] = i;
	}
}

uint32_t EncodingScrambled::encode(uint32_t v) {
	init();

	uint32_t result = 0;

	for(uint8_t i = 0; i < 32; i++, v >>= 1) {
		if(v & 1)
			result |= 1 << encodeMap[i];
	}

	return result;
}

uint32_t EncodingScrambled::decode(uint32_t v) {
	init();

	uint32_t result = 0;

	for(uint8_t i = 0; i < 32; i++, v >>= 1) {
		if(v & 1)
			result |= 1 << decodeMap[i];
	}

	return result;
}
