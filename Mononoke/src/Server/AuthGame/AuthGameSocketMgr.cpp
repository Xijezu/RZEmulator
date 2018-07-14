/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
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

#include "AuthGameSocketMgr.h"
#include "Config.h"
#include "NetworkThread.h"
#include "WorldSocket.h"
#include <boost/system/error_code.hpp>

class GameSocketThread : public NetworkThread<XSocket>
{
    public:
        void SocketAdded(std::shared_ptr<XSocket> sock) override
        {
            sock->SetSendBufferSize(sAGSocketMgr.GetApplicationSendBufferSize());
            sock->SetSession(new AuthGameSession{sock.get()});
            sock->Start();
        }

        void SocketRemoved(std::shared_ptr<XSocket> sock) override
        {

        }
};

static void OnSocketAccept(tcp::socket&& sock, uint32 threadIndex)
{
    sAGSocketMgr.OnSocketOpen(std::forward<tcp::socket>(sock), threadIndex);
}

AuthGameSocketMgr::AuthGameSocketMgr() : BaseSocketMgr(), _socketSystemSendBufferSize(-1), _socketApplicationSendBufferSize(65536), _tcpNoDelay(true)
{
}

AuthGameSocketMgr& AuthGameSocketMgr::Instance()
{
    static AuthGameSocketMgr instance;
    return instance;
}

bool AuthGameSocketMgr::StartWorldNetwork(Trinity::Asio::IoContext& ioContext, std::string const& bindIp, uint16 port, int threadCount)
{
    _tcpNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

    int const max_connections = TRINITY_MAX_LISTEN_CONNECTIONS;
    NG_LOG_DEBUG("misc", "Max allowed socket connections %d", max_connections);

    // -1 means use default
    _socketSystemSendBufferSize = sConfigMgr->GetIntDefault("Network.OutKBuff", -1);

    _socketApplicationSendBufferSize = sConfigMgr->GetIntDefault("Network.OutUBuff", 65536);

    if (_socketApplicationSendBufferSize <= 0)
    {
        NG_LOG_ERROR("misc", "Network.OutUBuff is wrong in your config file");
        return false;
    }

    if (!BaseSocketMgr::StartNetwork(ioContext, bindIp, port, threadCount))
        return false;

    _acceptor->SetSocketFactory(std::bind(&BaseSocketMgr::GetSocketForAccept, this));
    _acceptor->AsyncAcceptWithCallback<&OnSocketAccept>();

    return true;
}

void AuthGameSocketMgr::StopNetwork()
{
    BaseSocketMgr::StopNetwork();
}

void AuthGameSocketMgr::OnSocketOpen(tcp::socket&& sock, uint32 threadIndex)
{
    // set some options here
    if (_socketSystemSendBufferSize >= 0)
    {
        boost::system::error_code err;
        sock.set_option(boost::asio::socket_base::send_buffer_size(_socketSystemSendBufferSize), err);
        if (err && err != boost::system::errc::not_supported)
        {
            NG_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen sock.set_option(boost::asio::socket_base::send_buffer_size) err = %s", err.message().c_str());
            return;
        }
    }

    // Set TCP_NODELAY.
    if (_tcpNoDelay)
    {
        boost::system::error_code err;
        sock.set_option(boost::asio::ip::tcp::no_delay(true), err);
        if (err)
        {
            NG_LOG_ERROR("misc", "WorldSocketMgr::OnSocketOpen sock.set_option(boost::asio::ip::tcp::no_delay) err = %s", err.message().c_str());
            return;
        }
    }

    //sock->m_OutBufferSize = static_cast<size_t> (m_SockOutUBuff);

    BaseSocketMgr::OnSocketOpen(std::forward<tcp::socket>(sock), threadIndex);
}

NetworkThread<XSocket> *AuthGameSocketMgr::CreateThreads() const
{
    return new GameSocketThread[GetNetworkThreadCount()];
}
