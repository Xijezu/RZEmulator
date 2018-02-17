#ifndef PROJECT_SKILLPROP_H
#define PROJECT_SKILLPROP_H

#include "Common.h"
#include "Unit.h"

struct SkillPropInfo
{
    uint m_nStartTime;
    uint m_nEndTime;
    uint m_nInterval;
    uint m_nLastFireTime;
};

class SkillProp : public WorldObject
{
    public:
        static SkillProp* Create(uint caster, Skill* pSkill, int nMagicPoint, float fHateRatio);
        SkillProp() = delete;
        ~SkillProp() override = default;
        void Update(uint diff) override;
        bool IsSkillProp() const override;
        void PendRemove();

    protected:
        void INIT_AREA_EFFECT_MAGIC_DAMAGE();
        void INIT_AREA_EFFECT_HEAL();
        void INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP();
        void INIT_SKILL_PROP_PARAMETER(uint nDuration, uint nInterval);
        void FIRE_AREA_EFFECT_MAGIC_DAMAGE(Unit* pCaster);

    private:
        explicit SkillProp(uint caster, Skill* pSkill, int nMagicPoint, float fHateRatio);

        uint m_hCaster;
        SkillPropInfo m_Info;
        Skill* m_pSkill;
        bool m_bFired;
        bool m_bProcessEnded;
        bool m_bIsRemovePended;
        int m_nOwnerMagicPoint;
        float m_fHateRatio;
};

#endif // PROJECT_SKILLPROP_H
