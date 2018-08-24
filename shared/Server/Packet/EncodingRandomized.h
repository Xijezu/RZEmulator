#ifndef PACKETS_ENCODINGRANDOMIZED_H
#define PACKETS_ENCODINGRANDOMIZED_H

#include <stdint.h>

class EncodingRandomized {
private:
	union SerializedValue {
		uint64_t u64;
		uint16_t u16[4];
	};

public:
	static uint32_t getSize(int version) { return sizeof(uint64_t); }

	template<class T> static void serialize(T* buffer, uint32_t value) {
		SerializedValue sv;
		sv.u16[0] = 0;
		sv.u16[1] = value >> 16;
		sv.u16[2] = 0;
		sv.u16[3] = value & 0xFFFF;

		/* True implementation:
		 *
		 * sv.u16[0] = rand();
		 * sv.u16[2] = rand();
		 * sv.u16[1] = (value >> 16)    + 2*(sv.u16[2] - sv.u16[0]);
		 * sv.u16[3] = (value & 0xFFFF) - 2*(sv.u16[2] + sv.u16[0]);
		 */

		buffer->template write<uint64_t>("value", sv.u64);
	}

	template<class T> static void deserialize(T* buffer, uint32_t& value) {
		SerializedValue sv;
		buffer->template read<uint64_t>("value", sv.u64);

		value = (uint16_t(sv.u16[1] - 2 * (sv.u16[2] - sv.u16[0])) << 16) |
		        uint16_t(sv.u16[3] + 2 * (sv.u16[2] + sv.u16[0]));
	}

private:
	EncodingRandomized();
};

#endif  // PACKETS_ENCODINGRANDOMIZED_H
