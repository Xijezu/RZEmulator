/*
  *  Copyright (C) 2016-2016 Xijezu <http://xijezu.com/>
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
#include "GameNetwork/GameHandler.h"
#include "World.h"
#include "Database/DatabaseEnv.h"
#include "GameNetwork/ClientPackets.h"
#include "../../../../Mononoke/src/Server/AuthGame/AuthGamePackets.h"
#include "Network/AuthSession.h"
#include "MemPool.h"
#include "Messages.h"
#include "Scripting/XLua.h"

#include "Encryption/MD5.h"
#include "Map/ArRegion.h"
#include "ObjectMgr.h"
#include "Skill.h"

// Constructo - give it a socket
GameSession::GameSession(XSocket &socket) : _socket(socket)
{
    _rc4decode.SetKey("}h79q~B%al;k'y $E");
    _rc4encode.SetKey("}h79q~B%al;k'y $E");
}

// Close patch file descriptor before leaving
GameSession::~GameSession(void)
{
    _rc4decode.Clear();
    _rc4encode.Clear();
}

// Accept the connection - function itself not used here because we're only interested in the game server data itself
void GameSession::OnAccept(void)
{

}

void GameSession::OnClose(void)
{
    if (_accountName.length() > 0)
        sAuthNetwork->SendLogoutToAuth(_accountName);
    if (_player != nullptr && _player->IsInWorld())
        delete _player;
}

enum eStatus {
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct GameHandler {
    uint16_t cmd;
    uint8_t  status;
    bool (GameSession::*handler)(XPacket *);
} GameHandler;

const GameHandler packetHandler[] =
                          {
                                  {TS_CS_VERSION,               STATUS_CONNECTED, &GameSession::HandleNullPacket},
                                  {TS_CS_VERSION2,              STATUS_CONNECTED, &GameSession::HandleNullPacket},
                                  {TS_CS_PING,                  STATUS_CONNECTED, &GameSession::HandleNullPacket},
                                  {TS_AG_CLIENT_LOGIN,          STATUS_CONNECTED, &GameSession::onAuthResult},
                                  {TS_CS_ACCOUNT_WITH_AUTH,     STATUS_CONNECTED, &GameSession::onAccountWithAuth},
                                  {TS_CS_REQUEST_LOGOUT,        STATUS_AUTHED,    &GameSession::onLogoutTimerRequest},
                                  {TS_CS_REQUEST_RETURN_LOBBY,  STATUS_AUTHED,    &GameSession::onLogoutTimerRequest},
                                  {TS_CS_RETURN_LOBBY,          STATUS_AUTHED,    &GameSession::onReturnToLobby},
                                  {TS_CS_CHARACTER_LIST,        STATUS_AUTHED,    &GameSession::onCharacterList},
                                  {TS_CS_LOGIN,                 STATUS_AUTHED,    &GameSession::onLogin},
                                  {TS_CS_CHECK_CHARACTER_NAME,  STATUS_AUTHED,    &GameSession::onCharacterName},
                                  {TS_CS_CREATE_CHARACTER,      STATUS_AUTHED,    &GameSession::onCreateCharacter},
                                  {TS_CS_DELETE_CHARACTER,      STATUS_AUTHED,    &GameSession::onDeleteCharacter},
                                  {TS_CS_MOVE_REQUEST,          STATUS_AUTHED,    &GameSession::onMoveRequest},
                                  {TS_CS_REGION_UPDATE,         STATUS_AUTHED,    &GameSession::onRegionUpdate},
                                  {TS_CS_CHAT_REQUEST,          STATUS_AUTHED,    &GameSession::onChatRequest},
                                  {TS_CS_PUTON_ITEM,            STATUS_AUTHED,    &GameSession::onPutOnItem},
                                  {TS_CS_PUTOFF_ITEM,           STATUS_AUTHED,    &GameSession::onPutOffItem},
                                  {TS_CS_GET_SUMMON_SETUP_INFO, STATUS_AUTHED,    &GameSession::onGetSummonSetupInfo},
                                  {TS_CS_CONTACT,               STATUS_AUTHED,    &GameSession::onContact},
                                  {TS_CS_DIALOG,                STATUS_AUTHED,    &GameSession::onDialog},
                                  {TS_CS_BUY_ITEM,              STATUS_AUTHED,    &GameSession::onBuyItem},
                                  {TS_CS_CHANGE_LOCATION,       STATUS_AUTHED,    &GameSession::onChangeLocation},
                                  {TS_TIMESYNC,                 STATUS_AUTHED,    &GameSession::onTimeSync},
                                  {TS_CS_GAME_TIME,             STATUS_AUTHED,    &GameSession::onGameTime},
                                  {TS_CS_QUERY,                 STATUS_AUTHED,    &GameSession::onQuery},
                                  {TS_CS_UPDATE,                STATUS_AUTHED,    &GameSession::onUpdate},
                                  {TS_CS_JOB_LEVEL_UP,          STATUS_AUTHED,    &GameSession::onJobLevelUp},
                                  {TS_CS_LEARN_SKILL,           STATUS_AUTHED,    &GameSession::onLearnSkill}
                          };

const int tableSize = (sizeof(packetHandler) / sizeof(GameHandler));

/// Handler for incoming packets
void GameSession::ProcessIncoming(XPacket *pRecvPct)
{
    ACE_ASSERT(pRecvPct);

    // Manage memory
    ACE_Auto_Ptr<XPacket> aptr(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int  i    = 0;

    for (i = 0; i < tableSize; i++) {
        if ((uint16_t) packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (_isAuthed && packetHandler[i].status == STATUS_AUTHED))) {
            //MX_LOG_DEBUG("network", "Got data for id %u recv length %u", (uint32) _cmd, (uint32) pRecvPct->size());

            if (!(*this.*packetHandler[i].handler)(pRecvPct)) {
                MX_LOG_ERROR("network", "Packet handler failed for id %u recv length %u", (uint32) _cmd, (uint32) pRecvPct->size());
                return;
            }
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == tableSize) {
        MX_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), _socket.getRemoteAddress().c_str());
        return;
    }
    aptr.release();
}

void GameSession::Decrypt(void *pBuf, size_t size, bool isPeek)
{
    _rc4decode.Decode(pBuf, pBuf, size, isPeek);
}

void GameSession::Encrypt(void *pBuf, size_t size, bool isPeek)
{
    _rc4encode.Encode(pBuf, pBuf, size, isPeek);
}

/// TODO: The whole stuff needs a rework, it is working as intended but it's just a dirty hack
bool GameSession::onAccountWithAuth(XPacket *pGamePct)
{
    s_ClientWithAuth_CS *result = ((s_ClientWithAuth_CS *) (pGamePct)->contents());
    sAuthNetwork->SendAccountToAuth(*this, result->account, result->one_time_key);
    return true;
}

void GameSession::_SendResultMsg(uint16 _msg, uint16 _result, int _value)
{
    XPacket packet(TS_SC_RESULT);
    packet << (uint16) _msg;
    packet << (uint16) _result;
    packet << (int32) _value;
    _socket.SendPacket(packet);
    _socket.handle_output();
}

bool GameSession::onCharacterList(XPacket *pGamePct)
{
    XPacket packet(TS_SC_CHARACTER_LIST);
    packet << (uint32) time(nullptr);
    packet << (uint16) 0;
    auto info = _PrepareCharacterList(_accountId);
    packet << (uint16) info.size();
    for (int i = 0; i < info.size(); i++) {
        packet << info.at(i).sex;
        packet << info.at(i).race;
        for (int j = 0; j < 5; j++) {
            packet << info.at(i).model_id[j];
        }
        for (int j = 0; j < 24; j++) {
            packet << info.at(i).wear_info[j];
        }
        packet << info.at(i).level;
        packet << info.at(i).job;
        packet << info.at(i).job_level;
        packet << info.at(i).exp;
        packet << info.at(i).hp;
        packet << info.at(i).mp;
        packet << info.at(i).permission;
        packet << (uint8) 0;
        packet.fill(info.at(i).name, 19);
        packet << info.at(i).skin_color;
        packet.fill(info.at(i).szCreateTime, 30);
        packet.fill(info.at(i).szDeleteTime, 30);
        for (int j = 0; j < 24; j++) {
            packet << info.at(i).wear_item_enhance_info[j];
        }
        for (int j = 0; j < 24; j++) {
            packet << info.at(i).wear_item_level_info[j];
        }
    }
    _socket.SendPacket(packet);
    return true;
}

/// TODO: Might need to put this in player class?
std::vector<LobbyCharacterInfo> GameSession::_PrepareCharacterList(uint32 account_id)
{
    std::vector<LobbyCharacterInfo> _info;
    PreparedStatement               *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_CHARACTERLIST);
    stmt->setInt32(0, account_id);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        do {
            LobbyCharacterInfo info;
            int                sid = (*result)[0].GetInt32();
            info.name       = (*result)[1].GetString();
            info.race       = (*result)[2].GetInt32();
            info.sex        = (*result)[3].GetInt32();
            info.level      = (*result)[4].GetInt32();
            info.job_level  = (*result)[5].GetInt32();
            info.exp        = (*result)[6].GetInt32();
            info.hp         = (*result)[7].GetInt32();
            info.mp         = (*result)[8].GetInt32();
            info.job        = (*result)[9].GetInt32();
            info.permission = (*result)[10].GetInt32();
            info.skin_color = (*result)[11].GetUInt32();
            for (int i = 0; i < 5; i++) {
                info.model_id[i] = (*result)[12 + i].GetInt32();
            }
            info.szCreateTime = (*result)[17].GetString();
            info.szDeleteTime = (*result)[18].GetString();
            PreparedStatement *wstmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_WEARINFO);
            wstmt->setInt32(0, sid);
            if (PreparedQueryResult wresult = CharacterDatabase.Query(wstmt)) {
                do {
                    int wear_info = (*wresult)[0].GetInt32();
                    info.wear_info[wear_info]              = (*wresult)[1].GetInt32();
                    info.wear_item_enhance_info[wear_info] = (*wresult)[2].GetInt32();
                    info.wear_item_level_info[wear_info]   = (*wresult)[3].GetInt32();
                } while (wresult->NextRow());
            }
            _info.push_back(info);
        } while (result->NextRow());
    }
    return _info;
}

bool GameSession::onAuthResult(XPacket *pGamePct)
{
    AG_CLIENT_LOGIN *result = ((AG_CLIENT_LOGIN *) (pGamePct)->contents());
    if (result->result == TS_RESULT_SUCCESS) {
        _isAuthed    = true;
        _accountId   = result->nAccountID;
        _accountName = result->account;
    }
    _SendResultMsg(TS_CS_ACCOUNT_WITH_AUTH, result->result, 0);
    return true;
}

bool GameSession::onLogin(XPacket *pRecvPct)
{
    s_ClientLogin_CS *result = ((s_ClientLogin_CS *) (pRecvPct)->contents());

    //_player = new Player(this);
    _player = sMemoryPool->AllocPlayer();
    _player->SetSession(this);
    if (!_player->ReadCharacter(result->szName, _accountId)) {
        delete _player;
        _player = nullptr;
        return false;
    }

    Messages::SendTimeSynch(_player);

    sScriptingMgr->RunString(_player, "on_login('"s + _player->GetName() + "')"s);

    XPacket packet(TS_SC_LOGIN_RESULT); // Login Result
    packet << (uint8) 1;
    packet << _player->GetHandle();
    packet << _player->GetPositionX();
    packet << _player->GetPositionY();
    packet << _player->GetPositionZ();
    packet << (uint8) _player->GetLayer();
    packet << (uint32) _player->GetOrientation();
    packet << (uint32) sConfigMgr->GetIntDefault("Game.RegionSize", 180);
    packet << (uint32)_player->GetHealth();
    packet << (uint16) _player->GetMana();
    packet << (uint32)_player->GetMaxHealth();
    packet << (uint16) _player->GetMaxMana();
    packet << _player->GetUInt32Value(UNIT_FIELD_HAVOC);
    packet << _player->GetUInt32Value(UNIT_FIELD_HAVOC);
    packet << _player->GetUInt32Value(UNIT_FIELD_SEX);
    packet << _player->GetUInt32Value(UNIT_FIELD_RACE);
    packet << _player->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    packet << _player->GetUInt32Value(UNIT_FIELD_MODEL + 1);
    packet << _player->GetUInt32Value(UNIT_FIELD_MODEL);
    packet.fill(result->szName, 19);
    packet << (uint32) sConfigMgr->GetIntDefault("Game.CellSize", 6);
    packet << _player->GetUInt32Value(UNIT_FIELD_GUILD_ID);
    _socket.SendPacket(packet);

    _player->SendLoginProperties();
    return true;
}

bool GameSession::onMoveRequest(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::vector<Position> vPctInfo{ }, vMoveInfo{ };
    MX_LOG_DEBUG("network", "Before: %f, %f", _player->GetPositionX(), _player->GetPositionY());

    auto handle     = pRecvPct->read<uint32_t>();
    auto x          = pRecvPct->read<float>();
    auto y          = pRecvPct->read<float>();
    auto curr_time  = pRecvPct->read<uint32_t>();
    auto speed_sync = pRecvPct->read<uint8_t>() != 0;
    auto count      = pRecvPct->read<uint16_t>();

    if (_player == nullptr || _player->GetHealth() == 0 || !_player->IsInWorld() || count == 0)
        return true;

    for (int i   = 0; i < count; i++) {
        Position pos{ };
        pos.m_positionX = pRecvPct->read<float>();
        pos.m_positionY = pRecvPct->read<float>();
        vPctInfo.push_back(pos);
    }

    int      speed;
    float    distance;
    Position pos = _player->GetPosition();
    Position npos{ };
    Position curPosFromClient{ };
    Position wayPoint{ };

    uint ct = sWorld->GetArTime();
    speed = 50;
    auto mover = dynamic_cast<Unit *>(_player);

    if (handle == 0 || handle == _player->GetHandle()) {
        // Set Speed if ride
    } else {
        // Do Summon Movement
    }

    if (mover == nullptr) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return false;
    }
    npos.m_positionX = x;
    npos.m_positionY = y;
    npos.m_positionZ = 0.0f;

    if (x < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapWidth", 700000) < x || y < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapHeight", 1000000) < y || mover->GetExactDist2d(&npos) > 525.0f) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
        return true;
    }
    if (speed < 1)
        speed = 1;

    wayPoint.m_positionX  = x;
    wayPoint.m_positionY  = y;
    wayPoint.m_positionZ  = 0.0f;
    wayPoint._orientation = 0.0f;

    for (auto &mi : vPctInfo) {
        if (mover->GetSubType() == ST_Player && false /* CollisionToLine*/)
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return true;
        }
        curPosFromClient.m_positionX = mi.m_positionX;
        curPosFromClient.m_positionY = mi.m_positionY;
        curPosFromClient.m_positionZ = 0.0f;
        wayPoint.m_positionX         = curPosFromClient.m_positionX;
        wayPoint.m_positionY         = curPosFromClient.m_positionY;
        wayPoint.m_positionZ         = curPosFromClient.m_positionZ;
        wayPoint._orientation        = curPosFromClient._orientation;
        if (mi.m_positionX < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapWidth", 700000) < mi.m_positionX ||
                mi.m_positionY < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapHeight", 1000000) < mi.m_positionY) {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return true;
        }
        vMoveInfo.emplace_back(wayPoint);
    }

    if (vMoveInfo.empty())
        return false;

    Position cp = vMoveInfo.back();
    // TODO if(QuadTree stuff)
    // TODO if(IsBlocked)

    if (_player->IsInWorld())
    {
        if (mover->m_nMovableTime <= ct)
        {
            if (true /* IsActable() && IsMovable() && isInWorld */)
            {
                auto tpos2 = mover->GetCurrentPosition(curr_time);
                if (!vMoveInfo.empty()) {
                    cp = vMoveInfo.back();
                    npos.m_positionX  = cp.GetPositionX();
                    npos.m_positionY  = cp.GetPositionY();
                    npos.m_positionZ  = cp.GetPositionZ();
                    npos._orientation = cp.GetOrientation();
                } else {
                    npos.m_positionX  = 0.0f;
                    npos.m_positionY  = 0.0f;
                    npos.m_positionZ  = 0.0f;
                    npos._orientation = 0.0f;
                }
                if (mover->GetHandle() != _player->GetHandle() || sConfigMgr->GetFloatDefault("GameContent.MapLength", 16128.0f) / 5.0 >= tpos2.GetExactDist2d(&cp)
                    /*|| !_player.m_bAutoUsed*/ || !_player->m_nWorldLocationId != 110900) {
                    if (vMoveInfo.empty() || sConfigMgr->GetFloatDefault("GameContent.MapLength", 16128.0f) >= _player->GetCurrentPosition(ct).GetExactDist2d(&npos)) {
                        npos.m_positionX = x;
                        npos.m_positionY = y;
                        npos.m_positionZ = 0.0f;

                        sWorld->SetMultipleMove(mover, npos, vMoveInfo, speed, true, ct, true);
                        // TODO: Mount
                    }
                }
                return true;
            } //if (true /* IsActable() && IsMovable() && isInWorld */)
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return true;
        }
        if (!mover->SetPendingMove(vMoveInfo, (uint8_t) speed))
        {
            for(auto& i : vMoveInfo)
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return true;
        }
    } // is in world
    return true;
}

