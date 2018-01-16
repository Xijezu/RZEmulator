/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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

#include <Entities/Player/Player.h>
#include "ClientPackets.h"

WorldLocation::WorldLocation(const WorldLocation& src)
{
    idx           = src.idx;
    location_type = src.location_type;
    for (int x = 0; x < 7; ++x) {
        for (int y = 0; y < 4; ++y) {
            weather_ratio[x][y] = src.weather_ratio[x][y];
        }
    }
    current_weather     = src.current_weather;
    weather_change_time = src.weather_change_time;
    last_changed_time   = src.last_changed_time;
    shovelable_item     = src.shovelable_item;
    m_vIncludeClient    = src.m_vIncludeClient;
}

WorldLocation *WorldLocationManager::AddToLocation(uint idx, Player *player)
{
    if (player->m_WorldLocation != nullptr)
        RemoveFromLocation(player);

    for(int i = 0; i < m_vWorldLocation.size(); i++)
    {
        if (m_vWorldLocation[i].idx == idx) {
            m_vWorldLocation[i].m_vIncludeClient.emplace_back(player);
            XPacket worldLocationPct(TS_SC_WEATHER_INFO);
            worldLocationPct << (uint32_t)idx;
            worldLocationPct << (uint16_t)m_vWorldLocation[i].current_weather;
            player->SendPacket(worldLocationPct);
            return &m_vWorldLocation[i];
        }
    }
    return nullptr;
}

bool WorldLocationManager::RemoveFromLocation(Player *player)
{
        for(auto& wl : m_vWorldLocation)
        {
            if(player->m_WorldLocation == &wl)
            {
               wl.m_vIncludeClient.erase(std::remove(wl.m_vIncludeClient.begin(),
                                                     wl.m_vIncludeClient.end(), player),
                                         wl.m_vIncludeClient.end());
                return true;
            }
    }
    return false;
}

void WorldLocationManager::SendWeatherInfo(uint idx, Player *player)
{
    if(player == nullptr)
        return;
    for(auto& wl : m_vWorldLocation)
    {
        if(wl.idx == idx)
        {
            XPacket weatherPct(TS_SC_WEATHER_INFO);
            weatherPct << (uint32_t)idx;
            weatherPct << (uint16_t)wl.current_weather;
            player->SendPacket(weatherPct);
            return;
        }
    }
}

int WorldLocationManager::GetShovelableItem(uint idx)
{
    for(auto& wl : m_vWorldLocation)
    {
        if(wl.idx == idx)
        {
            return wl.shovelable_item;
        }
    }
}

uint WorldLocationManager::GetShovelableMonster(uint idx)
{
    return 0;
}

void WorldLocationManager::RegisterWorldLocation(uint idx, uint8_t location_type, uint time_id, uint weather_id, uint8_t ratio, uint weather_change_time, int shovelable_item)
{
    for(auto& wl : m_vWorldLocation) {
        if (wl.idx == idx) {
            wl.weather_ratio[weather_id][time_id] = ratio;
            wl.shovelable_item = shovelable_item;
            return;
        }
    }

    WorldLocation nwl { };
    nwl.idx = idx;
    nwl.location_type = location_type;
    nwl.weather_ratio[weather_id][time_id] = ratio;
    nwl.weather_change_time = weather_change_time;
    nwl.shovelable_item = shovelable_item;

    m_vWorldLocation.emplace_back(nwl);
}

void WorldLocationManager::RegisterMonsterLocation(uint idx, uint monster_id)
{
    std::vector<uint> ml{ };

    if (m_hsMonsterID.count(idx) >= 1) {
        ml = m_hsMonsterID[idx];
    } else {
        m_hsMonsterID[idx] = ml;
    }

    for (auto &id :  ml) {
        if (id == monster_id)
            return;
    }
    ml.emplace_back(monster_id);
}