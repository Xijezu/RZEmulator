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
 * 
 * Partial implementation taken from glandu2 at https://github.com/glandu2/librzu
 * 
*/

#include "Common.h"
#include "IoContext.h"
#include "MonitorSession.h"
#include "SingleSocketInstance.h"
#include <iostream>

class MonitorSocket
{
  public:
    MonitorSocket(const std::string &pszIP, uint16_t pnPort, int *nUserCount, bool *bbRequesterEnabled, NGemity::Asio::IoContext &pIOContext) : _socket(pIOContext), pUserCount(nUserCount), bRequesterEnabled(bbRequesterEnabled)
    {
        boost::asio::ip::tcp_endpoint endpoint(boost::asio::ip::make_address_v4(pszIP), pnPort);
        _socket.async_connect(endpoint, std::bind(&MonitorSocket::ConnectHandler, this, std::placeholders::_1));
    }

    ~MonitorSocket() = default;

  private:
    void ConnectHandler(const boost::system::error_code &ec)
    {
        if (!ec)
        {
            std::shared_ptr<MonitorSession> pSocket = std::make_shared<MonitorSession>(std::move(_socket));
            NGemity::SingleSocketInstance::Instance().AddSocket(pSocket);
            pSocket->DoRequest(pUserCount, bRequesterEnabled);
            delete this;
        }
        else
        {
            *pUserCount = -1;
            delete this;
        }
    }

    boost::asio::ip::tcp::socket _socket;
    std::string _szIPAddress;
    int *pUserCount;
    bool *bRequesterEnabled;
    uint16_t _nPort;
};