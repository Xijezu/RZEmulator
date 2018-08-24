#ifndef PACKETDECLARATION_H
#define PACKETDECLARATION_H

// To see the generated C++ code, use:
// gcc -E TS_*.h -DDEBUG_PREPROCESSOR -I../../lib | clang-format-6.0 -style llvm -

#ifndef DEBUG_PREPROCESSOR
#include "PacketEpics.h"
#include <limits>
#include <stdint.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <vector>
#endif

#define _ARG5(_0, _1, _2, _3, _4, _5, ...) _5
#define _EXPAND(x) x
#define NARG5(...) _EXPAND(_ARG5(__VA_ARGS__, 5, 4, 3, 2, 1, 0))

#define __ONE_OR_TWO_ARGS(MACRO, N, ...) _EXPAND(MACRO##N(__VA_ARGS__))
#define _ONE_OR_TWO_ARGS(MACRO, N, ...) __ONE_OR_TWO_ARGS(MACRO, N, __VA_ARGS__)
#define OVERLOADED_CALL(macro, ...) _ONE_OR_TWO_ARGS(macro, NARG5(__VA_ARGS__), __VA_ARGS__)

#ifndef DEBUG_PREPROCESSOR
namespace PacketDeclaration {

// These functions allow to handle differents types the same way

/**
 * @brief Copy value val2 to val1.
 * This overload is used for non array types.
 */
template<typename T, typename U> inline void copyDefaultValue(T& val1, U val2) {
	val1 = val2;
}

/**
 * @brief Copy value val2 to val1.
 * This overload is used for fixed array of POD types.
 */
template<typename T> inline void copyDefaultValue(T val1[], const T val2[], size_t size) {
	memcpy(val1, val2, size);
}

/**
 * @brief Copy value val2 to val1.
 * This overload is used for vectors.
 */
template<typename T> inline void copyDefaultValue(std::vector<T> val1, const T val2[], size_t size) {
	val1.assign(val2, val2 + size);
}

/**
 * @brief Return the size of an object given the version.
 * This calls the getSize function of the object.
 * The object need to be declared with CREATE_STRUCT / CREATE_PACKET or the likes.
 */
template<class T>
inline typename std::enable_if<!std::is_fundamental<T>::value && !std::is_enum<T>::value, int>::type getSizeOf(
    const T& value, int version) {
	return value.getSize(version);
}

/**
 * @brief Return the size of an basic type or enum.
 */
template<typename T>
inline typename std::enable_if<std::is_fundamental<T>::value || std::is_enum<T>::value, int>::type getSizeOf(
    const T& value, int version) {
	return sizeof(value);
}

/**
 * @brief Return a clamped size for the given type T.
 * For example, if the type T is a uint8_t, the maximum size the type can store is 255.
 * @return the maximum supported size that the type T can contains or realSize if it is smaller.
 */
template<typename T> uint32_t getClampedCount(size_t realSize) {
	static_assert(sizeof(T) <= sizeof(uint32_t),
	              "Maximum supported size is uint32_t because a packet is limited to 16k bytes");
	size_t maxSize = (size_t) std::numeric_limits<T>::max();
	return (uint32_t)(realSize < maxSize ? realSize : maxSize);
}

}  // namespace PacketDeclaration
#endif

/** @defgroup dispatch Dispatch macros to metatype macros
 *  These macros will will call the matching metatype macro
 *  like DEFINITION_F_simple().
 *  \see definitions
 *  \see local_definitions
 *  \see sizes
 *  \see serializations
 *  \see deserializations
 *  @{
 */
/** @def Dispatch declarations to \ref definitions. */
#define DEFINITION_F(x) DEFINITION_F_##x
/** @def Dispatch declarations to \ref local_definitions. */
#define LOCAL_DEFINITION_F(x) LOCAL_DEFINITION_F_##x
/** @def Dispatch declarations to \ref sizes. */
#define SIZE_F(x) SIZE_F_##x
/** @def Dispatch declarations to \ref serializations. */
#define SERIALIZATION_F(x) SERIALIZATION_F_##x
/** @def Dispatch declarations to \ref deserializations. */
#define DESERIALIZATION_F(x) DESERIALIZATION_F_##x
///@}