void GameSession::_SendMoveMsg(Object &obj, Position nPos, std::vector<Position> vMoveInfo)
{
    Player  *p = reinterpret_cast<Player *>(&obj);
    XPacket packet(TS_SC_MOVE);
    packet << (uint32) time(NULL);
    packet << p->GetHandle();
    packet << (uint8) p->GetLayer();
    packet << (uint8) 45;
    packet << (uint16) vMoveInfo.size();
    for (auto const &x : vMoveInfo) {
        packet << x.GetPositionY();
        packet << x.GetPositionX();
    }
    packet.textlike();
}

bool GameSession::onReturnToLobby(XPacket *pRecvPct)
{
    sWorld->RemoveSession(GetAccountId());
    _player->RemoveFromWorld();
    if (_player != nullptr) {
        _player->Save(false);
        delete _player;
        _player = nullptr;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), 0, 0);
    return true;
}

bool GameSession::onCreateCharacter(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    LobbyCharacterInfo info{ };
    *pRecvPct >> info.sex;
    *pRecvPct >> info.race;
    for (int &i : info.model_id)
        *pRecvPct >> i;
    for (int &i : info.wear_info)
        *pRecvPct >> i;
    *pRecvPct >> info.level;
    *pRecvPct >> info.job;
    *pRecvPct >> info.job_level;
    *pRecvPct >> info.exp;
    *pRecvPct >> info.hp;
    *pRecvPct >> info.mp;
    *pRecvPct >> info.permission;
    pRecvPct->read_skip(1);
    info.name = pRecvPct->ReadString(19);
    *pRecvPct >> info.skin_color;
    info.job = 0;
    if (checkCharacterName(info.name)) {
        uint8_t           j     = 0;
        PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_CHARACTER);
        stmt->setString(j++, info.name);
        stmt->setString(j++, _accountName);
        stmt->setInt32(j++, _accountId);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, info.race);
        stmt->setInt32(j++, info.sex);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, info.job);
        stmt->setInt32(j++, info.job_level);
        stmt->setInt32(j++, info.exp);
        stmt->setInt32(j++, 320);
        stmt->setInt32(j++, 320);
        stmt->setUInt32(j++, info.skin_color);
        for (int i         = 0; i < 5; i++) {
            stmt->setUInt32(j++, info.model_id[i]);
        }
        auto     playerUID = sWorld->getPlayerIndex();
        stmt->setUInt32(j, playerUID);
        CharacterDatabase.Query(stmt);

        int m_wear_item       = info.wear_info[2];
        int nDefaultBagCode   = 490001;
        int nDefaultArmorCode = 220100;
        if (m_wear_item == 602)
            nDefaultArmorCode = 220109;

        int nDefaultWeaponCode = 106100;
        if (info.race == 3) {
            nDefaultArmorCode     = 240100;
            if (m_wear_item == 602)
                nDefaultArmorCode = 240109;
            nDefaultWeaponCode    = 112100;
        } else {
            if (info.race == 5) {
                nDefaultArmorCode     = 230100;
                if (m_wear_item == 602)
                    nDefaultArmorCode = 230109;
                nDefaultWeaponCode    = 103100;
            }
        }

        auto itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->getItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultWeaponCode);
        itemStmt->setInt32(3, WearWeapon);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->getItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultArmorCode);
        itemStmt->setInt32(3, WearArmor);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->getItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultBagCode);
        itemStmt->setInt32(3, WearBagSlot);
        CharacterDatabase.Execute(itemStmt);

        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
        return true;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
}

