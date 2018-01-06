#ifndef PROJECT_SKILL_H
#define PROJECT_SKILL_H

#include "Common.h"
#include "Object.h"
#include "SkillBase.h"

class XPacket;
class Unit;

class Skill {
public:
    Skill() = delete;
    Skill(Unit* pOwner, uint64 _uid, int _id);
    static void DB_InsertSkill(Unit*,uint,uint,uint,uint,uint);
    static void DB_UpdateSkill(Unit*,uint,uint);

    int Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem);
    void ProcSkill();
    bool Cancel();

    uint64 m_nSkillUID;
    Unit* m_pOwner{nullptr};
    int m_nSummonID;
    int m_nSkillID;
    int m_nSkillLevel;
    int m_nSkillLevelAdd{0};
    int cool_time;

    SkillBase* m_SkillBase{nullptr};

    void broadcastSkillMessage(int rx, int ry, uint8 layer, int cost_hp, int cost_mp, int nType);
    void broadcastSkillMessage(int rx1, int ry1, int rx2, int ry2, uint8 layer, int cost_hp, int cost_mp, int nType);

    // For processing skill
    uint8 m_nRequestedSkillLevel;
private:
    Position m_targetPosition{};
    uint m_nCastingDelay;
    uint16 m_nErrorCode;
    uint m_hTarget;
    uint m_nCastTime;
    uint m_nFireTime;
protected:
    void assembleMessage(XPacket& pct, int nType, int cost_hp, int cost_mp);
private:
    void DoSummon();
    void DoUnsummon();
    int PrepareSummon(int nSkillLevel, uint handle, Position pos,  uint current_time);
};


#endif // PROJECT_SKILL_H
