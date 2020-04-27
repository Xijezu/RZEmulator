#ifndef PACKETS_ENCODEDINT_H
#define PACKETS_ENCODEDINT_H

#include <stdint.h>

template<class Encoder> class EncodedInt {
public:
	EncodedInt() {}
	EncodedInt(uint32_t v) : value(v) {}
	EncodedInt(const EncodedInt& v) : value(v.value) {}

	EncodedInt operator=(uint32_t v) {
		value = v;
		return *this;
	}
	EncodedInt operator=(EncodedInt v) {
		value = v.value;
		return *this;
	}
	bool operator==(uint32_t v) const { return v == value; }
	bool operator==(EncodedInt v) const { return v.value == value; }

	operator uint32_t() const { return value; }

	uint32_t getSize(int version) const { return Encoder::getSize(version); }

	template<class T> void serialize(T* buffer) const { Encoder::template serialize<T>(buffer, value); }

	template<class T> void deserialize(T* buffer) { Encoder::template deserialize<T>(buffer, value); }

private:
	uint32_t value;
};

#endif  // PACKETS_ENCODEDINT_H
