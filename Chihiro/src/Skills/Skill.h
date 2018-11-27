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
        // Deleting the copy & assignment operators
        // Better safe than sorry
        Skill(const Skill &) = delete;
        Skill &operator=(const Skill &) = delete;
        // Replace statement - acts as insert and update
        static void DB_InsertSkill(Unit *pUnit, int64 skillUID, int skill_id, int skill_level, int cool_time);
        // skills
        static void AddSkillResult(std::vector<SkillResult> &pvList, bool bIsSuccess, int nSuccessType, uint handle);
        static void AddSkillDamageResult(std::vector<SkillResult> &pvList, uint8 type, int damageType, DamageInfo damageInfo, uint handle);
        static void AddSkillDamageWithKnockBackResult(std::vector<SkillResult> &pvList, uint8_t type, int damage_type, const DamageInfo &damage_info, uint32_t handle, float x, float y, uint32_t knock_back_time);
        static int EnumSkillTargetsAndCalcDamage(const Position &_OriginalPos, uint8_t layer, const Position &_TargetPos, bool bTargetOrigin, const float fEffectLength, const int nRegionType, const float fRegionProperty, const int nOriginalDamage, const bool bIncludeOriginalPos, Unit *pCaster, const int nDistributeType, const int nTargetMax, /*out*/ std::vector<Unit *> &vTargetList, bool bEnemyOnly = true);

        int Cast(int nSkillLevel, uint handle, Position pos, uint8 layer, bool bIsCastedByItem);

        int GetSkillId() const { return GetSkillBase()->GetID(); }

        void ProcSkill();
        bool ProcAura();

        inline int GetVar(int idx) const { return GetSkillBase()->var[idx]; }

        bool Cancel();
        uint GetSkillEnhance() const;

        int GetCurrentSkillLevel() const { return m_nSkillLevel + m_nSkillLevelAdd; }

        SkillBase *GetSkillBase() const { return m_SkillBase; }

        uint8_t GetRequestedSkillLevel() const { return m_nRequestedSkillLevel; }

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
        uint32_t m_nAuraMPDecTime;
        uint32_t m_nAuraRefreshTime;
        Position m_RushPos;
        float    m_fRushFace;
        int32_t  m_nRushDamage;

        SkillStatus m_Status{ };
    protected:
        void assembleMessage(TS_SC_SKILL &pSkillPct, int nType, int cost_hp, int cost_mp);
        void Init();
    private:
        enum
        {
            MAX_SKILL_LEVEL     = 50,
            MAX_TOGGLE_GROUP    = 20,
            MAX_SKILL_VALUE     = 20,
            TOGGLE_REFRESH_TIME = 500,
            TOGGLE_LIVE_TIME    = 1100,
        };

        int InitError(uint16_t);

        uint32_t GetAuraMPDecTime() { return m_nAuraMPDecTime; }

        void SetAuraMPDecTime(uint32_t nTime) { m_nAuraMPDecTime = nTime; }

        uint32_t GetAuraRefreshTime() { return m_nAuraRefreshTime; }

        void SetAuraRefreshTime(uint32_t nTime) { m_nAuraRefreshTime = nTime; }

        uint32_t GetTargetHandle() const { return m_hTarget; }

        Position GetTargetPosition() const { return m_targetPosition; }

        uint32_t GetCastingTime() const { return m_nFireTime - m_nCastTime; }

        uint32_t GetOriginalCastingDelay() const { return m_nCastingDelay; }

        std::vector<SkillResult> m_vResultList{ };
        void process_target(uint t, SkillTargetFunctor &fn, Unit *pTarget);
        void FireSkill(Unit *pTarget, bool &bIsSuccess);
        void PostFireSkill(Unit *pTarget);
        Position GetMovableKnockBackPosition(Position &OriginalPos, Position &TargetPos);
        uint16 PrepareSummon(uint handle, Position pos);
        uint16 PrepareTaming(uint handle);

        void PHYSICAL_SINGLE_DAMAGE(Unit *pTarget);
        void SINGLE_MAGICAL_DAMAGE(Unit *pTarget);

        void SINGLE_MAGICAL_DAMAGE_WITH_ABSORB(Unit *pTarget);

        void MAGIC_SINGLE_REGION_DAMAGE(Unit *pTarget);
        void MAGIC_MULTIPLE_REGION_DAMAGE(Unit *pTarget);

        void ADD_REGION_STATE(Unit *pTarget);
        void ADD_STATE_BY_SELF_COST(Unit *pTarget);
        void ADD_REGION_STATE_BY_SELF_COST(Unit *pTarget);
        void PHYSICAL_DIRECTIONAL_DAMAGE(Unit *pTarget);
        void SINGLE_PHYSICAL_DAMAGE_ABSORB(Unit *pTarget);
        void MAKE_AREA_EFFECT_PROP_BY_FIELD_PROP(bool bIsTrap);
        void MAKE_AREA_EFFECT_PROP(Unit *pTarget, bool bIsTrap);

        void PHYSICAL_MULTIPLE_DAMAGE(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE(Unit *pTarget);
        void PHYSICAL_MULTIPLE_REGION_DAMAGE(Unit *pTarget);
        void PHYSICAL_SINGLE_SPECIAL_REGION_DAMAGE(Unit *pTarget);
        void PHYSICAL_SINGLE_REGION_DAMAGE(Unit *pTarget);
        void PHYSICAL_SINGLE_REGION_DAMAGE_OLD(Unit *pTarget);
        void PHYSICAL_SINGLE_DAMAGE_ABSORB(Unit *pTarget);
        void TAUNT(Unit *pTarget);

        void TOGGLE_AURA(Unit *pTarget);
        void SKILL_ADD_HP_MP(Unit *pTarget);

        void SINGLE_PHYSICAL_DAMAGE_T1(Unit *pTarget);
        void SINGLE_PHYSICAL_DAMAGE_T2(Unit *pTarget);

        bool PHYSICAL_DAMAGE_RUSH(Unit *pTarget, int &pnAdditionalDamage);
        bool AFFECT_RUSH_OLD(Unit *pTarget, float &pfRushDistance, Position &pRushPos, float &pface);
        int AFFECT_KNOCK_BACK(Unit *pTarget, float fRange, uint32_t knock_back_time);
        void PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD(Unit *pTarget);
        void PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE(Unit *pTarget);
        void PHYSICAL_SPECIAL_REGION_DAMAGE(Unit *pTarget);
        void SINGLE_PHYSICAL_DAMAGE_T2_ADD_ENERGY(Unit *pTarget);
        void MULTIPLE_PHYSICAL_DAMAGE_T1(Unit *pTarget);
        void MULTIPLE_PHYSICAL_DAMAGE_T2(Unit *pTarget);
        void MULTIPLE_PHYSICAL_DAMAGE_T3(Unit *pTarget);

        void SINGLE_MAGICAL_DAMAGE_T1(Unit *pTarget);
        void SINGLE_MAGICAL_DAMAGE_T2(Unit *pTarget);
        void SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE_T1(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE_AT_ONCE(Unit *pTarget);

        void SINGLE_MAGICAL_DAMAGE_OR_DEATH(Unit *pTarget);
        void ADD_HP_MP_BY_ABSORB_HP_MP(Unit *pTarget);
        void SINGLE_MAGICAL_TARGET_HP_PERCENT_DAMAGE(Unit *pTarget);
        void SINGLE_MAGICAL_MANABURN(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE_T2(Unit *pTarget);
        void MULTIPLE_MAGICAL_DAMAGE_T3(Unit *pTarget);
        void MAGIC_SINGLE_REGION_DAMAGE_OLD(Unit *pTarget);

        void SKILL_ADD_REGION_HP_MP(Unit *pTarget);
        void SKILL_ADD_REGION_HP(Unit *pTarget);
        void SKILL_ADD_REGION_MP(Unit *pTarget);

        void SKILL_RESURRECTION(Unit *pTarget);
        void ACTIVATE_FIELD_PROP();
        void TOWN_PORTAL();
        void DO_SUMMON();
        void DO_UNSUMMON();
        void CREATURE_TAMING();
        void ADD_ENERGY();
};