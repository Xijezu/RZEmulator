#ifndef PROJECT_SKILL_H
#define PROJECT_SKILL_H

#include "Common.h"

class Unit;
class Skill {
public:
    Skill() = default;
    Skill(Unit* pOwner, int _uid, int _id);
    static void DB_InsertSkill(Unit*,uint,uint,uint,uint,uint);
    static void DB_UpdateSkill(Unit*,uint,uint);

    int sid;
    int owner_id;
    int summon_id;
    int skill_id;
    int skill_level;
    int cool_time;
};


#endif // PROJECT_SKILL_H
