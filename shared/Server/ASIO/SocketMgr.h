#pragma once
/*
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
#include <memory>

#include <boost/asio/ip/tcp.hpp>

#include "AsyncAcceptor.h"
#include "Errors.h"
#include "NetworkThread.h"

using boost::asio::ip::tcp;

class SocketMgr {
public:
    virtual ~SocketMgr();

    virtual bool StartNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16_t port, int threadCount);
    virtual void StopNetwork();

    void Wait();

    virtual void OnSocketOpen(tcp::socket &&sock, uint32_t nThreadIndex);

    int32_t GetNetworkThreadCount() const { return _threadCount; }
    uint32_t SelectThreadWithMinConnections() const;
    std::pair<tcp::socket *, uint32_t> GetSocketForAccept();

protected:
    SocketMgr();
    virtual NetworkThread *CreateThreads() const = 0;

    AsyncAcceptor *_acceptor;
    NetworkThread *_threads;
    int32_t _threadCount;
    uint32_t m_nSocketMgrIdx;
};