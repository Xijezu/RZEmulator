#ifndef PACKET_STRUCTSERIALIZER_H
#define PACKET_STRUCTSERIALIZER_H

#include <type_traits>
#include <vector>

class StructSerializer {
private:
	int version;

public:
	StructSerializer(int version) { this->version = version; }

	int getVersion() const { return version; }

	// Type checking /////////////////////////

	// Primitives
	template<typename T>
	struct is_primitive : public std::integral_constant<bool, std::is_fundamental<T>::value || std::is_enum<T>::value> {
	};

	// Primitives with cast
	template<typename T, typename U>
	struct is_castable_primitive : public std::integral_constant<bool,
	                                                             is_primitive<T>::value && is_primitive<U>::value &&
	                                                                 !std::is_same<T, U>::value> {};
};

#endif  // PACKET_STRUCTSERIALIZER_H
