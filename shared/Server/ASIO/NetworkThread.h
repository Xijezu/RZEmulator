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
#include "Define.h"
#include "Errors.h"
#include "IoContext.h"
#include "Log.h"
#include "Timer.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include "XSocket.h"

using boost::asio::ip::tcp;

class NetworkThread
{
public:
  NetworkThread();
  virtual ~NetworkThread();

  bool Start();
  void Stop();
  void Wait();

  int32_t GetConnectionCount() const { return _connections; }
  virtual void AddSocket(std::shared_ptr<XSocket> sock);
  tcp::socket *GetSocketForAccept() { return &_acceptSocket; }

protected:
  virtual void SocketAdded(std::shared_ptr<XSocket> /*sock*/) {}
  virtual void SocketRemoved(std::shared_ptr<XSocket> /*sock*/) {}

  void AddNewSockets();
  void Run();
  void Update();

private:
  typedef std::vector<std::shared_ptr<XSocket>> SocketContainer;

  std::atomic<int32_t> _connections;
  std::atomic<bool> _stopped;

  std::thread *_thread;

  SocketContainer _sockets;

  std::mutex _newSocketsLock;
  SocketContainer _newSockets;

  NGemity::Asio::IoContext _ioContext;
  tcp::socket _acceptSocket;
  boost::asio::deadline_timer _updateTimer;
};

class XSocketThread : public NetworkThread
{
public:
  void SocketAdded(std::shared_ptr<XSocket> sock) override
  {
    sock->Start();
  }

  void SocketRemoved(std::shared_ptr<XSocket> /*sock*/) override
  {
  }
};