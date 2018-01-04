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

enum NPCStatus : int {
    NPCS_Normal        = 0,
    NPCS_Tracking      = 1,
    NPCS_FindAttackPos = 2,
    NPCS_Attack        = 3,
    NPCS_Dead          = 4,
};

enum  NPC_SpawnType : int {
    NPC_ST_Normal        = 0,
    NPC_ST_DungeonSeige  = 1,
    NPC_ST_DungeonNormal = 2,
    NPC_ST_None          = 3,
    NPC_ST_Cash          = 4,
};

#endif // PROJECT_NPCBASE_H
