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

class GameRule
{
public:
  static float GetItemLevelPenalty(int creature_level, int item_rank, int item_level);
  static int GetItemRecommendedLevel(int item_rank, int item_level);
  static int GetItemLevelLimit(int item_rank);
  static int GetItemRecommendModTable(int item_rank);
  static int GetRankLevel(int rank);
  static int GetLocalFlag();
  static float GetItemDropRate();
  static float GetGoldDropRate();
  static float GetChaosDropRate();
  static float GetEXPRate();
  static float GetItemValue(float, int, int, int, int);
  static float GetPickableRange();
  static int GetChipLevelLimit(int idx);

  static int GetMaxLevel() { return 150; }

  static int GetIntValueByRandomInt(double fValue);
  static int64 GetIntValueByRandomInt64(double fValue);
  static float GetStaminaRatio(int level);
  static float GetStaminaBonus();
  static int nEnhanceFailType;
  constexpr static float DEFAULT_UNIT_SIZE{12.0f};
  constexpr static int SKILL_CAST_COST{0};
  constexpr static int ATTACK_RANGE_UNIT{100};
  constexpr static float REFLECT_RANGE{4.0f * DEFAULT_UNIT_SIZE};
  constexpr static int MAX_STATE_LEVEL = 65535;
  constexpr static float fPVPDamageRateForPlayer{0.2f};
  constexpr static float fPVPDamageRateForSummon{0.13f};
  constexpr static float MONSTER_PRESPAWN_RATE{0.5f};

private:
  static int _chipLevelLimit[];
  static int _modtable[];
  static float _staminaExpRate[];
};