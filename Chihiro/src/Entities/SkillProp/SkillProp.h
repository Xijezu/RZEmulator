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
        static SkillProp *Create(uint caster, Skill *pSkill, int nMagicPoint, float fHateRatio);
        SkillProp() = delete;
        ~SkillProp() override = default;
        // Deleting the copy & assignment operators
        // Better safe than sorry
        SkillProp(const SkillProp &) = delete;
        SkillProp &operator=(const SkillProp &) = delete;

        void Update(uint diff) override;
        bool IsSkillProp() const override;
        void PendRemove();

    protected:
        void INIT_AREA_EFFECT_MAGIC_DAMAGE();
        void INIT_AREA_EFFECT_HEAL();
        void INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP();
        void INIT_SKILL_PROP_PARAMETER(uint nDuration, uint nInterval);
        void FIRE_AREA_EFFECT_MAGIC_DAMAGE(Unit *pCaster);

    private:
        explicit SkillProp(uint caster, Skill *pSkill, int nMagicPoint, float fHateRatio);

        uint          m_hCaster;
        SkillPropInfo m_Info;
        Skill *m_pSkill;
        bool  m_bFired;
        bool  m_bProcessEnded;
        bool  m_bIsRemovePended;
        int   m_nOwnerMagicPoint;
        float m_fHateRatio;
};