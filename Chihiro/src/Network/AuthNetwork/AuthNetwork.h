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

#ifndef NGEMITY_AUTHSESSION_H_
#define NGEMITY_AUTHSESSION_H_

#include "Common.h"
#include "Configuration/Config.h"
#include "IoContext.h"
#include "XSocket.h"
#include "GameAuthSession.h"
#include "Encryption/ByteBuffer.h"
#include <boost/asio/deadline_timer.hpp>

class AuthNetwork
{
    public:
        static AuthNetwork &Instance()
        {
            static AuthNetwork instance;
            return instance;
        }

        ~AuthNetwork() = default;

        void Update()
        {
            if (m_bClosed)
                return;

            _updateTimer->expires_from_now(boost::posix_time::milliseconds(10));
            _updateTimer->async_wait(std::bind(&AuthNetwork::Update, this));

            if (!m_pSocket->Update())
            {
                if (m_pSocket->IsOpen())
                    m_pSocket->CloseSocket();
                m_bClosed = true;
            }
        }

        void Stop()
        {
            m_bClosed = true;
            if (m_pThread != nullptr && m_pThread->joinable())
            {
                m_pThread->join();
            }
            _updateTimer->cancel();

            delete _updateTimer;
            delete m_pThread;
        }

        int InitializeNetwork(NGemity::Asio::IoContext &ioContext, std::string const &bindIp, uint16 port)
        {
            boost::asio::ip::tcp_endpoint endpoint(boost::asio::ip::address::from_string(bindIp), port);
            boost::asio::ip::tcp::socket  _socket(ioContext);

            _socket.connect(endpoint);
            _updateTimer = new boost::asio::deadline_timer(ioContext);

            m_pSocket.reset(new XSocket(std::move(_socket)));
            m_pSocket->SetSession(new GameAuthSession{m_pSocket.get()});
            m_pSocket->Start();
            reinterpret_cast<GameAuthSession*>(m_pSocket->GetSession())->SendGameLogin();

            m_pThread = new std::thread(&AuthNetwork::Run, this);
            return 0;
        }

        void Run()
        {
            _updateTimer->expires_from_now(boost::posix_time::milliseconds(10));
            _updateTimer->async_wait(std::bind(&AuthNetwork::Update, this));

            Update();
        }

        void SendAccountToAuth(WorldSession &session, const std::string &login_name, uint64 one_time_key)
        {
            reinterpret_cast<GameAuthSession *>(m_pSocket->GetSession())->AccountToAuth(&session, login_name, one_time_key);
        }

        void SendClientLogoutToAuth(const std::string &account)
        {
            reinterpret_cast<GameAuthSession *>(m_pSocket->GetSession())->ClientLogoutToAuth(account);
        }

    private:
        bool                     m_bClosed;
        std::shared_ptr<XSocket> m_pSocket;
        boost::asio::deadline_timer *_updateTimer;
        std::thread *m_pThread;

    protected:
        AuthNetwork() : m_bClosed(false), m_pSocket(nullptr), m_pThread(nullptr), _updateTimer(nullptr)
        {

        }
};

#define sAuthNetwork AuthNetwork::Instance()
#endif // NGEMITY_AUTHSESSION_H_