bool GameSession::checkCharacterName(std::string szName)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_NAMECHECK);
    stmt->setString(0, szName);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        return false;
    }
    return true;
}

bool GameSession::onCharacterName(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::string szName = pRecvPct->read<std::string>();
    if (!checkCharacterName(szName)) {
        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
        return true;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
    return true;

}

bool GameSession::onChatRequest(XPacket *_packet)
{
    CS_CHATREQUEST request = { };

    _packet->read_skip(7);
    _packet->read((uint8 *) &request.szTarget[0], 21);
    *_packet >> request.request_id;
    *_packet >> request.len;
    *_packet >> request.type;
    request.szMsg = _packet->ReadString(request.len);

    MX_LOG_INFO("network", "%s", request.szMsg.c_str());

    if (request.type != 3 && request.szMsg[0] == 47) {
        Tokenizer tokenizer(request.szMsg, ' ');
        if (tokenizer[0] == "/warp"s) {
            _player->SendWarpMessage((float) std::stoi(tokenizer[1], nullptr, 0), (float) std::stoi(tokenizer[2], nullptr, 0), 0, 0);
        } else if (tokenizer[0] == "/save"s) {
            _player->Save(false);
        } else if (tokenizer[0] == "/run"s) {
            sScriptingMgr->RunString(_player, request.szMsg.substr(5));
        } else if (tokenizer[0] == "/loc"s) {
            _player->ChangeLocation(_player->GetPositionX(), _player->GetPositionY(), false, false);
        } else if(tokenizer[0] == "/calc"s) {
            _player->CalculateStat();
        }
        return true;
    }

    if (request.type == 4) {
        // Global Chat Message
    } else if (request.type == 0) {
        // Local Message
        XPacket result(TS_SC_CHAT_LOCAL);
        result << (uint32_t) _player->GetHandle();
        result << (uint8_t) request.len;
        result << (uint8_t) request.type;
        result << std::string((char *) request.szMsg.data());
        _player->GetSession()._socket.SendPacket(result);
        return true;
    } else if (request.type == 3) {
        // Whisper
        return true;
    } else if (request.type == 2) {
        // Guild
        return true;
    } else if (request.type == 1) {
        // party
        return true;
    }

}

bool GameSession::onLogoutTimerRequest(XPacket *pRecvPct)
{
    sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
    return true;
}

bool GameSession::onPutOnItem(XPacket *_packet)
{
    _packet->read_skip(7);
    auto position      = _packet->read<uint8_t>();
    auto item_handle   = _packet->read<uint>();
    auto target_handle = _packet->read<uint>();

    if (position == WearTwoHand)
        position = WearWeapon;

    //if (_player->isAlive()) {
    if (true) {
        Item *ci = sMemoryPool->FindItem(item_handle);

        if (ci != nullptr) {
            if (!ci->IsWearable() || _player->FindItemBySID(ci->m_Instance.UID) == NULL) {
                sWorld->SendResult(*_player, TS_CS_PUTON_ITEM, TS_RESULT_ACCESS_DENIED, 0);
                return true;
            }

            if (_player->putonItem((ItemWearType) position, ci) == 0) {
                _player->CalculateStat();
                Messages::SendStatInfo(_player, _player);
                sWorld->SendResult(*_player, TS_CS_PUTON_ITEM, TS_RESULT_SUCCESS, 0);
                if (true) { // TODO: isPlayer()
                    _player->SendWearInfo();
                }
            }
        }
    }
    return true;
}

bool GameSession::onPutOffItem(XPacket *_packet)
{
    _packet->read_skip(7);
    auto position      = _packet->read<uint8_t>();
    auto target_handle = _packet->read<uint>();

    if (position == WearTwoHand)
        position = WearWeapon;

    //if(!_player->isAlive())
    if (false)
        sWorld->SendResult(*_player, TS_CS_PUTOFF_ITEM, 5, 0);

    Item *curitem = _player->GetWornItem((ItemWearType) position);
    if (curitem == nullptr) {
        sWorld->SendResult(*_player, TS_CS_PUTOFF_ITEM, 1, 0);
    } else {
        uint16_t por = _player->putoffItem((ItemWearType) position);
        _player->CalculateStat();
        Messages::SendStatInfo(_player, _player);
        sWorld->SendResult(*_player, _packet->GetPacketID(), 0, 0);
        if (por == 0) {
            if (true) {// TODO IsPlayer
                _player->SendWearInfo();
            }
        }
    }
    return true;
}

bool GameSession::onRegionUpdate(XPacket *pRecvPct)
{
    if (_player == nullptr)
        return true;

    pRecvPct->read_skip(7);
    auto update_time = pRecvPct->read<uint>();
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();
    auto z = pRecvPct->read<float>();
    auto bIsStopMessage = pRecvPct->read<bool>();

    sWorld->onRegionChange(_player, update_time, bIsStopMessage);

    return true;
}

bool GameSession::onGetSummonSetupInfo(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    bool showDialog = pRecvPct->read<uint8_t>() == 1;
    Messages::SendCreatureEquipMessage(_player, showDialog);
    return true;
}

bool GameSession::onContact(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint32_t>();
    auto npc    = dynamic_cast<NPC *>(sMemoryPool->getPtrFromId(handle));

    if (npc != nullptr) {
        _player->SetLastContact("npc", handle);
        sScriptingMgr->RunString(_player, npc->m_pBase.contact_script);
    }
    return true;
}

bool GameSession::onDialog(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto        size    = pRecvPct->read<uint16_t>();
    std::string trigger = pRecvPct->ReadString(size);

    if (trigger.empty())
        return true;

    if (!_player->IsValidTrigger(trigger)) {
        if (!_player->IsFixedDialogTrigger(trigger)) {
            MX_LOG_ERROR("scripting", "INVALID SCRIPT TRIGGER!!! [%s][%s]", _player->GetName(), trigger.c_str());
            return false;
        }
    }

    auto npc = dynamic_cast<NPC *>(sMemoryPool->getPtrFromId(_player->GetLastContactLong("npc")));
    if (npc == nullptr) {
        MX_LOG_TRACE("scripting", "onDialog: NPC not found!");
        return false;
    }

    sScriptingMgr->RunString(_player, trigger);
    if (_player->HasDialog())
        _player->ShowDialog();

    return true;
}

bool GameSession::onBuyItem(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto item_code = pRecvPct->read<int>();
    auto buy_count = pRecvPct->read<uint16_t>();

    auto szMarketName = _player->GetLastContactStr("market");
    if (buy_count == 0) {
        MX_LOG_TRACE("network", "onBuyItem - %s: buy_count was 0!", _player->GetName());
        sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return false;
    }

    auto market = sObjectMgr->GetMarketInfo(szMarketName);
    if (market.empty()) {
        MX_LOG_TRACE("network", "onBuyItem - %s: market was empty!", _player->GetName());
        sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return false;
    }

    bool bJoinable{false};

    for (auto mt : market) {
        if (mt.code == item_code) {
            auto ibs = sObjectMgr->GetItemBase(item_code);
            if (ibs.flaglist[FLAG_DUPLICATE] == 1) {
                bJoinable = true;
            } else {
                bJoinable     = false;
                if (buy_count != 1)
                    buy_count = 1;
            }

            auto nTotalPrice = (uint) floor(buy_count * mt.price_ratio);
            if (nTotalPrice / buy_count != mt.price_ratio || _player->GetGold() < nTotalPrice) {
                sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
                return true;
            }
            // TODO Add Huntaholic Check
            // TODO Add Weight Check
            uint32_t uid = 0;

            auto result = _player->ChangeGold(_player->GetGold() - nTotalPrice);
            if (result != TS_RESULT_SUCCESS) {
                sWorld->SendResult(*_player, pRecvPct->GetPacketID(), result, 0);
                return true;
            }

            if (bJoinable) {
                auto item = Item::AllocItem(0, mt.code, buy_count, GenerateCode::ByMarket, 0, 0, 0, 0, 0, 0, 0, 0);
                _player->PushItem(item, buy_count, false);
            } else {
                for (int i = 0; i < buy_count; i++) {
                    auto item = Item::AllocItem(0, mt.code, 1, GenerateCode::ByMarket, 0, 0, 0, 0, 0, 0, 0, 0);
                    _player->PushItem(item, buy_count, false);
                }
            }
            sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item_code);
            XPacket resultPct(TS_SC_NPC_TRADE_INFO);
            resultPct << (uint8_t) 0;
            resultPct << item_code;
            resultPct << (int64) buy_count;
            resultPct << (int64) mt.price_ratio;
            resultPct << (int64) mt.huntaholic_ratio;
            resultPct << (uint32_t) _player->GetLastContactLong("npc");
            _player->GetSession().GetSocket().SendPacket(resultPct);
        }
    }

}

