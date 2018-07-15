/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

/** \addtogroup u2w User to World Communication
 *  @{
 *  \file WorldSocketMgr.h
 *  \author Derex <derex101@gmail.com>
 */

#ifndef __AUTHGAMESOCKETMGR_H
#define __AUTHGAMESOCKETMGR_H

#include "SocketMgr.h"
#include "XSocket.h"
#include "AuthGameSession.h"

/// Manages all sockets connected to peers and network threads
class AuthGameSocketMgr : public SocketMgr<XSocket>
{
        typedef SocketMgr<XSocket> BaseSocketMgr;

    public:
        ~AuthGameSocketMgr() = default;

        static AuthGameSocketMgr &Instance();

/// Start network, listen at address:port .
        bool StartWorldNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16 port, int networkThreads);

/// Stops all network threads, It will wait for all running threads .
        void StopNetwork() override;

        void OnSocketOpen(tcp::socket &&sock, uint32 threadIndex) override;

        std::size_t GetApplicationSendBufferSize() const { return _socketApplicationSendBufferSize; }

    protected:
        AuthGameSocketMgr();

        NetworkThread<XSocket> *CreateThreads() const override;

    private:
// private, must not be called directly
        bool StartNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16 port, int threadCount) override
        {
            return BaseSocketMgr::StartNetwork(ioContext, bindIp, port, threadCount);
        }

        int32 _socketSystemSendBufferSize;
        int32 _socketApplicationSendBufferSize;
        bool  _tcpNoDelay;
};

#define sAGSocketMgr AuthGameSocketMgr::Instance()

#endif
/// @}