/** @defgroup definitions Body fields metatype macros
 * These macros define the fields of the generated structs given the metatype of the field.
 * They are formated like this: DEFINITION_F_<metatype>(...).
 * They are called from #DEFINITION_F.
 * The arguments come from the packet definition.
 * For example, `_(simple)(uint8_t, n)` will cause a call to `DEFINITION_F_simple(uint8_t, n)`.
 *  @{
 */
/** @brief Generate a simple field. For example: `uint8_t n;` */
#define DEFINITION_F_simple(type, name, ...) type name;

/** @brief Generate a fixed array field. For example: `uint8_t n[5];` */
#define DEFINITION_F_array(type, name, size, ...) type name[size];

/** @brief Generate a dynamic array field. For example: `std::vector<uint8_t> n;`
 * Also generate a metadata struct containing an enum (this will take no place in the generated struct).
 * The enum allows dynamically sized fields to behave generically like `dynstring` metatype for the `count` metatype.
 */
#define DEFINITION_F_dynarray(type, name, ...) \
	std::vector<type> name; \
	struct _metadata_##name { \
		enum { addNullTerminator = 0 }; \
	};

/** @brief No field is generated for a `count` metatype.
 * The underlying value is read from `ref.size()`
 * and written as `ref.resize()` instead.
 */
#define DEFINITION_F_count(type, ref, ...)

/** @brief Generate a fixed string field. For example: `std::string n;`
 * The generated field is not a char n[size] as std::string is easier to handle
 * and abstract the difference with a dynamic sized string.
 */
#define DEFINITION_F_string(name, size, ...) std::string name;

/** @brief Generate a fixed string field. For example: `std::string n;`.
 * Also generate a metadata struct containing an enum (this will take no place in the generated struct).
 * The enum allows dynamically sized fields to behave generically like `dynarray` metatype for the `count` metatype.
 * The enum indicate if the serialized string also contains the null terminator (\0) or not.
 */
#define DEFINITION_F_dynstring(name, hasNullTerminator, ...) \
	std::string name; \
	struct _metadata_##name { \
		enum { addNullTerminator = !!hasNullTerminator }; \
	};

/** @brief No field is generated for a `padmarker` metatype. */
#define DEFINITION_F_padmarker(...)

/** @brief No field is generated for a `pad` metatype. */
#define DEFINITION_F_pad(...)

/** @brief Generate a string field. For example: `std::string n;`. */
#define DEFINITION_F_endstring(name, hasNullTerminator, ...) std::string name;

/** @brief Generate a dynamic array field. For example: `std::vector<uint8_t> n;`. */
#define DEFINITION_F_endarray(type, name, ...) std::vector<type> name;
/** @} */

