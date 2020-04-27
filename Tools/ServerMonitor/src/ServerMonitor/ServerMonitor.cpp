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

#include "ServerMonitor.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "SingleSocketInstance.h"

NGemity::ServerMonitor::ServerMonitor()
    : _stopped(true)
    , m_nStartTime(getMSTime())
{
}

void NGemity::ServerMonitor::InitializeServerMonitor()
{
    using json = nlohmann::json;
    std::ifstream inFile(sConfigMgr->GetStringDefault("monitor.serverlist", "servers.json"), std::ios::in);
    json j = json::parse(inFile);
    inFile.close();

    for (auto &region : j["region"])
    {
        ServerRegion serverRegion{};
        serverRegion.szRegionName = region["name"].get<std::string>();
        serverRegion.szAuthIPAddress = region["auth"].get<std::string>();
        for (auto &vServer : region["servers"])
        {
            Server server{};
            server.szIPAddress = vServer["ip"].get<std::string>();
            server.szName = vServer["name"].get<std::string>();
            server.nPort = vServer["port"].get<uint16_t>();
            serverRegion.vServerList.emplace_back(server);
        }
        m_vServerRegion.emplace_back(serverRegion);
    }
}

void NGemity::ServerMonitor::InitializeMonitoring(std::shared_ptr<NGemity::Asio::IoContext> pIoContext)
{
    if (!_stopped)
        return;
    _stopped = false;
    _ioContext = pIoContext;
    _updateTimer = std::make_shared<boost::asio::deadline_timer>((*_ioContext.get()));
    _updateTimer->expires_from_now(boost::posix_time::seconds(1));
    _updateTimer->async_wait(std::bind(&NGemity::ServerMonitor::Update, this));
    _ioContext->run();
}

void NGemity::ServerMonitor::Update()
{
    if (_stopped)
        return;

    _updateTimer->expires_from_now(boost::posix_time::minutes(ConfigMgr::instance()->GetIntDefault("monitor.tick", 1)));
    _updateTimer->async_wait(std::bind(&NGemity::ServerMonitor::Update, this));

    std::ofstream outFile(ConfigMgr::instance()->GetStringDefault("monitor.outfile", "/tmp/currlist.json"));
    outFile << GetEverything();
    outFile.close();

    // Remove finished sockets
    for (auto it = std::begin(vSockets); it != std::end(vSockets);)
    {
        if (it->second->Finished())
            it = vSockets.erase(it);
        else
            ++it;
    }

    for (auto &serverRegion : m_vServerRegion)
    {
        for (auto &server : serverRegion.vServerList)
        {

            if (vSockets.count(server.szName) != 0)
                continue;

            boost::asio::ip::tcp_endpoint endpoint(NGemity::Net::make_address_v4(server.szIPAddress), server.nPort);
            boost::asio::ip::tcp::socket socket(*(_ioContext.get()));
            try
            {
                socket.connect(endpoint);
                socket.set_option(boost::asio::ip::tcp::no_delay(true));
            }
            catch (std::exception &)
            {
                NG_LOG_ERROR("network", "Cannot connect to game server at %s:%d", server.szIPAddress.c_str(), server.nPort);
                continue;
            }

            if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - lastRequester) > std::chrono::minutes(30))
                vSockets[server.szName] = std::make_shared<MonitorSession>(std::move(socket), nullptr, &server.bRequesterEnabled, server);
            else
                vSockets[server.szName] = std::make_shared<MonitorSession>(std::move(socket), &server.nPlayerCount, nullptr, server);

            NGemity::SingleSocketInstance::Instance().AddSocket(vSockets[server.szName]);
            vSockets[server.szName]->DoRequest();
        }
    }

    NG_LOG_TRACE("network", "Currently %d connections.", NGemity::SingleSocketInstance::Instance().GetConnectionCount());
    lastRequester = std::chrono::steady_clock::now();
}

std::string NGemity::ServerMonitor::GetEverything()
{
    std::string szResult{};
    nlohmann::json root; // ServerList
    root["last_update"] = time(nullptr);
    for (auto &serverRegion : m_vServerRegion)
    {
        nlohmann::json region; // Server
        region["name"] = serverRegion.szRegionName;
        for (auto &server : serverRegion.vServerList)
        {
            nlohmann::json region_server;
            region_server["name"] = server.szName;
            region_server["usercount"] = server.nPlayerCount;
            region_server["requester"] = server.bRequesterEnabled;
            region["server"].push_back(region_server);
        }
        root["servers"].push_back(region);
    }
    return root.dump();
}