#ifndef PACKETS_TS_SC_PROPERTY_H
#define PACKETS_TS_SC_PROPERTY_H

#include "Server/Packets/PacketDeclaration.h"

// No string size field
// String value include null terminator
#define TS_SC_PROPERTY_DEF(_)                                                                                                                                 \
    _(simple)                                                                                                                                                 \
    (uint32_t, handle) _(simple)(uint8_t, is_number) _(string)(name, 16) _(def)(simple)(int64_t, value)_(impl)(simple)(int64_t, value, version >= EPIC_4_1_1) \
        _(impl)(simple)(int32_t, value, version < EPIC_4_1_1) _(endstring)(string_value, true, !is_number)

CREATE_PACKET(TS_SC_PROPERTY, 507);

#endif // PACKETS_TS_SC_PROPERTY_H
