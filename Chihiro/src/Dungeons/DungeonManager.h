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
#include "Object.h"
#include "X2D/Boxf.h"

struct DungeonTemplate {
    int32_t id;
    int32_t level;
    Position raid_start_pos;
    Position siege_start_pos;
    Position siege_defense_pos;
    Position core_pos;
    Position connector_pos;
    int32_t core_id;
    int32_t start_time;
    int32_t end_time;
    int32_t raid_start_time;
    int32_t raid_end_time;
    float_t core_offset_x;
    float_t core_offset_y;
    float_t core_offset_z;
    float_t core_around_x;
    float_t core_around_y;
    float_t core_around_z;
    float_t core_scale_x;
    float_t core_scale_y;
    float_t core_scale_z;
    bool core_is_lock_height;
    float_t core_lock_height;
    int32_t boss_id[2];
    X2D::Boxf box;
    int32_t connector_id;
    int32_t owner_guild_id;
    int32_t raid_guild_id;
    uint32_t best_raid_time;
    int32_t original_owner_guild_id;
    uint64_t last_global_notice_time;
    uint64_t last_dungeon_siege_finish_time;
    uint64_t last_dungeon_raid_wrap_up_time;
    bool bDungeonSiege;
    bool bDungeonSiegeCreated;
    bool bDungeonSiegeKicked;
    bool bDungeonSiegeNeedToDestroy;
    bool bNeedToChangePosition;
    int32_t max_guild_party;
    int32_t max_raid_party;
    int32_t tax_rate;
};

struct Position;
class DungeonManager {
public:
    static DungeonManager &Instance()
    {
        static DungeonManager instance;
        return instance;
    }

    ~DungeonManager() = default;

    /// \brief Gets the dungeon ID from coordinates
    /// \param x X coord
    /// \param y Y coord
    /// \return DungeonID on success, 0 on failure
    int32_t GetDungeonID(float_t x, float_t y) const;
    /// \brief Registers our template to the list
    void RegisterDungeonTemplate(DungeonTemplate);

    static time_t GetDungeonRaidStartTime(int32_t nStartTime);
    static time_t GetDungeonRaidEndTime(int32_t nEndtime);
    static time_t GetNextDungeonSiegeStartTime(int32_t nStartTime, time_t lastDungeonSiegeFinishTime);
    static time_t GetNextDungeonSiegeEndTime(int32_t nEndTime, time_t dungeonSiegeStartTime);

    void OnUpdate(uint32_t time);

    Position GetRaidStartPosition(int32_t nDungeonID);
    Position GetSiegeStartPosition(int32_t nDungeonID);
    Position GetSiegeDefencePosition(int32_t nDungeonID);

    int32_t GetDungeonLevel(int32_t nDungeonID);
    int32_t GetMaxRaidParty(int32_t nDungeonID);
    int32_t GetMaxGuildParty(int32_t nDungeonID);

    bool BeginDungeonRaid(int32_t nDungeonID, int32_t nGuildID);
    uint8_t GetRaidDungeonLayer(int32_t nDungeonID, int32_t nGuildID);
    bool IsRaidBegin(int32_t nDungeonID, int32_t nGuildID);
    bool IsDungeonRaidTime(int32_t nDungeonID);
    bool IsDungeonSiegeAttackteamMakingPeriod(int32_t nDungeonID);
    /*void CreateDungeonSiege(int32_t nDungeonID);
    void BeginDungeonSiege(int32_t nDungeonID);
    void EndDungeonSiege(int32_t nDungeonID);
    bool DropDungeonOwnership(int32_t nDungeonID);*/

private:
    std::vector<DungeonTemplate> m_vDungeonInfo{};

protected:
    DungeonManager() = default;
};

#define sDungeonManager DungeonManager::Instance()
