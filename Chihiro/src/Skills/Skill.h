#ifndef PROJECT_SKILL_H
#define PROJECT_SKILL_H

#include "Common.h"
#include "Object.h"
#include "SkillBase.h"

class XPacket;
class Unit;

class Skill {
    friend class Unit;
public:
    friend struct SkillTargetFunctor;
    Skill() = delete;
    Skill(Unit* pOwner, int64 _uid, int _id);
    static void DB_InsertSkill(Unit*,int64,uint,uint,uint,uint);
    static void DB_UpdateSkill(Unit*,int64,uint);

    int Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem);
    void ProcSkill();
    bool Cancel();
    uint GetSkillEnhance() const;

    bool CheckCoolTime(uint t) const;
    uint GetSkillCoolTime() const;
    void SetRemainCoolTime(uint time);


    int64 m_nSkillUID;
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
    uint m_nEnhance{};
    uint16 m_nErrorCode{};
    uint m_hTarget{};
    uint m_nCastTime{};
    uint m_nNextCoolTime{};
    uint m_nFireTime{};
    uint m_nTargetCount;
    uint m_nFireCount;
protected:
    void assembleMessage(XPacket& pct, int nType, int cost_hp, int cost_mp);
    void Init();
private:
    std::vector<SkillResult> m_vResultList{};
    void FireSkill(Unit* pTarget, bool &bIsSuccess);
    uint16 PrepareSummon(int nSkillLevel, uint handle, Position pos,  uint current_time);
    uint16 PrepareTaming(int nSkillLevel, uint handle, Position pos,  uint current_time);

    // I'm sorry, I'm copying retail code here
    void SINGLE_PHYSICAL_DAMAGE(Unit* pTarget);
    void SINGLE_MAGICAL_DAMAGE(Unit* pTarget);

    void HEALING_SKILL_FUNCTOR(Unit* pTarget);

    void ACTIVATE_FIELD_PROP();

    void TOWN_PORTAL();
    void DO_SUMMON();
    void DO_UNSUMMON();
    void CREATURE_TAMING();
};


#endif // PROJECT_SKILL_H
