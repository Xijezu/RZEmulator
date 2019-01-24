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
#include "MonsterBase.h"

class Monster;
struct MonsterDeleteHandler;
struct DropGroup;
struct WayPointInfo;
class NPC;
struct NPCTemplate;

class GameContent
{
    public:
        static Monster *RespawnMonster(float x, float y, uint8_t layer, int id, bool is_wandering, int way_point_id, MonsterDeleteHandler *pDeleteHandler, bool bNeedLock);
        static bool IsInRandomPoolMonster(int group_id, int monster_id);
        static bool LearnAllSkill(Player *pPlayer);
        static bool SelectItemIDFromDropGroup(int nDropGroupID, int &nItemID, int64_t &nItemCount);
        static ushort IsLearnableSkill(Unit *, int, int, int &);
        static int GetLocationID(float x, float y);
        static bool IsBlocked(float x, float y);
        static bool CollisionToLine(float x1, float y1, float x2, float y2);
        static NPC *GetNewNPC(NPCTemplate *npc_info, uint8_t layer);
        static void AddNPCToWorld();
        static int64_t GetItemSellPrice(int64_t price, int rank, int lv, bool same_price_for_buying);
    private:
        static ushort isLearnableSkill(Unit *pUnit, int skill_id, int skill_level, int nJobID, int unit_job_level);
        GameContent() = default;
        ~GameContent() = default;
};