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
#include "Unit.h"
#include "SummonBase.h"

class Player;
class Summon : public Unit
{
public:
  friend class Player;
  static Summon *AllocSummon(Player *, uint);
  explicit Summon(uint, uint);
  // Deleting the copy & assignment operators
  // Better safe than sorry
  Summon(const Summon &) = delete;
  Summon &operator=(const Summon &) = delete;
  ~Summon();

  static void DB_InsertSummon(Player *, Summon *);
  static void DB_UpdateSummon(Player *, Summon *);
  static void EnterPacket(XPacket &, Summon *, Player *pPlayer);

  CreatureStat *GetBaseStat() const override;

  uint GetCreatureGroup() const override { return 9; }

  void OnAfterReadSummon();
  uint32_t GetCardHandle();

  bool IsSummon() const override { return true; }

  bool IsAlly(const Unit *pTarget) override;

  int32_t GetSummonCode();
  float GetFCM() const override;

  Player *GetMaster() const { return m_pMaster; }

  void Update(uint /*diff*/) override;

  bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> *ItemList) override;

  float GetSize() const override;
  float GetScale() const override;

  void SetSummonInfo(int);
  bool DoEvolution();

  int m_nSummonInfo{};
  int m_nCardUID{};
  int m_nTransform{};
  uint8_t m_cSlotIdx{};
  Item *m_pItem{nullptr};

protected:
  void onCantAttack(uint handle, uint t) override;
  uint16 putonItem(ItemWearType pos, Item *pItem) override;
  uint16 putoffItem(ItemWearType pos) override;
  void onRegisterSkill(int64 skillUID, int skill_id, int prev_level, int skill_level) override;
  void processWalk(uint t);
  void onExpChange() override;
  ///- Stat calculations overwriting from Unit class
  void onBeforeCalculateStat() override;
  void applyJobLevelBonus() override;
  void onUpdateState(State *state, bool bIsExpire) override;
  void onAfterRemoveState(State *state) override;
  void applyPassiveSkillEffect() override;
  void applyState(State &state) override;
  void applyPassiveSkillAmplifyEffect() override;
  void onModifyStatAndAttribute() override;
  void onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio) override;
  void onCompleteCalculateStat() override;
  void applyStatByState() override;
  void onApplyStat() override;

private:
  SummonResourceTemplate *m_tSummonBase{nullptr};
  Player *m_pMaster{nullptr};

  int m_nAccountID{};
};