bool GameSession::onDeleteCharacter(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto name = pRecvPct->ReadString(19);
    auto stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_CHARACTER);
    stmt->setString(0, name);
    stmt->setInt32(1, _accountId);
    CharacterDatabase.Execute(stmt);
    //sWorld->SendResult(*_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

bool GameSession::onChangeLocation(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();

    _player->ChangeLocation(x, y, true, true);
    return true;
}

bool GameSession::onTimeSync(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto packet_time = pRecvPct->read<int>();
    uint ct          = sWorld->GetArTime();
    _player->m_TS.onEcho(ct - packet_time);
    if (_player->m_TS.m_vT.size() >= 4) {
        XPacket result(TS_SC_SET_TIME);
        result << (uint32_t) _player->m_TS.GetInterval();
        _player->GetSession().GetSocket().SendPacket(result);
        return true;
    } else {
        Messages::SendTimeSynch(_player);
        return true;
    }
    ACE_NOTREACHED(return true);
}

bool GameSession::onGameTime(XPacket *pRecvPct)
{
    Messages::SendGameTime(_player);
    return true;
}

bool GameSession::onQuery(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();

    WorldObject* obj = sMemoryPool->getPtrFromId(handle);
    if(obj != nullptr) {
        if(!sArRegion->IsVisibleRegion((uint)(obj->GetPositionX() / g_nRegionSize),
                (uint)(obj->GetPositionY() / g_nRegionSize),
                (uint)(_player->GetPositionX() / g_nRegionSize),
                (uint)(_player->GetPositionY() / g_nRegionSize))) {
            MX_LOG_DEBUG("network", "onQuery failed: Not visible region!");
            return true;
        }
        Messages::sendEnterMessage(_player, obj, false);
    }
    return true;
}

