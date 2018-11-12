#pragma once
/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "SharedMutex.h"
#include "ClientPackets.h"
#include "Configuration/Config.h"
#include "IoContext.h"
#include "XSocket.h"
#include "GameAuthSession.h"
#include "Encryption/ByteBuffer.h"
#include <boost/asio/deadline_timer.hpp>
#include "NetworkThread.h"

class AuthNetwork
{
    public:
        static AuthNetwork &Instance()
        {
            static AuthNetwork instance;
            return instance;
        }

        ~AuthNetwork()
        {
            if(!m_bClosed)
            {
                Stop();
            }
        }

        void Update()
        {
            if (m_bClosed)
                return;

            if(!m_pSocket->IsOpen())
                m_bClosed = true;

            if (m_nLastPingTime + 18000 < m_nLastPingTime && m_pSocket && m_pSocket->IsOpen())
            {
                m_nLastPingTime = sWorld.GetArTime();
                TS_CS_PING ping{};
                m_pSocket->SendPacket(ping);
            }

            std::this_thread::sleep_for(std::chrono::seconds(20));
        }

        void Stop()
        {
            NG_UNIQUE_GUARD _guard(_mutex);
            m_bClosed = true;
            if (m_pThread != nullptr && m_pThread->joinable())
            {
                m_pThread->join();
                delete m_pThread;
            }

            if(m_pNetworkThread != nullptr)
                delete m_pNetworkThread;
            if (m_pSocket != nullptr)
                m_pSocket->DeleteSession();
        }

        bool InitializeNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16 port)
        {
            NG_UNIQUE_GUARD               _guard(_mutex);
            boost::asio::ip::tcp_endpoint endpoint(boost::asio::ip::make_address_v4(bindIp), port);
            boost::asio::ip::tcp::socket socket(ioContext);
            m_pNetworkThread = new NetworkThread<XSocket>{};

            try
            {
                socket.connect(endpoint);
                socket.set_option(boost::asio::ip::tcp::no_delay(true));
            }
            catch (std::exception &)
            {
                NG_LOG_ERROR("server.network", "Cannot connect to login server at %s:%d", bindIp.c_str(), port);
                return false;
            }

            m_pSocket.reset(new XSocket(std::move(socket)));
            m_pSocket->SetSendBufferSize(sConfigMgr->GetIntDefault("Network.OutUBuff", 65536));
            m_pSocket->SetSession(new GameAuthSession{m_pSocket.get()});

            m_pNetworkThread->AddSocket(m_pSocket);
            m_pSocket->Start();
            m_pNetworkThread->Start();

            reinterpret_cast<GameAuthSession *>(m_pSocket->GetSession())->SendGameLogin();

            m_pThread = new std::thread(&AuthNetwork::Update, this);
            return true;
        }

        void SendAccountToAuth(WorldSession &session, const std::string &login_name, uint64 one_time_key)
        {
            NG_UNIQUE_GUARD _guard(_mutex);
            reinterpret_cast<GameAuthSession *>(m_pSocket->GetSession())->AccountToAuth(&session, login_name, one_time_key);
        }

        void SendClientLogoutToAuth(const std::string &account)
        {
            NG_UNIQUE_GUARD _guard(_mutex);
            reinterpret_cast<GameAuthSession *>(m_pSocket->GetSession())->ClientLogoutToAuth(account);
        }

    private:
        bool                           m_bClosed;
        std::shared_ptr<XSocket>       m_pSocket;
        std::thread                    *m_pThread;
        std::atomic<uint32>            m_nLastPingTime;
        NetworkThread<XSocket> *m_pNetworkThread;
        NG_SHARED_MUTEX _mutex;

    protected:
        AuthNetwork() : m_bClosed(false), m_pSocket(nullptr), m_pThread(nullptr), m_nLastPingTime(0), m_pNetworkThread(nullptr)
        {
        };
};

#define sAuthNetwork AuthNetwork::Instance()