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
#include "IpAddress.h"
#include "IoContext.h"
#include "AsioHacksFwd.h"
#include <boost/asio/ip/tcp.hpp>
#include "Log.h"

class XSocket;
class XSocketThread;

namespace NGemity
{

template <class SOCKET_TYPE>
static std::shared_ptr<SOCKET_TYPE> GetXSocket(NGemity::Asio::IoContext &ioContext, const std::string &szIPAddress, uint16_t nPort)
{
    boost::asio::ip::tcp_endpoint endpoint(NGemity::Net::make_address_v4(szIPAddress), nPort);
    boost::asio::ip::tcp::socket socket(ioContext);
    try
    {
        socket.connect(endpoint);
        socket.set_option(boost::asio::ip::tcp::no_delay(true));
    }
    catch (std::exception &)
    {
        NG_LOG_ERROR("server.network", "Cannot connect to game server at %s:%d", szIPAddress.c_str(), nPort);
        return nullptr;
    }

    return std::make_shared<SOCKET_TYPE>(std::move(socket));
}

class SingleSocketInstance
{
  public:
    ~SingleSocketInstance() = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    SingleSocketInstance(const SingleSocketInstance &) = delete;
    SingleSocketInstance &operator=(const SingleSocketInstance &) = delete;

    static SingleSocketInstance &Instance()
    {
        static SingleSocketInstance instance;
        return instance;
    }

    /**
     * @brief Starts the network thread to handle the update
     * Use AddSocket afterwards to automatically start sockets, etc.
     */
    void InitializeSingleSocketInstance();
    /**
     * @brief Adds a socket to the Network thread, will be autosttarted
     * 
     * @param sock Socket to add
     */
    void AddSocket(std::shared_ptr<XSocket> sock);

    /**
     * @brief Stops the SingleSocketInstance
     * 
     */
    void StopSingleSocketInstance();

    int32_t GetConnectionCount();

  private:
    SingleSocketInstance();
    std::unique_ptr<XSocketThread> m_pNetworkThread;
};
} // namespace NGemity