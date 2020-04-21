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
 * 
 * Partial implementation taken from glandu2 at https://github.com/glandu2/librzu
 * 
*/
#include "Common.h"
#include "MonitorSession.h"
#include "MonitorStructs.h"
#include "NetworkThread.h"
#include <mutex>
#include <string>

namespace NGemity
{
class ServerMonitor
{
  public:
    static ServerMonitor &Instance()
    {
        static ServerMonitor instance;
        return instance;
    }

    void InitializeServerMonitor();
    void InitializeMonitoring(std::shared_ptr<NGemity::Asio::IoContext> pIoContext);
    void Update();

    std::string GetEverything();
    uint32_t GetTime() { return GetMSTimeDiffToNow(m_nStartTime) / 10; }

    const std::vector<NGemity::ServerRegion> &GetServerList() const { return m_vServerRegion; }

  private:
    std::vector<NGemity::ServerRegion> m_vServerRegion{};
    std::shared_ptr<NGemity::Asio::IoContext> _ioContext;
    std::shared_ptr<boost::asio::deadline_timer> _updateTimer;
    bool _stopped;

  protected:
    ServerMonitor();
    uint32_t m_nStartTime{};
    std::unordered_map<std::string, std::shared_ptr<MonitorSession>> vSockets{};
    std::chrono::steady_clock::time_point lastRequester;
    std::mutex m_pMutex;
};
} // namespace NGemity
#define sServerMonitor NGemity::ServerMonitor::Instance()