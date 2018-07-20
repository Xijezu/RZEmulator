#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "Common.h"

struct NPCTemplate
{
    int         id;
    float       x;
    float       y;
    float       z;
    int         spawn_type;
    int         face;
    int         local_flag;
    std::string contact_script;
};

enum NPC_STATUS
{
    NPC_STATUS_NORMAL          = 0x0,
    NPC_STATUS_TRACKING        = 0x1,
    NPC_STATUS_FIND_ATTACK_POS = 0x2,
    NPC_STATUS_ATTACK          = 0x3,
    NPC_STATUS_DEAD            = 0x4
};

enum NPC_SpawnType : int
{
    SPAWN_NORMAL         = 0,
    SPAWN_SIEGE_DUNGEON  = 1,
    SPAWN_NORMAL_DUNGEON = 2,
    SPAWN_NONE           = 3,
    SPAWN_CASH           = 4,
};