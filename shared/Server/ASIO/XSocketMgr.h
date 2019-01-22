#pragma once
/*
 * Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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

#include "SocketMgr.h"
#include "NetworkThread.h"
#include "Config.h"
/// Manages all sockets connected to peers and network threads

template <class SOCKET_TYPE>
class XSocketMgr : public SocketMgr
{
public:
  XSocketMgr() : SocketMgr(), _socketSystemSendBufferSize(-1), _socketApplicationSendBufferSize(65536), _tcpNoDelay(true)
  {
  }

  ~XSocketMgr() = default;

  /// Start network, listen at address:port .
  bool StartWorldNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16_t port, int threadCount)
  {
    _tcpNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

    int const max_connections = NGEMITY_MAX_LISTEN_CONNECTIONS;
    NG_LOG_DEBUG("misc", "Max allowed socket connections %d", max_connections);

    _socketSystemSendBufferSize = sConfigMgr->GetIntDefault("Network.OutKBuff", -1);
    _socketApplicationSendBufferSize = sConfigMgr->GetIntDefault("Network.OutUBuff", 65536);

    if (_socketApplicationSendBufferSize <= 0)
    {
      NG_LOG_ERROR("misc", "Network.OutUBuff is wrong in your config file");
      return false;
    }

    if (!SocketMgr::StartNetwork(ioContext, bindIp, port, threadCount))
      return false;

    _acceptor->SetSocketFactory(std::bind(&SocketMgr::GetSocketForAccept, this));
    _acceptor->AsyncAcceptWithCallback(std::bind(&XSocketMgr<SOCKET_TYPE>::OnSocketOpen, this, std::placeholders::_1, std::placeholders::_2));

    return true;
  }

  /// Stops all network threads, It will wait for all running threads .
  void StopNetwork() override { SocketMgr::StopNetwork(); }

  void OnSocketOpen(tcp::socket &&sock, uint32_t threadIndex) override
  {
    // set some options here
    if (_socketSystemSendBufferSize >= 0)
    {
      boost::system::error_code err;
      sock.set_option(boost::asio::socket_base::send_buffer_size(_socketSystemSendBufferSize), err);
      if (err && err != boost::system::errc::not_supported)
      {
        NG_LOG_ERROR("network", "WorldSocketMgr::OnSocketOpen sock.set_option(boost::asio::socket_base::send_buffer_size) err = %s", err.message().c_str());
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
    // By default, we're bypassing this function because we dont want a default XSocket, we want a child class
    //SocketMgr::OnSocketOpen(std::forward<tcp::socket>(sock), threadIndex);

    try
    {
      std::shared_ptr<SOCKET_TYPE> newSocket = std::make_shared<SOCKET_TYPE>(std::move(sock));
      _threads[threadIndex].AddSocket(newSocket);
    }
    catch (boost::system::system_error const &err)
    {
      NG_LOG_WARN("network", "Failed to retrieve client's remote address %s", err.what());
    }
  }

  std::size_t GetApplicationSendBufferSize() const { return _socketApplicationSendBufferSize; }

protected:
  NetworkThread *CreateThreads() const override
  {
    return new XSocketThread[GetNetworkThreadCount()];
  }

private:
  // private, must not be called directly
  bool StartNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16 port, int threadCount) override
  {
    return SocketMgr::StartNetwork(ioContext, bindIp, port, threadCount);
  }

  int32_t _socketSystemSendBufferSize;
  int32_t _socketApplicationSendBufferSize;
  bool _tcpNoDelay;
};