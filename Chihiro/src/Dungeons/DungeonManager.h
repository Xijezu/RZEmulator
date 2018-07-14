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

#ifndef NGEMITY_DUNGEONMANAGER_H
#define NGEMITY_DUNGEONMANAGER_H

#include "X2D/Boxf.h"
#include "Common.h"
#include "Object.h"

struct DungeonTemplate
{
    int       id;
    int       level;
    Position  raid_start_pos;
    Position  siege_start_pos;
    Position  siege_defense_pos;
    Position  core_pos;
    Position  connector_pos;
    int       core_id;
    int       start_time;
    int       end_time;
    int       raid_start_time;
    int       raid_end_time;
    float     core_offset_x;
    float     core_offset_y;
    float     core_offset_z;
    float     core_around_x;
    float     core_around_y;
    float     core_around_z;
    float     core_scale_x;
    float     core_scale_y;
    float     core_scale_z;
    bool      core_is_lock_height;
    float     core_lock_height;
    int       boss_id[2];
    X2D::Boxf box;
    int       connector_id;
    int       owner_guild_id;
    int       raid_guild_id;
    uint      best_raid_time;
    int       original_owner_guild_id;
    uint64    last_global_notice_time;
    uint64    last_dungeon_siege_finish_time;
    uint64    last_dungeon_raid_wrap_up_time;
    bool      bDungeonSiege;
    bool      bDungeonSiegeCreated;
    bool      bDungeonSiegeKicked;
    bool      bDungeonSiegeNeedToDestroy;
    bool      bNeedToChangePosition;
    int       max_guild_party;
    int       max_raid_party;
    int       tax_rate;
};

class DungeonManager
{
    public:
        static DungeonManager &Instance()
        {
            static DungeonManager instance;
            return instance;
        }

        ~DungeonManager() = default;

        /// \brief Gets the startlocation of the dungeon (used for enter_dungeon() for example)
        /// \param nDungeonID DungeonID
        /// \return the correct position
        Position GetRaidStartPosition(int nDungeonID);
        /// \brief Gets the dungeon ID from coordinates
        /// \param x X coord
        /// \param y Y coord
        /// \return DungeonID on success, 0 on failure
        int GetDungeonID(float x, float y);
        /// \brief Registers our template to the list
        void RegisterDungeonTemplate(DungeonTemplate);
        /*Position GetSiegeStartPosition(int nDungeonID);
        Position GetSiegeDefencePosition(int nDungeonID);*/
    private:
        std::vector<DungeonTemplate> m_vDungeonInfo{ };

    protected:
        DungeonManager() = default;
};

#define sDungeonManager DungeonManager::Instance()
#endif // NGEMITY_DUNGEONMANAGER_H
