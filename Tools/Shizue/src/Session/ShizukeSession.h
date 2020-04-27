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
 */
#include "Common.h"
#include "XSocket.h"

class XPacket;

// Handle login commands
class ShizukeSession : public XSocket
{
public:
    explicit ShizukeSession(boost::asio::ip::tcp::socket &&socket)
        : XSocket(std::move(socket))
    {
    }
    ~ShizukeSession();

    // Network handlers
    void OnClose() override;
    ReadDataHandlerResult ProcessIncoming(XPacket *) override;
    bool IsEncrypted() const override { return true; }

    // Packet handlers
    void onResultHandler(const TS_SC_RESULT *);
};