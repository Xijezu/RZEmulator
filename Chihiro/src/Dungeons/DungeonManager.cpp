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

#include "DungeonManager.h"

Position DungeonManager::GetRaidStartPosition(int32_t nDungeonID)
{
    for (auto &dt : m_vDungeonInfo) {
        if (dt.id == nDungeonID)
            return dt.raid_start_pos;
    }
    return Position{};
}

int32_t DungeonManager::GetDungeonID(float_t x, float_t y)
{
    for (auto &dt : m_vDungeonInfo) {
        if (dt.box.IsInclude(x, y))
            return dt.id;
    }
    return 0;
}

void DungeonManager::RegisterDungeonTemplate(DungeonTemplate pTemplate)
{
    m_vDungeonInfo.emplace_back(pTemplate);
}
