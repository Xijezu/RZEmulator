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

#include <chrono>

#include "Log.h"

static constexpr int32_t WEEK_TIME_IN_SECOND = 3600 * 24 * 7;

time_t GetWeekBeginTime()
{
    auto timePoint = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    auto dayPoint = std::chrono::floor<std::chrono::days>(timePoint);
    auto weekday = std::chrono::weekday{dayPoint};

    dayPoint -= weekday - std::chrono::Monday;
    return std::chrono::system_clock::to_time_t(dayPoint);
}

int32_t DungeonManager::GetDungeonID(float_t x, float_t y) const
{
    for (const auto &dt : m_vDungeonInfo) {
        if (dt.box.IsInclude(x, y))
            return dt.id;
    }
    return 0;
}

void DungeonManager::RegisterDungeonTemplate(DungeonTemplate pTemplate)
{
    m_vDungeonInfo.emplace_back(pTemplate);
}


time_t DungeonManager::GetDungeonRaidStartTime(int32_t nStartTime)
{
    return GetWeekBeginTime() + nStartTime;
}

time_t DungeonManager::GetDungeonRaidEndTime(int32_t nEndTime)
{
    return GetWeekBeginTime() + nEndTime;
}

time_t DungeonManager::GetNextDungeonSiegeStartTime(int32_t nStartTime, time_t lastDungeonSiegeFinishTime)
{
    time_t dungeonSiegeStartTime = GetWeekBeginTime() + nStartTime;

    while (dungeonSiegeStartTime <= lastDungeonSiegeFinishTime)
        dungeonSiegeStartTime += WEEK_TIME_IN_SECOND;

    return dungeonSiegeStartTime;
}

time_t DungeonManager::GetNextDungeonSiegeEndTime(int nEndTime, time_t dungeonSiegeStartTime)
{
    time_t dungeonSiegeEndTime = GetWeekBeginTime() + nEndTime;

    while (dungeonSiegeEndTime <= dungeonSiegeStartTime)
        dungeonSiegeEndTime += WEEK_TIME_IN_SECOND;

    return dungeonSiegeEndTime;
}

Position DungeonManager::GetRaidStartPosition(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.raid_start_pos;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon Setting: %d - DungeonManager::GetRaidStartPosition", nDungeonID);
    return Position{};
}

Position DungeonManager::GetSiegeStartPosition(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.siege_start_pos;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon Setting: %d - DungeonManager::GetSiegeStartPosition", nDungeonID);
    return Position{};
}

Position DungeonManager::GetSiegeDefencePosition(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.siege_defense_pos;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon: %d - DungeonManager::GetSiegeDefencePosition", nDungeonID);
    return Position{};
}

int32_t DungeonManager::GetDungeonLevel(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.level;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon: %d - DungeonManager::GetDungeonLevel", nDungeonID);
    return 0;
}

int32_t DungeonManager::GetMaxRaidParty(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.max_raid_party;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon: %d - DungeonManager::GetMaxRaidParty", nDungeonID);
    return 0;
}

int32_t DungeonManager::GetMaxGuildParty(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id == nDungeonID)
            return dungeon.max_guild_party;
    }
    NG_LOG_ERROR("server.dungeon", "Invalid Dungeon: %d - DungeonManager::GetMaxGuildParty", nDungeonID);
    return 0;
}

bool DungeonManager::BeginDungeonRaid(int32_t nDungeonID, int32_t nGuildID)
{
    ASSERT(0, "NOT IMPLEMENTED");
    return false;
}

uint8_t DungeonManager::GetRaidDungeonLayer(int32_t nDungeonID, int32_t nGuildID)
{
    ASSERT(0, "NOT IMPLEMENTED");
    return 0;
}

bool DungeonManager::IsRaidBegin(int32_t nDungeonID, int32_t nGuildID)
{
    ASSERT(0, "NOT IMPLEMENTED");
    return false;
}

bool DungeonManager::IsDungeonRaidTime(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id != nDungeonID)
            continue;

        auto currTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (currTime > GetDungeonRaidEndTime(dungeon.raid_end_time) || currTime < GetDungeonRaidStartTime(dungeon.raid_start_time))
            return false;

        return true;
    }

    return false;
}

bool DungeonManager::IsDungeonSiegeAttackteamMakingPeriod(int32_t nDungeonID)
{
    for (const auto &dungeon : m_vDungeonInfo) {
        if (dungeon.id != nDungeonID)
            continue;

        auto currTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if (currTime >= GetDungeonRaidEndTime(dungeon.raid_end_time) && currTime <= GetNextDungeonSiegeEndTime(dungeon.end_time, GetWeekBeginTime()))
            return true;

        return false;
    }

    return false;
}