bool GameSession::onUpdate(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();

    auto unit = dynamic_cast<Unit*>(_player);
    if(handle != _player->GetHandle()) {
        // Do Summon stuff here
    }
    if(unit != nullptr) {
        unit->OnUpdate();
        return true;
    }
    return false;
}

bool GameSession::onJobLevelUp(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto target = pRecvPct->read<uint>();

    Unit* cr = dynamic_cast<Player*>(_player);
    if(cr == nullptr) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, target);
        return false;
    }
    if(cr->GetSubType() == ST_Player && cr->GetHandle() != _player->GetHandle()) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, target);
        return false;
    }
    int jp = sObjectMgr->GetNeedJpForJobLevelUp(cr->GetCurrentJLv(), _player->GetJobDepth());
    if(cr->GetJP() < jp) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_JP, target);
        return true;
    }
    if(jp == 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_LIMIT_MAX, target);
        return true;
    }
    cr->SetJP(cr->GetJP() -jp);
    cr->SetCurrentJLv(cr->GetCurrentJLv() + 1);
    cr->CalculateStat();
    if(cr->GetSubType() == ST_Player) {
        dynamic_cast<Player*>(cr)->Save(true);
    } else {
        // Summon
    }
    _player->Save(true);
    Messages::SendPropertyMessage(_player, cr, "job_level", cr->GetCurrentJLv());
    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, target);
    return true;
}

bool GameSession::onLearnSkill(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto target_handle = pRecvPct->read<uint>();
    auto skill_id = pRecvPct->read<int>();
    //auto skill_level = pRecvPct->read<int16>();

    if(_player == nullptr)
        return false;

    ushort result = 0;
    int jobID = 0;
    int value = 0;

    auto target = dynamic_cast<Unit*>(_player);
    if(false /*if creature*/){

    }
    int currentLevel = target->GetBaseSkillLevel(skill_id)+1;
    //if(skill_level == currentLevel)
    //{
        result = sObjectMgr->IsLearnableSkill(target, skill_id, currentLevel, jobID);
        if(result == TS_RESULT_SUCCESS)
        {
            target->RegisterSkill(skill_id, currentLevel, 0, jobID);
        }
        Messages::SendResult(_player,pRecvPct->GetPacketID(), result, value);
    //}
}
