#ifndef PROJECT_RESPAWNOBJECT_H
#define PROJECT_RESPAWNOBJECT_H

#include "Common.h"
#include "Monster.h"

struct RespawnInfo : public MonsterRespawnInfo {
    explicit RespawnInfo(MonsterRespawnInfo info) : MonsterRespawnInfo(info)
    {
        count = 0;
        prespawn_count = info.max_num / 2;
        if(prespawn_count < 1 || dungeon_id != 0 || way_point_id != 0)
            prespawn_count = info.max_num;
    }

    uint count;
    uint prespawn_count;
};

class RespawnObject {
public:
    RespawnObject(MonsterRespawnInfo rh);
    ~RespawnObject() = default;

    void OnMonsterDelete(Monster* mob);
    void Update(uint diff);

    RespawnInfo info;
    uint m_nMaxRespawnNum;
    std::vector<uint> m_vRespawnedMonster;
    uint lastDeadTime;
};


#endif // PROJECT_RESPAWNOBJECT_H
