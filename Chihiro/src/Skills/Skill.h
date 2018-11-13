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
#include "Object.h"
#include "SkillBase.h"

class XPacket;
class Unit;

enum SkillStatus : int
{
    SS_IDLE     = 0,
    SS_CAST     = 1,
    SS_FIRE     = 2,
    SS_COMPLETE = 3
};

struct SkillTargetFunctor;
class Skill
{
        friend class SkillProp;
        friend class Unit;
    public:
        friend struct SkillTargetFunctor;
        Skill() = delete;
        Skill(Unit *pOwner, int64 _uid, int _id);
        // Replace statement - acts as insert and update
        static void DB_InsertSkill(Unit *pUnit, int64 skillUID, int skill_id, int skill_level, int cool_time);

        int Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem);
        void ProcSkill();
        bool Cancel();
        uint GetSkillEnhance() const;

        bool CheckCoolTime(uint t) const;
        uint GetSkillCoolTime() const;
        void SetRemainCoolTime(uint time);
        void SetRequestedSkillLevel(int nLevel);

        int64 m_nSkillUID;
        Unit  *m_pOwner{nullptr};
        int   m_nSummonID;
        int   m_nSkillID;
        int   m_nSkillLevel;
        int   m_nSkillLevelAdd{0};
        int   cool_time;

        SkillBase *m_SkillBase{nullptr};

        void broadcastSkillMessage(WorldObject *pObject, int cost_hp, int cost_mp, int nType);
        void broadcastSkillMessage(Unit *pUnit1, Unit *pUnit2, int cost_hp, int cost_mp, int nType);

        // For processing skill
        uint8 m_nRequestedSkillLevel;
    private:
        Position m_targetPosition{ };
        float    m_fRange;
        uint     m_nCastingDelay;
        uint     m_nEnhance{ };
        uint16   m_nErrorCode{ };
        uint     m_hTarget{ };
        uint     m_nCastTime{ };
        uint     m_nNextCoolTime{ };
        uint     m_nFireTime{ };
        int      m_nCurrentFire{ };
        int      m_nTotalFire{ };
        bool     m_bMultiple{false};
        uint     m_nTargetCount;
        uint     m_nFireCount;

        SkillStatus m_Status{ };
    protected:
        void assembleMessage(TS_SC_SKILL &pSkillPct, int nType, int cost_hp, int cost_mp);
        void Init();
    private:
        int GetCurrentMPCost();
        int GetCurrentHPCost();

        std::vector<SkillResult> m_vResultList{ };
        void process_target(uint t, SkillTargetFunctor &fn, Unit *pTarget);
        void FireSkill(Unit *pTarget, bool &bIsSuccess);
        void PostFireSkill(Unit *pTarget);
        uint16 PrepareSummon(int nSkillLevel, uint handle, Position pos, uint current_time);
        uint16 PrepareTaming(int nSkillLevel, uint handle, Position pos, uint current_time);

        // I'm sorry, I'm copying retail code here
        void SINGLE_PHYSICAL_DAMAGE(Unit *pTarget);
        void SINGLE_MAGICAL_DAMAGE(Unit *pTarget);

        void SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(Unit *pTarget);

        void MAGIC_SINGLE_REGION_DAMAGE(Unit *pTarget);
        void MAGIC_MULTIPLE_REGION_DAMAGE(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE(Unit *pTarget);
        void TOGGLE_AURA(Unit *pTarget);

        void HEALING_SKILL_FUNCTOR(Unit *pTarget);
        void MANA_SKILL_FUNCTOR(Unit *pTarget);
        void SKILL_RESURRECTION(Unit *pTarget);
        void ACTIVATE_FIELD_PROP();
        void TOWN_PORTAL();
        void DO_SUMMON();
        void DO_UNSUMMON();
        void CREATURE_TAMING();
        void ADD_ENERGY();
};