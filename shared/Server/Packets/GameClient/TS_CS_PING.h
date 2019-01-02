/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NGEMITY_TS_CS_PING_H
#define NGEMITY_TS_CS_PING_H

#include "Packets/PacketDeclaration.h"

#define TS_CS_PING_DEF(_) \
    _(endarray)(uint8_t, dummy)
CREATE_PACKET(TS_CS_PING, 9999);

#endif //NGEMITY_TS_CS_PING_H
