#ifndef PROJECT_NPCBASE_H
#define PROJECT_NPCBASE_H

#include "Common.h"

struct NPCTemplate {
    uint32      id;
    uint32      x;
    uint32      y;
    uint32      z;
    int         spawn_type;
    uint32      face;
    uint32      local_flag;
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

enum  NPC_SpawnType : int
{
    SPAWN_NORMAL         = 0,
    SPAWN_SIEGE_DUNGEON  = 1,
    SPAWN_NORMAL_DUNGEON = 2,
    SPAWN_NONE           = 3,
    SPAWN_CASH           = 4,
};

#endif // PROJECT_NPCBASE_H
