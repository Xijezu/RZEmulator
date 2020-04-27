/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
 * Partial implementation taken from glandu2 at https://github.com/glandu2/librzuxi
 *
 */

#include "MonitorSession.h"

#include <iostream>

#include "Common.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "ServerMonitor.h"
#include "cipher/XStrZlibWithSimpleCipherUtil.h"

enum eStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct
{
    int cmd;
    eStatus status;
    std::function<void(MonitorSession *, XPacket *)> handler;
} PacketHandler;

template<typename T>
PacketHandler declareHandler(eStatus status, void (MonitorSession::*handler)(const T *packet))
{
    PacketHandler handlerData{};
    handlerData.cmd = sConfigMgr->GetPacketVersion();
    handlerData.handler = [handler](MonitorSession *instance, XPacket *packet) -> void {
        T deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };
    return handlerData;
}

const PacketHandler _PacketHandler[] = {{declareHandler(STATUS_CONNECTED, &MonitorSession::onResultHandler)}};

constexpr int PacketTableSize = (sizeof(_PacketHandler) / sizeof(PacketHandler));

ReadDataHandlerResult MonitorSession::ProcessIncoming(XPacket *pRecvPct)
{
    ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int i = 0;

    for (i = 0; i < PacketTableSize; i++)
    {
        if ((uint16_t)_PacketHandler[i].cmd == _cmd)
        {
            _PacketHandler[i].handler(this, pRecvPct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == PacketTableSize)
    {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Error;
    }
    return ReadDataHandlerResult::Ok;
}

void MonitorSession::OnClose() {}

void MonitorSession::DoRequest()
{
    if (bRequesterEnabled != nullptr)
    {
        TS_CS_REQUEST requestPct{};
        requestPct.t = 'u';
        requestPct.command = XStrZlibWithSimpleCipherUtil::Encrypt("SELECT TOP 1 id FROM Character");
        SendPacket(requestPct);
    }

    if (pUserCount != nullptr)
    {
        TS_CS_VERSION versionPct{};
        versionPct.szVersion = sConfigMgr->GetStringDefault("monitor.version", "ASER");
        SendPacket(versionPct);
    }
}

void MonitorSession::onResultHandler(const TS_SC_RESULT *resultPct)
{
    if (resultPct->getReceivedId() == 60 && bRequesterEnabled != nullptr)
    {
        *bRequesterEnabled = true;
    }
    else if (pUserCount != nullptr)
    {
        *pUserCount = resultPct->value ^ 0xADADADAD;
        auto stmt = LogDatabase.GetPreparedStatement(LOG_REP_USER_COUNT);
        stmt->setString(0, m_Server.szName);
        stmt->setInt32(1, *pUserCount);
        LogDatabase.Execute(stmt);
    }
    CloseSocket();
}

bool MonitorSession::Finished()
{
    bool bRet = sServerMonitor.GetTime() > m_nLastUpdateTime + 10000 || !IsOpen();
    if (bRet && IsOpen())
        CloseSocket();
    return bRet;
}

MonitorSession::MonitorSession(boost::asio::ip::tcp::socket &&socket, int *ppUserCount, bool *ppRequester, NGemity::Server server)
    : XSocket(std::move(socket))
    , m_nLastUpdateTime(sServerMonitor.GetTime())
    , pUserCount(ppUserCount)
    , bRequesterEnabled(ppRequester)
    , m_Server(server)
{
}

MonitorSession::~MonitorSession() {}