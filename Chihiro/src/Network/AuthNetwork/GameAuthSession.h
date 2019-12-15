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
#include "XSocket.h"

class XPacket;
class WorldSession;

// Handle login commands
class GameAuthSession : public XSocket
{
public:
  typedef std::unordered_map<std::string, WorldSession *> AuthQueue;
  explicit GameAuthSession(boost::asio::ip::tcp::socket &&socket);
  ~GameAuthSession();

  // Network handlers
  void OnClose() override;
  ReadDataHandlerResult ProcessIncoming(XPacket *) override;
  bool IsEncrypted() const override { return false; }

  // Packet handlers
  void HandleGameLoginResult(XPacket *);
  void HandleClientLoginResult(XPacket *);
  void HandleClientKick(XPacket *);

  void HandleNullPacket(XPacket *);

  void SendGameLogin();
  void AccountToAuth(WorldSession *pSession, const std::string &szLoginName, uint64_t nOneTimeKey);
  void ClientLogoutToAuth(const std::string &account);

  int32_t GetAccountId() const;
  std::string GetAccountName();

private:
  AuthQueue m_queue;

  uint16_t m_nGameIDX;
  std::string m_szGameName;
  std::string m_szGameSSU;
  bool m_bGameIsAdultServer;
  std::string m_szGameIP;
  int32_t m_nGamePort;
};