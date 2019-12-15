#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "DamageTemplate.h"
#include "Object.h"
#include "SkillBase.h"

class XPacket;
class Unit;

enum SkillStatus : int
{
  SS_IDLE = 0,
  SS_CAST = 1,
  SS_FIRE = 2,
  SS_COMPLETE = 3,
  SS_PRE_CAST = 4
};

struct SkillTargetFunctor;
class Skill
{
  friend class SkillProp;
  friend class Unit;

public:
  enum
  {
    MAX_SKILL_LEVEL = 50,
    MAX_TOGGLE_GROUP = 20,
    MAX_SKILL_VALUE = 20,
    TOGGLE_REFRESH_TIME = 500,
    TOGGLE_LIVE_TIME = 1100,
  };

  friend struct SkillTargetFunctor;
  Skill() = delete;
  Skill(Unit *pOwner, int64_t _uid, int32_t _id);
  // Replace statement - acts as insert and update
  static void DB_InsertSkill(Unit *pUnit, int64_t skillUID, int32_t skill_id,
                             int32_t skill_level, int32_t cool_time);
  // skills
  static void AddSkillResult(std::vector<SkillResult> &pvList, bool bIsSuccess,
                             int32_t nSuccessType, uint32_t handle);
  static void AddSkillDamageResult(std::vector<SkillResult> &pvList, uint8_t type,
                                   int32_t damageType, DamageInfo damageInfo,
                                   uint32_t handle);
  static void AddSkillDamageWithKnockBackResult(
      std::vector<SkillResult> &pvList, uint8_t type, int32_t damage_type,
      const DamageInfo &damage_info, uint32_t handle, float x, float y,
      uint32_t knock_back_time);
  static int32_t EnumSkillTargetsAndCalcDamage(
      const Position &_OriginalPos, uint8_t layer, const Position &_TargetPos,
      bool bTargetOrigin, const float fEffectLength, const int32_t nRegionType,
      const float fRegionProperty, const int32_t nOriginalDamage,
      const bool bIncludeOriginalPos, Unit *pCaster, const int32_t nDistributeType,
      const int32_t nTargetMax, /*out*/ std::vector<Unit *> &vTargetList,
      bool bEnemyOnly = true);

  int32_t Cast(int32_t nSkillLevel, uint32_t handle, Position pos, uint8_t layer,
           bool bIsCastedByItem);

  int32_t GetSkillId() const { return GetSkillBase()->GetID(); }

  void ProcSkill();
  bool ProcAura();

  inline float GetVar(int32_t idx) const { return GetSkillBase()->var[idx]; }

  bool Cancel();
  uint32_t GetSkillEnhance() const;

  int32_t GetCurrentSkillLevel() const { return m_nSkillLevel + m_nSkillLevelAdd; }

  SkillBase *GetSkillBase() const { return m_SkillBase; }

  uint8_t GetRequestedSkillLevel() const { return m_nRequestedSkillLevel; }

  bool CheckCoolTime(uint32_t t) const;
  uint32_t GetSkillCoolTime() const;
  void SetRemainCoolTime(uint32_t time);
  void SetRequestedSkillLevel(int32_t nLevel);

  int64_t m_nSkillUID;
  Unit *m_pOwner{nullptr};
  int32_t m_nSummonID;
  int32_t m_nSkillID;
  int32_t m_nSkillLevel;
  int32_t m_nSkillLevelAdd{0};
  int32_t cool_time;

  SkillBase *m_SkillBase{nullptr};

  void broadcastSkillMessage(WorldObject *pObject, int32_t cost_hp, int32_t cost_mp,
                             int32_t nType);
  void broadcastSkillMessage(Unit *pUnit1, Unit *pUnit2, int32_t cost_hp,
                             int32_t cost_mp, int32_t nType);

  // For processing skill
  uint8_t m_nRequestedSkillLevel;

private:
  Position m_targetPosition{};
  float m_fRange;
  uint32_t m_nCastingDelay;
  uint32_t m_nEnhance{};
  uint16_t m_nErrorCode{};
  uint32_t m_hTarget{};
  uint32_t m_nCastTime{};
  uint32_t m_nNextCoolTime{};
  uint32_t m_nFireTime{};
  int32_t m_nCurrentFire{};
  int32_t m_nTotalFire{};
  bool m_bMultiple{false};
  uint32_t m_nTargetCount;
  uint32_t m_nFireCount;
  uint32_t m_nAuraMPDecTime;
  uint32_t m_nAuraRefreshTime;
  Position m_RushPos;
  float m_fRushFace;
  int32_t m_nRushDamage;