// Local fields
#define LOCAL_DEFINITION_F_simple(type, name, ...)
#define LOCAL_DEFINITION_F_array(type, name, size, ...)
#define LOCAL_DEFINITION_F_dynarray(type, name, ...)
#define LOCAL_DEFINITION_F_count(type, ref, ...) \
	uint32_t ref##_size = 0; \
	(void) (ref##_size);
#define LOCAL_DEFINITION_F_string(name, size, ...)
#define LOCAL_DEFINITION_F_dynstring(name, hasNullTerminator, ...)
#define LOCAL_DEFINITION_F_padmarker(...)
#define LOCAL_DEFINITION_F_pad(...)
#define LOCAL_DEFINITION_F_endstring(name, hasNullTerminator, ...)
#define LOCAL_DEFINITION_F_endarray(type, name, ...)

// Size function
#define SIZE_F_simple(...) OVERLOADED_CALL(SIZE_F_SIMPLE, __VA_ARGS__)
#define SIZE_F_array(...) OVERLOADED_CALL(SIZE_F_ARRAY, __VA_ARGS__)
#define SIZE_F_dynarray(...) OVERLOADED_CALL(SIZE_F_DYNARRAY, __VA_ARGS__)
#define SIZE_F_count(...) OVERLOADED_CALL(SIZE_F_COUNT, __VA_ARGS__)
#define SIZE_F_string(...) OVERLOADED_CALL(SIZE_F_STRING, __VA_ARGS__)
#define SIZE_F_dynstring(...) OVERLOADED_CALL(SIZE_F_DYNSTRING, __VA_ARGS__)
#define SIZE_F_padmarker(...) OVERLOADED_CALL(SIZE_F_PADMARKER, __VA_ARGS__)
#define SIZE_F_pad(...) OVERLOADED_CALL(SIZE_F_PAD, __VA_ARGS__)
#define SIZE_F_endstring(...) OVERLOADED_CALL(SIZE_F_ENDSTRING, __VA_ARGS__)
#define SIZE_F_endarray(...) OVERLOADED_CALL(SIZE_F_ENDARRAY, __VA_ARGS__)

#define SIZE_F_SIMPLE2(type, name) size += PacketDeclaration::getSizeOf((type) name, version);
#define SIZE_F_SIMPLE3(type, name, cond) \
	if(cond) \
		size += PacketDeclaration::getSizeOf((type) name, version);
#define SIZE_F_SIMPLE4(type, name, cond, defaultval) \
	if(cond) \
		size += PacketDeclaration::getSizeOf((type) name, version);

#define SIZE_F_ARRAY3(type, name, _size) \
	for(int i = 0; i < _size; ++i) \
		size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_ARRAY4(type, name, _size, cond) \
	if(cond) \
		for(int i = 0; i < _size; ++i) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_ARRAY5(type, name, _size, cond, defaultval) \
	if(cond) \
		for(int i = 0; i < _size; ++i) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);

