#pragma once
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
 * Partial implementation taken from glandu2 at https://github.com/glandu2/librzu
 *
 */
#include <iostream>

#include "Common.h"
#include "MonitorStructs.h"
#include "RSACipher.h"
#include "XSocket.h"

class XPacket;
struct Server;
// Handle login commands
class MonitorSession : public XSocket {
public:
    explicit MonitorSession(boost::asio::ip::tcp::socket &&socket, int *ppUserCount, bool *ppRequester, NGemity::Server server);
    ~MonitorSession();

    // Network handlers
    void OnClose() override;
    ReadDataHandlerResult ProcessIncoming(XPacket *) override;
    bool IsEncrypted() const override { return true; }
    void DoRequest();
    void onResultHandler(const TS_SC_RESULT *resultPct);
    bool Finished();

private:
    uint32_t m_nLastUpdateTime;
    int *pUserCount{nullptr};
    bool *bRequesterEnabled{nullptr};
    std::unique_ptr<RsaCipher> m_pCipher;
    NGemity::Server m_Server;
};