  SkillStatus m_Status{};

protected:
  void assembleMessage(TS_SC_SKILL &pSkillPct, int32_t nType, int32_t cost_hp,
                       int32_t cost_mp);
  void Init();

private:
  int32_t InitError(uint16_t);
  uint32_t GetAuraMPDecTime() { return m_nAuraMPDecTime; }
  void SetAuraMPDecTime(uint32_t nTime) { m_nAuraMPDecTime = nTime; }
  uint32_t GetAuraRefreshTime() { return m_nAuraRefreshTime; }
  void SetAuraRefreshTime(uint32_t nTime) { m_nAuraRefreshTime = nTime; }
  uint32_t GetTargetHandle() const { return m_hTarget; }
  Position GetTargetPosition() const { return m_targetPosition; }
  uint32_t GetCastingTime() const { return m_nFireTime - m_nCastTime; }
  uint32_t GetOriginalCastingDelay() const { return m_nCastingDelay; }
  std::vector<SkillResult> m_vResultList{};
  void process_target(uint32_t t, SkillTargetFunctor &fn, Unit *pTarget);
  void FireSkill(Unit *pTarget, bool &bIsSuccess);
  void PostFireSkill(Unit *pTarget);
  Position GetMovableKnockBackPosition(Position &OriginalPos,
                                       Position &TargetPos);
  uint16_t PrepareSummon(uint32_t handle, Position pos);
  uint16_t PrepareTaming(uint32_t handle);

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
  bool RUSH(Unit *pTarget, float fSpeed);
  bool AFFECT_RUSH(Unit *pTarget, float &pfRushDistance, Position &pRushPos,
                   float &pface, float fSpeed);

  void TOGGLE_AURA(Unit *pTarget);
  void SKILL_ADD_HP_MP(Unit *pTarget);

  void SINGLE_PHYSICAL_DAMAGE_T1(Unit *pTarget);
  void SINGLE_PHYSICAL_DAMAGE_T2(Unit *pTarget);

  bool PHYSICAL_DAMAGE_RUSH(Unit *pTarget, int32_t &pnAdditionalDamage);
  bool AFFECT_RUSH_OLD(Unit *pTarget, float &pfRushDistance, Position &pRushPos,
                       float &pface);
  int32_t AFFECT_KNOCK_BACK(Unit *pTarget, float fRange, uint32_t knock_back_time);
  void PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD(Unit *pTarget);
  void PHYSICAL_MULTIPLE_SPECIAL_REGION_DAMAGE(Unit *pTarget);
  void PHYSICAL_SPECIAL_REGION_DAMAGE(Unit *pTarget);
  void SINGLE_PHYSICAL_DAMAGE_T2_ADD_ENERGY(Unit *pTarget);
  void MULTIPLE_PHYSICAL_DAMAGE_T1(Unit *pTarget);
  void MULTIPLE_PHYSICAL_DAMAGE_T2(Unit *pTarget);
  void MULTIPLE_PHYSICAL_DAMAGE_T3(Unit *pTarget);

  void PHYSICAL_SINGLE_DAMAGE_ADD_ENERGY(Unit *pTarget);
  void PHYSICAL_REALTIME_MULTIPLE_DAMAGE(Unit *pTarget);
  void PHYSICAL_REALTIME_MULTIPLE_REGION_DAMAGE(Unit *pTarget);
  void PHYSICAL_MULTIPLE_DAMAGE_TRIPLE_ATTACK(Unit *pTarget);
  void CASTING_CANCEL_WITH_ADD_STATE(Unit *pTarget);
  void CORPSE_ABSORB(Unit *pTarget);
  void CORPSE_EXPLOSION(Unit *pTarget);

  void UNSUMMON_AND_ADD_STATE();

  void SINGLE_MAGICAL_DAMAGE_T1(Unit *pTarget);
  void SINGLE_MAGICAL_DAMAGE_T2(Unit *pTarget);
  void SINGLE_DAMAGE_BY_CONSUMING_TARGETS_STATE(Unit *pTarget);
  void MULTIPLE_MAGICAL_DAMAGE_T1(Unit *pTarget);
  void MULTIPLE_MAGICAL_DAMAGE_AT_ONCE(Unit *pTarget);
  void SKILL_RESURRECTION_WITH_RECOVER(Unit *pTarget);

  void SINGLE_PHYSICAL_DAMAGE_T3(Unit *pTarget);
  void MULTIPLE_PHYSICAL_DAMAGE_T4(Unit *pTarget);
  void CREATE_ITEM(Unit *pTarget, bool &pbIsSuccess);
  void REGION_HEAL_BY_FIELD_PROP();
  void MAGIC_SINGLE_REGION_DAMAGE_BY_SUMMON_DEAD(Unit *pTarget);
  void REGION_TAUNT(Unit *pTarget);
  void REMOVE_HATE(Unit *pTarget);
  void REGION_REMOVE_HATE(Unit *pTarget);
  void MAGIC_MULTIPLE_REGION_DAMAGE_AT_ONCE(Unit *pTarget);
  void MAGIC_MULTIPLE_REGION_DAMAGE_OLD(Unit *pTarget);
  void MAGIC_MULTIPLE_REGION_DAMAGE_T2(Unit *pTarget);
  void MAGIC_SPECIAL_REGION_DAMAGE_OLD(Unit *pTarget);
  void MAGIC_SPECIAL_REGION_DAMAGE(Unit *pTarget);
  void MAGIC_SINGLE_REGION_PERCENT_DAMAGE(Unit *pTarget);
  void MAGIC_ABSORB_DAMAGE(Unit *pTarget);

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