#define SIZE_F_DYNARRAY2(type, name) \
	for(size_t i = 0; i < name##_size; i++) \
		size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_DYNARRAY3(type, name, cond) \
	if(cond) \
		for(size_t i = 0; i < name##_size; i++) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_DYNARRAY4(type, name, cond, defaultval) \
	if(cond) \
		for(size_t i = 0; i < name##_size; i++) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);

#define SIZE_F_COUNT2(type, ref) \
	ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
	size += sizeof(type);
#define SIZE_F_COUNT3(type, ref, cond) \
	if(cond) { \
		ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
		size += sizeof(type); \
	}
#define SIZE_F_COUNT4(type, ref, cond, defaultval) \
	if(cond) { \
		ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
		size += sizeof(type); \
	} else { \
		ref##_size = defaultval; \
	}

#define SIZE_F_STRING2(name, _size) size += _size;
#define SIZE_F_STRING3(name, _size, cond) \
	if(cond) \
		size += _size;
#define SIZE_F_STRING4(name, _size, cond, defaultval) \
	if(cond) \
		size += _size;

#define SIZE_F_DYNSTRING2(name, hasNullTerminator) size += name##_size;
#define SIZE_F_DYNSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		size += name##_size;
#define SIZE_F_DYNSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		size += name##_size;

#define SIZE_F_PADMARKER1(marker) const uint32_t marker = size;
#define SIZE_F_PAD2(_size, marker) \
	if(size < marker + (_size)) \
		size = marker + (_size);
#define SIZE_F_PAD3(_size, marker, cond) \
	if(cond && size < marker + (_size)) \
		size = marker + (_size);

#define SIZE_F_ENDSTRING2(name, hasNullTerminator) size += (uint32_t) name.size();
#define SIZE_F_ENDSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		size += (uint32_t) name.size();
#define SIZE_F_ENDSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		size += (uint32_t) name.size();

#define SIZE_F_ENDARRAY2(type, name) \
	for(size_t i = 0; i < name.size(); i++) \
		size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_ENDARRAY3(type, name, cond) \
	if(cond) \
		for(size_t i = 0; i < name.size(); i++) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);
#define SIZE_F_ENDARRAY4(type, name, cond, defaultval) \
	if(cond) \
		for(size_t i = 0; i < name.size(); i++) \
			size += PacketDeclaration::getSizeOf((type) name[i], version);

// Serialization function
#define SERIALIZATION_F_simple(...) OVERLOADED_CALL(SERIALIZATION_F_SIMPLE, __VA_ARGS__)
#define SERIALIZATION_F_array(...) OVERLOADED_CALL(SERIALIZATION_F_ARRAY, __VA_ARGS__)
#define SERIALIZATION_F_dynarray(...) OVERLOADED_CALL(SERIALIZATION_F_DYNARRAY, __VA_ARGS__)
#define SERIALIZATION_F_count(...) OVERLOADED_CALL(SERIALIZATION_F_COUNT, __VA_ARGS__)
#define SERIALIZATION_F_string(...) OVERLOADED_CALL(SERIALIZATION_F_STRING, __VA_ARGS__)
#define SERIALIZATION_F_dynstring(...) OVERLOADED_CALL(SERIALIZATION_F_DYNSTRING, __VA_ARGS__)
#define SERIALIZATION_F_padmarker(...) OVERLOADED_CALL(SERIALIZATION_F_PADMARKER, __VA_ARGS__)
#define SERIALIZATION_F_pad(...) OVERLOADED_CALL(SERIALIZATION_F_PAD, __VA_ARGS__)
#define SERIALIZATION_F_endstring(...) OVERLOADED_CALL(SERIALIZATION_F_ENDSTRING, __VA_ARGS__)
#define SERIALIZATION_F_endarray(...) OVERLOADED_CALL(SERIALIZATION_F_ENDARRAY, __VA_ARGS__)

#define SERIALIZATION_F_SIMPLE2(type, name) buffer->write(#name, (type) name);
#define SERIALIZATION_F_SIMPLE3(type, name, cond) \
	if(cond) \
		buffer->write(#name, (type) name);
#define SERIALIZATION_F_SIMPLE4(type, name, cond, defaultval) \
	if(cond) \
		buffer->write(#name, (type) name);

#define SERIALIZATION_F_ARRAY3(type, name, size) buffer->template writeArray<type>(#name, name, size);
#define SERIALIZATION_F_ARRAY4(type, name, size, cond) \
	if(cond) \
		buffer->template writeArray<type>(#name, name, size);
#define SERIALIZATION_F_ARRAY5(type, name, size, cond, defaultval) \
	if(cond) \
		buffer->template writeArray<type>(#name, name, size);

#define SERIALIZATION_F_DYNARRAY2(type, name) buffer->template writeDynArray<type>(#name, name, name##_size);
#define SERIALIZATION_F_DYNARRAY3(type, name, cond) \
	if(cond) \
		buffer->template writeDynArray<type>(#name, name, name##_size);
#define SERIALIZATION_F_DYNARRAY4(type, name, cond, defaultval) \
	if(cond) \
		buffer->template writeDynArray<type>(#name, name, name##_size);

#define SERIALIZATION_F_COUNT2(type, ref) \
	ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
	buffer->writeSize(#ref, (type) ref##_size);
#define SERIALIZATION_F_COUNT3(type, ref, cond) \
	if(cond) { \
		ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
		buffer->writeSize(#ref, (type) ref##_size); \
	}
#define SERIALIZATION_F_COUNT4(type, ref, cond, defaultval) \
	if(cond) { \
		ref##_size = PacketDeclaration::getClampedCount<type>(ref.size() + _metadata_##ref::addNullTerminator); \
		buffer->writeSize(#ref, (type) ref##_size); \
	} else { \
		ref##_size = defaultval; \
	}

#define SERIALIZATION_F_STRING2(name, size) buffer->writeString(#name, name, size);
#define SERIALIZATION_F_STRING3(name, size, cond) \
	if(cond) \
		buffer->writeString(#name, name, size);
#define SERIALIZATION_F_STRING4(name, size, cond, defaultval) \
	if(cond) \
		buffer->writeString(#name, name, size);

#define SERIALIZATION_F_DYNSTRING2(name, hasNullTerminator) buffer->writeDynString(#name, name, name##_size);
#define SERIALIZATION_F_DYNSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		buffer->writeDynString(#name, name, name##_size);
#define SERIALIZATION_F_DYNSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		buffer->writeDynString(#name, name, name##_size);

#define SERIALIZATION_F_PADMARKER1(marker) const uint32_t marker = buffer->getParsedSize();
#define SERIALIZATION_F_PAD2(_size, marker) \
	if(buffer->getParsedSize() < marker + (_size)) \
		buffer->pad("pad_" #marker, marker + (_size) -buffer->getParsedSize());
#define SERIALIZATION_F_PAD3(_size, marker, cond) \
	if(cond && buffer->getParsedSize() < marker + (_size)) \
		buffer->pad("pad_" #marker, marker + (_size) -buffer->getParsedSize());

#define SERIALIZATION_F_ENDSTRING2(name, hasNullTerminator) buffer->writeDynString(#name, name, (uint32_t) name.size());
#define SERIALIZATION_F_ENDSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		buffer->writeDynString(#name, name, (uint32_t) name.size());
#define SERIALIZATION_F_ENDSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		buffer->writeDynString(#name, name, (uint32_t) name.size());

#define SERIALIZATION_F_ENDARRAY2(type, name) buffer->template writeDynArray<type>(#name, name, (uint32_t) name.size());
#define SERIALIZATION_F_ENDARRAY3(type, name, cond) \
	if(cond) \
		buffer->template writeDynArray<type>(#name, name, (uint32_t) name.size());
#define SERIALIZATION_F_ENDARRAY4(type, name, cond, defaultval) \
	if(cond) \
		buffer->template writeDynArray<type>(#name, name, (uint32_t) name.size());

// Deserialization function
#define DESERIALIZATION_F_simple(...) OVERLOADED_CALL(DESERIALIZATION_F_SIMPLE, __VA_ARGS__)
#define DESERIALIZATION_F_array(...) OVERLOADED_CALL(DESERIALIZATION_F_ARRAY, __VA_ARGS__)
#define DESERIALIZATION_F_dynarray(...) OVERLOADED_CALL(DESERIALIZATION_F_DYNARRAY, __VA_ARGS__)
#define DESERIALIZATION_F_count(...) OVERLOADED_CALL(DESERIALIZATION_F_COUNT, __VA_ARGS__)
#define DESERIALIZATION_F_string(...) OVERLOADED_CALL(DESERIALIZATION_F_STRING, __VA_ARGS__)
#define DESERIALIZATION_F_dynstring(...) OVERLOADED_CALL(DESERIALIZATION_F_DYNSTRING, __VA_ARGS__)
#define DESERIALIZATION_F_pad(...) OVERLOADED_CALL(DESERIALIZATION_F_PAD, __VA_ARGS__)
#define DESERIALIZATION_F_padmarker(...) OVERLOADED_CALL(DESERIALIZATION_F_PADMARKER, __VA_ARGS__)
#define DESERIALIZATION_F_endstring(...) OVERLOADED_CALL(DESERIALIZATION_F_ENDSTRING, __VA_ARGS__)
#define DESERIALIZATION_F_endarray(...) OVERLOADED_CALL(DESERIALIZATION_F_ENDARRAY, __VA_ARGS__)

#define DESERIALIZATION_F_SIMPLE2(type, name) buffer->template read<type>(#name, name);
#define DESERIALIZATION_F_SIMPLE3(type, name, cond) \
	if(cond) \
		buffer->template read<type>(#name, name);
#define DESERIALIZATION_F_SIMPLE4(type, name, cond, defaultval) \
	if(cond) \
		buffer->template read<type>(#name, name); \
	else { \
		static const type defaultValue = defaultval; \
		PacketDeclaration::copyDefaultValue(name, defaultValue); \
	}

#define DESERIALIZATION_F_ARRAY3(type, name, size) buffer->template readArray<type>(#name, name, size);
#define DESERIALIZATION_F_ARRAY4(type, name, size, cond) \
	if(cond) \
		buffer->template readArray<type>(#name, name, size);
#define DESERIALIZATION_F_ARRAY5(type, name, size, cond, defaultval) \
	if(cond) \
		buffer->template readArray<type>(#name, name, size); \
	else { \
		static const type defaultArray[size] = defaultval; \
		PacketDeclaration::copyDefaultValue(name, defaultArray, size); \
	}

#define DESERIALIZATION_F_DYNARRAY2(type, name) buffer->template readDynArray<type>(#name, name, name##_size);
#define DESERIALIZATION_F_DYNARRAY3(type, name, cond) \
	if(cond) \
		buffer->template readDynArray<type>(#name, name, name##_size);
#define DESERIALIZATION_F_DYNARRAY4(type, name, cond, defaultval) \
	if(cond) \
		buffer->template readDynArray<type>(#name, name, name##_size); \
	else { \
		static const type defaultArray[] = defaultval; \
		PacketDeclaration::copyDefaultValue(name, defaultArray, sizeof(defaultArray) / sizeof(type)); \
	}

#define DESERIALIZATION_F_COUNT2(type, ref) buffer->template readSize<type>(#ref, ref##_size);
#define DESERIALIZATION_F_COUNT3(type, ref, cond) \
	if(cond) \
		buffer->template readSize<type>(#ref, ref##_size);
#define DESERIALIZATION_F_COUNT4(type, ref, cond, defaultval) \
	if(cond) \
		buffer->template readSize<type>(#ref, ref##_size); \
	else \
		ref##_size = defaultval;

#define DESERIALIZATION_F_STRING2(name, size) buffer->readString(#name, name, size);
#define DESERIALIZATION_F_STRING3(name, size, cond) \
	if(cond) \
		buffer->readString(#name, name, size);
#define DESERIALIZATION_F_STRING4(name, size, cond, defaultval) \
	if(cond) \
		buffer->readString(#name, name, size); \
	else \
		name = defaultval;

#define DESERIALIZATION_F_DYNSTRING2(name, hasNullTerminator) \
	buffer->readDynString(#name, name, name##_size, hasNullTerminator);
#define DESERIALIZATION_F_DYNSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		buffer->readDynString(#name, name, name##_size, hasNullTerminator);
#define DESERIALIZATION_F_DYNSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		buffer->readDynString(#name, name, name##_size, hasNullTerminator); \
	else \
		name = defaultval;

#define DESERIALIZATION_F_PADMARKER1(marker) const uint32_t marker = buffer->getParsedSize();
#define DESERIALIZATION_F_PAD2(_size, marker) \
	if(buffer->getParsedSize() < marker + (_size)) \
		buffer->discard("pad_" #marker, marker + (_size) -buffer->getParsedSize());
#define DESERIALIZATION_F_PAD3(_size, marker, cond) \
	if(cond && buffer->getParsedSize() < marker + (_size)) \
		buffer->discard("pad_" #marker, marker + (_size) -buffer->getParsedSize());

#define DESERIALIZATION_F_ENDSTRING2(name, hasNullTerminator) buffer->readEndString(#name, name, hasNullTerminator);
#define DESERIALIZATION_F_ENDSTRING3(name, hasNullTerminator, cond) \
	if(cond) \
		buffer->readEndString(#name, name, hasNullTerminator);
#define DESERIALIZATION_F_ENDSTRING4(name, hasNullTerminator, cond, defaultval) \
	if(cond) \
		buffer->readEndString(#name, name, hasNullTerminator); \
	else \
		name = defaultval;

#define DESERIALIZATION_F_ENDARRAY2(type, name) buffer->template readEndArray<type>(#name, name);
#define DESERIALIZATION_F_ENDARRAY3(type, name, cond) \
	if(cond) \
		buffer->template readEndArray<type>(#name, name);
#define DESERIALIZATION_F_ENDARRAY4(type, name, cond, defaultval) \
	if(cond) \
		buffer->template readEndArray<type>(#name, name); \
	else { \
		static const type defaultArray[] = defaultval; \
		PacketDeclaration::copyDefaultValue(name, defaultArray, sizeof(defaultArray) / sizeof(type)); \
	}

// def / impl mode implementation
#define DO_NOTHING(...)

#define DEFINITION_F_def(x) DEFINITION_F_##x
#define LOCAL_DEFINITION_F_def(x) LOCAL_DEFINITION_F_##x
#define SIZE_F_def(x) DO_NOTHING
#define SERIALIZATION_F_def(x) DO_NOTHING
#define DESERIALIZATION_F_def(x) DO_NOTHING

#define DEFINITION_F_impl(x) DO_NOTHING
#define LOCAL_DEFINITION_F_impl(x) DO_NOTHING
#define SIZE_F_impl(x) SIZE_F_##x
#define SERIALIZATION_F_impl(x) SERIALIZATION_F_##x
#define DESERIALIZATION_F_impl(x) DESERIALIZATION_F_##x

#define CREATE_STRUCT_IMPL(name_, size_base_, definition_header_, serialization_header_, deserialization_header_) \
	struct name_ { \
		static inline const char* getName() { return #name_; } \
		definition_header_; \
		name_##_DEF(DEFINITION_F); \
		uint32_t getSize(int version) const { \
			uint32_t size = size_base_; \
			(void) (version); \
			name_##_DEF(LOCAL_DEFINITION_F); \
			name_##_DEF(SIZE_F); \
			return size; \
		} \
		template<class T> void serialize(T* buffer) const { \
			const int version = buffer->getVersion(); \
			(void) (version); \
			serialization_header_; \
			name_##_DEF(LOCAL_DEFINITION_F); \
			name_##_DEF(SERIALIZATION_F); \
		} \
		template<class T> void deserialize(T* buffer) { \
			const int version = buffer->getVersion(); \
			(void) (version); \
			deserialization_header_; \
			name_##_DEF(LOCAL_DEFINITION_F); \
			name_##_DEF(DESERIALIZATION_F); \
		} \
	}

#define CREATE_STRUCT(name_) CREATE_STRUCT_IMPL(name_, 0, /* empty */, /* empty */, /* empty */)

#define CREATE_PACKET_DEFINITION_HEADER(id_) \
	static const uint16_t packetID = id_; \
	static inline uint16_t getId(int version) { \
		(void) version; \
		return id_; \
	} \
	uint16_t id;

#define CREATE_PACKET_SERIALIZATION_HEADER \
	uint32_t size = getSize(buffer->getVersion()); \
	buffer->writeHeader(size, packetID);

#define CREATE_PACKET_DESERIALIZATION_HEADER buffer->readHeader(id);

#define CREATE_PACKET(name_, id_) \
	CREATE_STRUCT_IMPL(name_, \
	                   7, \
	                   CREATE_PACKET_DEFINITION_HEADER(id_), \
	                   CREATE_PACKET_SERIALIZATION_HEADER, \
	                   CREATE_PACKET_DESERIALIZATION_HEADER)

#define HEADER_F_ID(id_, ...) static const uint16_t packetID_##id_ = id_;

#define SERIALISATION_F_ID(...) OVERLOADED_CALL(SERIALISATION_F_ID, __VA_ARGS__)
#define SERIALISATION_F_ID1(id_) id = id_;
#define SERIALISATION_F_ID2(id_, condition_) \
	if(condition_) \
		id = id_;

#define CREATE_PACKET_VER_ID_HEADER_HEADER(name_) \
	name_##_ID(HEADER_F_ID); \
	static inline uint16_t getId(int version) { \
		uint16_t id; \
		(void) version; \
		name_##_ID(SERIALISATION_F_ID); \
		return id; \
	}; \
	uint16_t id;

#define CREATE_PACKET_VER_ID_SERIALIZATION_HEADER(name_) \
	uint32_t size = getSize(version); \
	uint16_t id = getId(version); \
	buffer->writeHeader(size, id);

#define CREATE_PACKET_VER_ID_DESERIALIZATION_HEADER buffer->readHeader(id);

#define CREATE_PACKET_VER_ID(name_) \
	CREATE_STRUCT_IMPL(name_, \
	                   7, \
	                   CREATE_PACKET_VER_ID_HEADER_HEADER(name_), \
	                   CREATE_PACKET_VER_ID_SERIALIZATION_HEADER(name_), \
	                   CREATE_PACKET_VER_ID_DESERIALIZATION_HEADER)

#define CREATE_PACKET_VER_ID_SWITCH_CASES_2(id_, ...) packetID_##id_:

#define CREATE_PACKET_VER_ID_SWITCH_CASES(name_) case name_::CREATE_PACKET_VER_ID_SWITCH_CASES_2

#define case_packet_is(name_) name_##_ID(CREATE_PACKET_VER_ID_SWITCH_CASES(name_))

#endif  // PACKETDECLARATION_H
