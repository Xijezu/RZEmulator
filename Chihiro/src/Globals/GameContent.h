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
#include "MonsterBase.h"
#include "Unit.h"

class Monster;
struct MonsterDeleteHandler;
struct DropGroup;
struct WayPointInfo;
class NPC;
struct NPCTemplate;

class GameContent {
public:
    static Monster *RespawnMonster(float x, float y, uint8_t layer, int32_t id, bool is_wandering, int32_t way_point_id, MonsterDeleteHandler *pDeleteHandler, bool bNeedLock);
    static bool IsInRandomPoolMonster(int32_t group_id, int32_t monster_id);
    static bool LearnAllSkill(Player *pPlayer);
    static bool SelectItemIDFromDropGroup(int32_t nDropGroupID, int32_t &nItemID, int64_t &nItemCount);
    static uint16_t IsLearnableSkill(Unit *, int, int, int32_t &);
    static int32_t GetLocationID(float x, float y);
    static bool IsBlocked(float x, float y);
    static bool CollisionToLine(float x1, float y1, float x2, float y2);
    static NPC *GetNewNPC(NPCTemplate *npc_info, uint8_t layer);
    static void AddNPCToWorld();
    static int64_t GetItemSellPrice(int64_t price, int32_t rank, int32_t lv, bool same_price_for_buying);

private:
    static uint16_t isLearnableSkill(Unit *pUnit, int32_t skill_id, int32_t skill_level, int32_t nJobID, int32_t unit_job_level);
    GameContent() = default;
    ~GameContent() = default;
};