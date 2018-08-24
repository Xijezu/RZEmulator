#ifndef PACKETS_TS_CS_HIDE_EQUIP_INFO_H
#define PACKETS_TS_CS_HIDE_EQUIP_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_HIDE_EQUIP_INFO_DEF(_) \
	_(simple)(uint32_t, nHideEquipFlag)

CREATE_PACKET(TS_CS_HIDE_EQUIP_INFO, 221);

#endif // PACKETS_TS_CS_HIDE_EQUIP_INFO_H
