/*
  *  Copyright (C) 2016-2016 Xijezu <http://xijezu.com>
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

#include <Database/PreparedStatement.h>
#include "Database/DatabaseEnv.h"
#include "Common.h"
#include "AuthClient/ClientHandler.h"
#include "Lists/GameList.h"

#include "AuthClient/ClientAuthPackets.h"
#include "AuthClient/AuthClientPackets.h"

#include "Encryption/MD5.h"


// Constructor - Initiate the en/decryption as well as the DES key
ClientHandler::ClientHandler(XSocket &socket) : _socket(socket)
{
    _rc4decode.SetKey("}h79q~B%al;k'y $E");
    _rc4encode.SetKey("}h79q~B%al;k'y $E");
    _descipher.Init("MERONG");
}

// Close patch file descriptor before leaving
ClientHandler::~ClientHandler()
{
    _rc4encode.Clear();
    _rc4decode.Clear();
}

void ClientHandler::Decrypt(void *dest, size_t size, bool isPeek)
{
    _rc4decode.Decode(dest, dest, size, isPeek);
}

void ClientHandler::Encrypt(void *dest, size_t size, bool isPeek)
{
    _rc4encode.Encode(dest, dest, size, isPeek);
}

// Accept the connection and print debug stuff.
void ClientHandler::OnAccept()
{
    MX_LOG_DEBUG("network", "Accepting connection from '%s:%d'", _socket.getRemoteAddress().c_str(), _socket.getRemotePort());
}

void ClientHandler::OnClose()
{
    MX_LOG_DEBUG("network","Disconnected: '%s:%d'", _socket.getRemoteAddress().c_str(), _socket.getRemotePort());
}

enum eStatus {
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct AuthHandler {
    uint16_t cmd;
    uint8_t status;

    bool (ClientHandler::*handler)(XPacket *);
} AuthHandler;

const AuthHandler packetHandler[] =
        {
                {TS_CA_VERSION,       STATUS_CONNECTED, &ClientHandler::HandleVersion},
                {TS_CA_PING,          STATUS_CONNECTED, &ClientHandler::HandleNULL},
                {TS_CA_ACCOUNT,       STATUS_CONNECTED, &ClientHandler::HandleLoginPacket},
                {TS_CA_SERVER_LIST,   STATUS_AUTHED,    &ClientHandler::HandleServerList},
                {TS_CA_SELECT_SERVER, STATUS_AUTHED,    &ClientHandler::HandleSelectServer}
        };

const int tableSize = sizeof(packetHandler) / sizeof(AuthHandler);

// Proccess the incoming packets
void ClientHandler::ProcessIncoming(XPacket *_packet)
{
    ACE_ASSERT(_packet);

    // Manage memory
    ACE_Auto_Ptr<XPacket> aptr(_packet);

    auto _cmd = _packet->GetPacketID();
    int i = 0;

    for (i = 0; i < tableSize; i++) {
        if ((uint16_t) packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (_bIsAuthed && packetHandler[i].status == STATUS_AUTHED))) {
            MX_LOG_DEBUG("server.authserver", "Got data for id %u recv length %u", (uint32) _cmd, (uint32) _packet->size());

            if (!(*this.*packetHandler[i].handler)(_packet)) {
                MX_LOG_DEBUG("server.authserver", "Command handler failed for id %u recv length %u", (uint32) _cmd, (uint32) _packet->size());
                return;
            }
            break;
        }
    }

    // Report unknown packets in th e error log
    if (i == tableSize) {
        MX_LOG_DEBUG("server.authserver", "Got unknown packet '%d' from '%s'", _packet->GetPacketID(), _socket.getRemoteAddress().c_str());
        return;
    }
    aptr.release();
}

bool ClientHandler::HandleVersion(XPacket *_packet)
{
    std::string version = ((CA_VERSION *) _packet->contents())->szVersion;
    MX_LOG_DEBUG("network", "[Version] Client version is %s", version.c_str());
    return true;
}

bool ClientHandler::HandleServerList(XPacket *_packet)
{
    XPacket packet(ACPACKETS::TS_AC_SERVER_LIST);
    packet << (uint16_t) 0;
    packet << (uint16_t) sGameMapList->size();
    for (auto const &x : sGameMapList->GetMap()) {
        packet << (uint16_t) x.second.server_idx;
        packet.fill(x.second.server_name, 21);
        packet << (uint8) (x.second.is_adult_server ? 1 : 0);
        packet.fill(x.second.server_screenshot_url, 256);
        packet.fill(x.second.server_ip, 16);
        packet << (int32) x.second.server_port;
        packet << (uint16_t) 0;
    }
    _socket.SendPacket(packet);
}

bool ClientHandler::HandleLoginPacket(XPacket *packet)
{
    CA_ACCOUNT *accountPct = ((CA_ACCOUNT *) (packet)->contents());
    _descipher.Decrypt(accountPct->password, 32);
    std::string encrypt("2011");
    encrypt.append((char *) accountPct->password);

    // Declare Result packet
    AC_RESULT resultPct = {};
    resultPct.result = 0;
    resultPct.request_msg_id = TS_CA_ACCOUNT;
    resultPct.login_flag = 0;

    // SQL part
    PreparedStatement *stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT);
    stmt->setString(0, std::string(accountPct->account));
    stmt->setString(1, md5(encrypt));
    if (PreparedQueryResult result = LoginDatabase.Query(stmt)) {
        _player.account_id = (*result)[0].GetUInt32();
        _player.login_name = (*result)[1].GetString();
        _player.last_login_server_idx = (*result)[2].GetUInt32();
        _player.block = (*result)[3].GetBool();
        _player.isInGame = false;
        if (_player.block) {
            resultPct.result = 0x6; // Banned, little piece of shit
        } else if (true/*!sPlayerMapList->contains(_player.login_name) || !sPlayerMapList->GetMap()[_player.login_name].isInGame*/) {
            if (sPlayerMapList->contains(_player.login_name)) {
                // We're removing him because values might have changed
                sPlayerMapList->RemovePlayer(_player.login_name);
            }
            _bIsAuthed = true;
            sPlayerMapList->AddPlayer(_player);
            resultPct.login_flag = 1;
        } else {
            resultPct.result = 0x9; // "Already connected"
        }
    } else {
        // "User not found"
        resultPct.result = 1;
    }
    XPacket _resPack(ACPACKETS::TS_AC_RESULT);
    _resPack << (uint16_t) resultPct.request_msg_id;
    _resPack << (uint16_t) resultPct.result;
    _resPack << (uint32_t) resultPct.login_flag;
    _socket.SendPacket(_resPack);
    return true;
}

bool ClientHandler::HandleSelectServer(XPacket *_packet)
{
    _player.one_time_key = ((uint64) rand()) * rand() * rand() * rand();
    _player.isInGame = true;
    sPlayerMapList->UpdatePlayer(_player);
    XPacket packet(ACPACKETS::TS_AC_SELECT_SERVER);
    packet << (uint16_t) 0;
    packet << (int64_t) _player.one_time_key;
    packet << (uint32_t) 0;
    _socket.SendPacket(packet);
}