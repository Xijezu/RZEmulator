#pragma once
/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include "Skill.h"
#include "Unit.h"

class SkillProp : public WorldObject {
public:
    static SkillProp *Create(uint32_t caster, Skill *pSkill, int32_t nMagicPoint, float fHateRatio);
    static void EnterPacket(XPacket &pEnterPct, SkillProp *pSkillProp, Player *pPlayer);
    SkillProp() = delete;
    ~SkillProp() override = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    SkillProp(const SkillProp &) = delete;
    SkillProp &operator=(const SkillProp &) = delete;

    void Update(uint32_t diff) override;
    bool IsSkillProp() const override;
    void PendRemove();

protected:
    void INIT_AREA_EFFECT_MAGIC_DAMAGE();
    void INIT_AREA_EFFECT_HEAL();
    void INIT_AREA_EFFECT_HEAL_BY_FIELD_PROP();

    void FIRE_AREA_EFFECT_MAGIC_DAMAGE_OLD(Unit *pCaster);
    void FIRE_AREA_EFFECT_HEAL(Unit *pCaster);
    void FIRE_AREA_EFFECT_HEAL_BY_FIELD_PROP(Unit *pCaster);

    void INIT_SKILL_PROP_PARAMETER(uint32_t nDuration, uint32_t nInterval);

    void FIRE_AREA_EFFECT_MAGIC_DAMAGE(Unit *pCaster);
    void FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL(Unit *pCaster);
    void FIRE_AREA_EFFECT_MAGIC_DAMAGE_AND_HEAL_T2(Unit *pCaster);

    void FIRE_TRAP_DAMAGE(Unit *pCaster);
    void FIRE_TRAP_MULTIPLE_DAMAGE(Unit *pCaster);

private:
    explicit SkillProp(uint32_t caster, Skill pSkill, int32_t nMagicPoint, float fHateRatio);

    struct _SKILL_PROP_INFO {
        uint32_t m_nStartTime;
        uint32_t m_nEndTime;
        uint32_t m_nInterval;
        uint32_t m_nLastFireTime;
    };

    uint32_t m_hCaster;
    _SKILL_PROP_INFO m_Info;
    Skill m_pSkill;
    bool m_bFired;
    bool m_bProcessEnded;
    bool m_bIsRemovePended;
    int32_t m_nOwnerMagicPoint;
    float m_fHateRatio;
};