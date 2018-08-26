# Table of Contents

   * [Packet description format](#packet-description-format)
      * [Plain structures](#plain-structures)
      * [Packet declarative descriptions](#packet-declarative-descriptions)
         * [Basic packet](#basic-packet)
         * [More complex packet](#more-complex-packet)
      * [Field description reference](#field-description-reference)
         * [Field metatypes](#field-metatypes)
         * [`(def)`/`(impl)` indicators](#defimpl-indicators)
         * [Complex generated code](#complex-generated-code)

# Packet description format
This folder contains packets descriptions in two different format:
 - Plain structures
 - Packet declarative description allowing serialization and de-serialization

## Plain structures
Packets defined with plain structures are like this:
```cpp
#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_AC_RESULT : public TS_MESSAGE
{
	uint16_t request_msg_id;
	uint16_t result;
	int32_t login_flag;
	static const uint16_t packetID = 10000;
};
#pragma pack(pop)
```

Packet defined like this respect these rules:
- They derive from TS_MESSAGE for packet with fixed size or TS_MESSAGE_WNA for packets with dynamic size
  - Packets with dynamic size should contains a dynamic array like this:
    ```cpp
    #pragma pack(push, 1)
    struct TS_CU_UPLOAD : public TS_MESSAGE_WNA
    {
        uint32_t file_length;
        unsigned char file_contents[0];

        static const uint16_t packetID = 50007;
    };
    #pragma pack(pop)
    ```
    Here, file_contents has a dynamic size. In some packets, `[0]` can be `[]` instead.
- The header file includes `Packet/PacketBaseMessage.h` to make `TS_MESSAGE`  and  `TS_MESSAGE_WNA` available.
- The struct is enclosed inside `#pragma pack(push, 1)` and `#pragma pack(pop)`.
- The struct contains a special variable `static const uint16_t packetID` containing the packet ID.

## Packet declarative descriptions
Declarative description allow to use a serializer class to iterate over fields. Serializing a packet is slower than with plain structures but much more powerful.
It is possible to generate alternative packet formats than binary like JSON, lua table to use in lua scripts, ....
It is also possible to support multiple packet versions without recompiling.
The implementation is based on https://chadaustin.me/struct.txt.

### Basic packet
A declarative description of a packet looks like this:
```cpp
#define TS_SC_WARP_DEF(_) \
	_(simple)(float, x) /* a comment, don't use // */ \
	/* an empty line with no field */ \
	/* backslash need to be at the end of the line */ \
	_(simple)(float, y) \
	_(simple)(float, z) \
	_(simple)(int8_t, layer) /* no backslash on the last line */

CREATE_PACKET(TS_SC_WARP, 12);
```

The packet structure description is made via a `#define name_DEF(_)`.
It follows these rules:
- Packet structure is instantiated using `CREATE_PACKET(name, packet ID);`. In the example, it will generate a structure named   `TS_SC_WARP` with packet ID `12`.
- The `#define` above `CREATE_PACKET` must be named `name_DEF` (append a `_DEF` to the packet name used in `CREATE_PACKET`). It must ends with a backslash `\`:
	- `#define TS_SC_WARP_DEF(_) \`
- Each field is on its line with this convention:
	- `_(<field_metatype>)(arguments, ...) \`
	- See later in this file for information on fields.
- There should be no empty lines between `#define` and fields. If you still want to add one, put a backslash at the end of the line `\`.
- Comments inside fields lines need to be put before the backslash `\` and started with `/*` end ended with `*/`.
- The last field line must not have a backslash `\`.

### More complex packet

A more complex description is like this:
```cpp
#define TS_BONUS_INFO_DEF(_) \
	_(simple)(int32_t, type) \
	_(simple)(int32_t, rate, version >= EPIC_5_1) \
	_(def)(simple) (int64_t, exp) \
	_(impl)(simple)(int64_t, exp, version >= EPIC_6_1) \
	_(impl)(simple)(int32_t, exp, version < EPIC_6_1) \
	_(simple)(int32_t, jp)

CREATE_STRUCT(TS_BONUS_INFO);

#define TS_SC_BONUS_EXP_JP_DEF(_) \
	_(simple)(uint32_t, handle) \
	_(count)(uint16_t, bonus) \
	_(dynarray)(TS_BONUS_INFO, bonus)

CREATE_PACKET(TS_SC_BONUS_EXP_JP, 1004);
```

Here, the use of `CREATE_STRUCT` works the same as `CREATE_PACKET` but it doesn't do packet specific things like associating the packet with an ID or add the packet header (size, id, msg_checksum, like `TS_MESSAGE`).
The struct can be then used as a type for a field (here: `TS_SC_BONUS_EXP_JP::bonus`).

Omitting generated functions, the generated structures are like this (comments are added to explain things):
```cpp
struct TS_BONUS_INFO {
  // This function getName() will return the name of the struct as a string
  static inline const char *getName() { return "TS_BONUS_INFO"; };

  // These following fields are the ones declared.
  // _(impl) lines are not used (only for deserialize/serialize functions)
  int32_t type;
  int32_t rate;
  int64_t exp;
  int32_t jp;

  // This function returns the structure size for the given version
  // The version argument is used as the packet might have newer fields in later versions
  uint32_t getSize(int version) const;

  // These functions take an arbitrary class variable "buffer" where functions will be called with the field name and the field value and other arguments as needed.
  // Their goal is to serialize the structure into a format generated by the class T.
  template <class T> void serialize(T *buffer) const;
  template <class T> void deserialize(T *buffer);
};

struct TS_SC_BONUS_EXP_JP {
  // Same as for strucs (see above)
  static inline const char *getName() { return "TS_SC_BONUS_EXP_JP"; }

  // This field can be used to retrieve the packet id like this: TS_SC_BONUS_EXP_JP::packetID.
  // When possible, getId should be prefered as it is generic (some packets have differents ID depending on version)
  static const uint16_t packetID = 1004;
  static inline uint16_t getId(int version) { return 1004; }

  // This field is always here for packets.
  // It contains the real id (which can be different than packetID or getId in case of a mismatch.
  uint16_t id;

  // Declared fields of the packet.
  // Note that the _(count) field will contains bonus.size().
  uint32_t handle;
  std::vector<TS_BONUS_INFO> bonus;

  // Same as for strucs (see above)
  uint32_t getSize(int version) const;
  template <class T> void serialize(T *buffer) const;
  template <class T> void deserialize(T *buffer);
};
```

## Field description reference

Each fields can be of two form:
- `_(metatype)(arguments...)`
- `_(def/impl)(metatype)(arguments...)`

The first is the most common case.
The second adds a new indicator: `(def/impl)`.

### Field metatypes
Each field must have a "metatype".
They have common argument on almost all metatype:
- `type`: the field type. Supported types are:
	- `int8_t` / `uint8_t`: signed / unsigned 8 bits integer
	- `int16_t` / `uint16_t`: signed / unsigned 16 bits integer
	- `int32_t` / `uint32_t`: signed / unsigned 32 bits integer
	- `int64_t` / `uint64_t`: signed / unsigned 64 bits integer
	- `bool`: boolean (8 bits)
	- `float`: 32 bits floating point number
	- `double`: 64 bits floating point number
	- any structure type declared using `CREATE_STRUCT`
	- any enum type. It is preferred to use an enum with specified underlying type to ensure its size.
	- Don't use `char`, for strings use their matching metatype `(string)` or `(dynstring)`
-  `name`: the field name. This is the name used in the generated struct.
	- Note: `version` is a reserved name, don't use it as a name.
- `condition?`: An optional condition. If omitted, it behaves as it was true
	- The condition can use previous fields or `version`.
	  The `version` special field contains the target version epic. Use it when a field changed in some epic.
	  If this condition is true, the field will be serialized/deserialized. Else it won't.
	- Example:
		- `version >= EPIC_6_1` (while EPIC_6_1 is a #define to a number)
		- `type == ST_Fire || type == ST_RegionFire` (`type` is a field in the packet)
- `defaultValue?`: An optional default value. Requires the `condition?` argument to be present.
	- This is rarely used, and I even think it should be removed :-)
	- When deserializing, if this argument is set and the `condition?` is not satisfied, the field is set to `defaultValue?`.

The following list describe the available metatype and their arguments.
- `(simple)(type, name, condition?, defaultValue?)`
	- Simple field generated as `type name;`
- `(array)(type, name, size, condition?, defaultValue?)`
	- Fixed size array generated as `type name[size];`
	  The `size` argument indicate the size of the fixed array.
- `(dynarray)(type, name, condition?, defaultValue?)`
	- Dynamic sized array generated as `std::vector<type> name;`
	  The size is stored in the matching `(count)` metatype field.
	  If the array size is larger than the maximum possible value that the matching `(count)` field can hold, it is truncated.
- `(count)(type, ref, condition?, defaultValue?)`
	- Special field indicating the size of the dynamic array whose `name` is `ref`.
	  Match `std::vector::size()` and is not available directly in the generated structure.
- `(string)(name, size, condition?, defaultValue?)`
	- Fixed size string generated as `std::string name`
	  The `size` argument indicate the size of the serialized fixed string.
	  If the string + null terminator (`\0`) is larger that the given argument `size`, it will be truncated.
 - `(dynstring)(name, hasNullTerminator, condition?, defaultValue?)`
	- Dynamic sized string generated as `std::string name`.
	  The size is stored in the matching `(count)` metatype field.
	  The `hasNullTerminator` argument is a boolean indicating if the serialized string must include the null terminator `\0`.
	  If the std::string size (+ its null terminator if hasNullTerminator is true) is larger than the maximum possible value that the matching `(count)` field can hold, it is truncated.
- `(endstring)(name, hasNullTerminator, condition?, defaultValue?)`
	- Same as `(dynstring)` but without a matching `(count)`. The actual size of the string is the remaining bytes of the packets.
	  This field metatype can only be used as the last field of a packet/struct.
- `(endarray)(type, name, condition?, defaultValue?)`
	- Same as `(dynarray)` but without a matching `(count)`. The actual size of the array is the remaining bytes of the packets.
	  This field metatype can only be used as the last field of a packet/struct.
- `(padmarker)(marker)`
	- Declare a padding marker. This is not actually a field.
	  This allow to ensure a certain sized padding with `(pad)`
- `(pad)(size, marker, condition?)`
	- Declare a padding to ensure. This is used with the `(padmarker)` whose `marker` is the same marker.
	  This metatype will ensure that the size in bytes between the `(padmarker)` and this `(pad)` match `size` (if `condition?` is fulfilled).
	- For example:
		```cpp
	    _(padmarker)(first_field_pad_marker) \
	    _(simple)(int32_t, first_field) \
	    _(pad)    (10, first_field_pad_marker)
	    _(simple)(int32_t, second_field) \
	  ```
	  Will ensure that the size between the `(padmarker)` `first_field_pad_marker` and `(pad)` `first_field_pad_marker` is at least 10 bytes.
	  In that case, there are `10 - sizeof(first_field) =` 6 bytes of padding between `first_field` and `second_field`.

### `(def)`/`(impl)` indicators
Here is its description:
- `(def)`:
	- When this is present, the field declaration will be used only for the field declaration in the generated structure.
	- It won't be used in serialize/deserialize function.
- `(impl)`:
	- When this is present, it indicate a way to serialize/deserialize an already declared field (with `(def)`).

For example, in this case:
```cpp
_(def)(simple) (int64_t, exp) \
_(impl)(simple)(int64_t, exp, version >= EPIC_6_1) \
_(impl)(simple)(int32_t, exp, version < EPIC_6_1) \
```

The generated structure will have a field `int64_t exp;`.
But the serialize/deserialize function will read/write a int64_t type (that is, 8 bytes) if the version is >= EPIC_6_1, else if it is < EPIC_6_1, it will read/write a int32_t type (that is 4 bytes) and store it in the generated structure's `exp` field which is int64_t.

Generated code is:
```cpp
[...]
int64_t exp; // Match the (def) line
[...]
uint32_t getSize(int version) const {
[...] // Match the (impl) line
  if (version >= EPIC_6_1)
    size += PacketDeclaration::getSizeOf((int64_t)exp, version);
  if (version < EPIC_6_1)
    size += PacketDeclaration::getSizeOf((int32_t)exp, version);
[...]
}
template <class T> void serialize(T *buffer) const {
[...] // Match the (impl) line
  if (version >= EPIC_6_1)
    buffer->write("exp", (int64_t)exp);
  if (version < EPIC_6_1)
    buffer->write("exp", (int32_t)exp);
[...]
}
template <class T> void deserialize(T *buffer) {
[...] // Match the (impl) line
  if (version >= EPIC_6_1)
    buffer->template read<int64_t>("exp", exp);
  if (version < EPIC_6_1)
    buffer->template read<int32_t>("exp", exp);
[...]
}
```

### Packet with multiple IDs
Some packets have a changing ID over the epics, for example TS_CS_VERSION.

To handle this, the packet description change a bit:
- Instead of CREATE_PACKET(name, id), use CREATE_PACKET_VER_ID(name)
- Add a new `#define <name>_ID(X) \` where the following lines are of the form `X(id, condition)`

For example, TS_CS_VERSION use ID 50 before epic 7.4 and then ID 51:
```
#define TS_CS_VERSION_DEF(_) \
	_(string)(szVersion, 20) \
	_(array)(uint8_t, checksum, 256, version >= EPIC_9_5_2)

#define TS_CS_VERSION_ID(X) \
	X(50, version < EPIC_7_4) \
	X(51, version >= EPIC_7_4)

CREATE_PACKET_VER_ID(TS_CS_VERSION);
```

Packets defined with `CREATE_PACKET_VER_ID` will have the function `uint16_t getId(int version)` returns the packet's ID for the given version.

### Ways to use packet IDs (getId, packetID, id)

- `T::packetID`: this is the ID we expect to get for that packet type. Only available if that ID never changed accross epics
- `T::getId(int version)`: this retrieve the packet ID for the given epic (that function is always available). It will return 50 or 51 for TS_CS_VERSION for example.
- `packet.id` field: this field contains the ID as received in the real packet

So to reply to a packet and retrieve the ID the client used, use `packet->id`.
To use the ID in a switch/case, use `case T::packetID:` or, when the packet can use different IDs for different epics, use `case_packet_is(T)` without colon `:` in place of a normal case `case T::packetID:`.
When you need the packet version in other part of the code, use `T::getId(EPIC_X_Y)` if possible.

### Complex generated code

To see a complex case (TS_SC_SKILL) of generated code, see [Packet_generated_code.md].
