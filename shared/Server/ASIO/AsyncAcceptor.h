#pragma once
/*
 * Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <atomic>
#include <functional>

#include <boost/asio/ip/tcp.hpp>

#include "IoContext.h"
#include "IpAddress.h"
#include "Log.h"

using boost::asio::ip::tcp;

#if BOOST_VERSION >= 106600
#define NGEMITY_MAX_LISTEN_CONNECTIONS boost::asio::socket_base::max_listen_connections
#else
#define NGEMITY_MAX_LISTEN_CONNECTIONS boost::asio::socket_base::max_connections
#endif

class AsyncAcceptor
{
    using SocketHandler = std::function<void(tcp::socket &&sock, uint32_t threadIndex)>;

public:
    // typedef void (*AcceptCallback)(tcp::socket &&newSocket, uint32_t threadIndex);

    AsyncAcceptor(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16_t port);
    void AsyncAccept();

    // template <AcceptCallback acceptCallback>
    void AsyncAcceptWithCallback(SocketHandler handler)
    {
        tcp::socket *socket;
        uint32_t threadIndex;
        std::tie(socket, threadIndex) = _socketFactory();
        _acceptor.async_accept(*socket, [this, socket, threadIndex, handler](boost::system::error_code error) {
            if (!error)
            {
                try
                {
                    socket->non_blocking(true);

                    (handler)(std::move(*socket), threadIndex);
                }
                catch (boost::system::system_error const &err)
                {
                    NG_LOG_INFO("network", "Failed to initialize client's socket %s", err.what());
                }
            }

            if (!_closed)
                this->AsyncAcceptWithCallback((handler));
        });
    }

    bool Bind();
    void Close();

    void SetSocketFactory(std::function<std::pair<tcp::socket *, uint32_t>()> func) { _socketFactory = func; }

private:
    std::pair<tcp::socket *, uint32_t> DefeaultSocketFactory() { return std::make_pair(&_socket, 0); }

    tcp::acceptor _acceptor;
    tcp::endpoint _endpoint;
    tcp::socket _socket;
    std::atomic<bool> _closed;
    std::function<std::pair<tcp::socket *, uint32_t>()> _socketFactory;
};