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

#include "ShizukeSession.h"

#include "Common.h"
#include "Config.h"

ShizukeSession::~ShizukeSession() {}

enum eStatus { STATUS_CONNECTED = 0, STATUS_AUTHED };

typedef struct {
    int cmd;
    eStatus status;
    std::function<void(ShizukeSession *, XPacket *)> handler;
} ShizukeHandler;

template<typename T>
ShizukeHandler declareHandler(eStatus status, void (ShizukeSession::*handler)(const T *packet))
{
    ShizukeHandler handlerData{};
    handlerData.cmd = sConfigMgr->GetPacketVersion();
    handlerData.handler = [handler](ShizukeSession *instance, XPacket *packet) -> void {
        T deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };
    return handlerData;
}

const ShizukeHandler shizukePacketHandler[] = {{declareHandler(STATUS_CONNECTED, &ShizukeSession::onResultHandler)}};

constexpr int shizukeTableSize = (sizeof(shizukePacketHandler) / sizeof(ShizukeHandler));

ReadDataHandlerResult ShizukeSession::ProcessIncoming(XPacket *pRecvPct)
{
    ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int i = 0;

    for (i = 0; i < shizukeTableSize; i++) {
        if ((uint16_t)shizukePacketHandler[i].cmd == _cmd) {
            shizukePacketHandler[i].handler(this, pRecvPct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == shizukeTableSize) {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Ok;
    }
    return ReadDataHandlerResult::Ok;
}

void ShizukeSession::OnClose()
{
    NG_LOG_ERROR("network", "Authserver has closed connection!");
}

void ShizukeSession::onResultHandler(const TS_SC_RESULT *resultPct)
{
    NG_LOG_INFO("network", "Received result:");
    NG_LOG_INFO("network", "Result: %d", resultPct->result);
    NG_LOG_INFO("network", "Value: %d", (resultPct->value ^ 0xADADADAD));
}