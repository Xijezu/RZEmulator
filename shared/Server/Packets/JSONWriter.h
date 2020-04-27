#ifndef PACKETS_JSONWRITER_H
#define PACKETS_JSONWRITER_H

#include "EncodedInt.h"
#include "StructSerializer.h"
#include <sstream>

class JSONWriter : public StructSerializer {
private:
	std::stringstream json;
	int depth;
	bool newList;
	const bool compact;

private:
	const char* getEndFieldSeparator() { return compact ? "\":" : "\": "; }

public:
	JSONWriter(int version, bool compact) : StructSerializer(version), depth(1), newList(true), compact(compact) {
		json << "{";
	}

	void finalize() {
		json << (compact ? "}\n" : "\n}\n");
		newList = true;
	}
	void start() { json << "{"; }
	void clear() {
		json.str(std::string());
		json.clear();
	}

	std::string toString() { return json.str(); }

	uint32_t getParsedSize() const { return 0; }

	void printIdent(bool addNewLine = true) {
		if(!newList)
			json << (compact ? "," : ", ");

		if(addNewLine && !compact) {
			json << "\n";
			for(int i = 0; i < depth; i++)
				json << '\t';
		}

		newList = false;
	}

	void writeEncodedString(const char* source) {
		char newString[2048];
		size_t currentSize = 0;

		for(size_t i = 0; currentSize < sizeof(newString) - 1; i++) {
			const unsigned char c = source[i];

			// Normal visible chars
			if(c >= 0x20 && c < 0x7F && c != '\"' && c != '\\') {
				newString[currentSize++] = c;

			} else if(c == '\"' && c == '\\') {
				newString[currentSize++] = '\\';
				newString[currentSize++] = c;
			} else if(c == '\0') {
				break;
			} else {
				int ret = snprintf(&newString[currentSize], sizeof(newString) - currentSize, "\\u%04x", c);
				if(ret >= 0)
					currentSize += ret;
			}
		}

		newString[currentSize] = '\0';

		json << '\"' << newString << '\"';
	}

	// Write functions /////////////////////////

	void writeHeader(uint32_t /* size */, uint16_t id) { write<uint16_t>("id", id); }

	// handle ambiguous << operator for int16_t and int8_t
	template<typename T>
	typename std::enable_if<is_primitive<T>::value && sizeof(T) < sizeof(int), void>::type write(const char* fieldName,
	                                                                                             T val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << (int) val;
	}

	// Primitives
	template<typename T>
	typename std::enable_if<is_primitive<T>::value && sizeof(T) >= sizeof(int), void>::type write(const char* fieldName,
	                                                                                              T val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << val;
	}

	// Encoded values
	template<typename T> void write(const char* fieldName, const EncodedInt<T>& val) {
		printIdent(fieldName != nullptr);
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << (uint32_t) val;
	}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type write(const char* fieldName, const T& val) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '{';

		newList = true;
		depth++;
		val.serialize(this);
		depth--;

		newList = true;
		printIdent();
		json << '}';
	}

	// String
	void writeString(const char* fieldName, const std::string& val, size_t /* maxSize */) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		writeEncodedString(val.c_str());
	}

	void writeDynString(const char* fieldName, const std::string& val, size_t /* count */) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		writeEncodedString(val.c_str());
	}

	void writeArray(const char* fieldName, const char* val, size_t /* size */) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		writeEncodedString(val);
	}

	// Fixed array
	template<typename T> void writeArray(const char* fieldName, const T* val, size_t size) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '[';

		newList = true;
		depth++;
		for(size_t i = 0; i < size; i++) {
			write(nullptr, val[i]);
		}
		depth--;

		newList = true;
		printIdent(false);
		json << ']';
	}

	// Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeArray(const char* fieldName,
	                                                                                   const U* val,
	                                                                                   size_t size) {
		writeArray<U>(fieldName, val, size);
	}

	// Dynamic array
	template<typename T> void writeDynArray(const char* fieldName, const std::vector<T>& val, uint32_t) {
		printIdent();
		if(fieldName)
			json << '\"' << fieldName << getEndFieldSeparator();
		json << '[';

		newList = true;
		depth++;
		auto it = val.begin();
		auto itEnd = val.end();
		for(; it != itEnd; ++it)
			write(nullptr, *it);
		depth--;

		newList = true;
		printIdent(false);
		json << ']';
	}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type writeDynArray(const char* fieldName,
	                                                                                      const std::vector<U>& val,
	                                                                                      uint32_t count) {
		writeDynArray<U>(fieldName, val, count);
	}

	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type writeSize(const char* /* fieldName */, T /* size */) {}

	void pad(const char* /* fieldName */, size_t /* size */) {}

	// Read functions /////////////////////////

	void readHeader(uint16_t& /* id */) {}

	// Primitives via arg
	template<typename T, typename U>
	typename std::enable_if<is_primitive<U>::value, void>::type read(const char* fieldName, U& val) {}

	// Objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type read(const char* fieldName, T& val) {}

	// String
	void readString(const char* fieldName, std::string& val, size_t size) {}
	void readDynString(const char* fieldName, std::string& val, size_t sizeToRead, bool hasNullTerminator) {}
	void readEndString(const char* fieldName, std::string& val, bool hasNullTerminator) {}

	// Fixed array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {}

	// Fixed array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readArray(const char* fieldName,
	                                                                                  U* val,
	                                                                                  size_t size) {}

	// Fixed array of objects
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readArray(const char* fieldName, T* val, size_t size) {
	}

	// Dynamic array of primitive
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                         std::vector<T>& val) {}

	// Dynamic array of primitive with cast
	template<typename T, typename U>
	typename std::enable_if<is_castable_primitive<T, U>::value, void>::type readDynArray(const char* fieldName,
	                                                                                     std::vector<U>& val) {}

	// Dynamic array of object
	template<typename T>
	typename std::enable_if<!is_primitive<T>::value, void>::type readDynArray(const char* fieldName,
	                                                                          std::vector<T>& val) {}

	// End array, read to the end of stream
	template<typename T> void readEndArray(const char* fieldName, std::vector<T>& val) {}

	// read size for objects (std:: containers)
	template<typename T>
	typename std::enable_if<is_primitive<T>::value, void>::type readSize(const char* fieldName, T& val) {}

	void discard(const char* fieldName, size_t size) {}
};

#endif /* PACKETS_JSONWRITER_H */
