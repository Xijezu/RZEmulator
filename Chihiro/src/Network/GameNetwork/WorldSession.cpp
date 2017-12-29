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
#include "GameNetwork/WorldSession.h"
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
WorldSession::WorldSession(WorldSocket *socket) : _socket(socket)
{
    _rc4decode.SetKey("}h79q~B%al;k'y $E");
    _rc4encode.SetKey("}h79q~B%al;k'y $E");
}

// Close patch file descriptor before leaving
WorldSession::~WorldSession()
{
    _rc4decode.Clear();
    _rc4encode.Clear();
}

// Accept the connection - function itself not used here because we're only interested in the game server data itself
/*void WorldSession::OnAccept()
{

}
*/
void WorldSession::OnClose()
{
    if (_accountName.length() > 0)
        sAuthNetwork->SendLogoutToAuth(_accountName);
    onReturnToLobby(nullptr);
}

enum eStatus {
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct GameHandler {
    uint16_t cmd;
    uint8_t  status;
    bool (WorldSession::*handler)(XPacket *);
} GameHandler;

const GameHandler packetHandler[] =
                          {
                                  {TS_CS_VERSION,               STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                  {TS_CS_VERSION2,              STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                  {TS_CS_PING,                  STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                  {TS_AG_CLIENT_LOGIN,          STATUS_CONNECTED, &WorldSession::onAuthResult},
                                  {TS_CS_ACCOUNT_WITH_AUTH,     STATUS_CONNECTED, &WorldSession::onAccountWithAuth},
                                  {TS_CS_REQUEST_LOGOUT,        STATUS_AUTHED,    &WorldSession::onLogoutTimerRequest},
                                  {TS_CS_REQUEST_RETURN_LOBBY,  STATUS_AUTHED,    &WorldSession::onLogoutTimerRequest},
                                  {TS_CS_RETURN_LOBBY,          STATUS_AUTHED,    &WorldSession::onReturnToLobby},
                                  {TS_CS_CHARACTER_LIST,        STATUS_AUTHED,    &WorldSession::onCharacterList},
                                  {TS_CS_LOGIN,                 STATUS_AUTHED,    &WorldSession::onLogin},
                                  {TS_CS_CHECK_CHARACTER_NAME,  STATUS_AUTHED,    &WorldSession::onCharacterName},
                                  {TS_CS_CREATE_CHARACTER,      STATUS_AUTHED,    &WorldSession::onCreateCharacter},
                                  {TS_CS_DELETE_CHARACTER,      STATUS_AUTHED,    &WorldSession::onDeleteCharacter},
                                  {TS_CS_MOVE_REQUEST,          STATUS_AUTHED,    &WorldSession::onMoveRequest},
                                  {TS_CS_REGION_UPDATE,         STATUS_AUTHED,    &WorldSession::onRegionUpdate},
                                  {TS_CS_CHAT_REQUEST,          STATUS_AUTHED,    &WorldSession::onChatRequest},
                                  {TS_CS_PUTON_ITEM,            STATUS_AUTHED,    &WorldSession::onPutOnItem},
                                  {TS_CS_PUTOFF_ITEM,           STATUS_AUTHED,    &WorldSession::onPutOffItem},
                                  {TS_CS_GET_SUMMON_SETUP_INFO, STATUS_AUTHED,    &WorldSession::onGetSummonSetupInfo},
                                  {TS_CS_CONTACT,               STATUS_AUTHED,    &WorldSession::onContact},
                                  {TS_CS_DIALOG,                STATUS_AUTHED,    &WorldSession::onDialog},
                                  {TS_CS_BUY_ITEM,              STATUS_AUTHED,    &WorldSession::onBuyItem},
                                  {TS_CS_CHANGE_LOCATION,       STATUS_AUTHED,    &WorldSession::onChangeLocation},
                                  {TS_TIMESYNC,                 STATUS_AUTHED,    &WorldSession::onTimeSync},
                                  {TS_CS_GAME_TIME,             STATUS_AUTHED,    &WorldSession::onGameTime},
                                  {TS_CS_QUERY,                 STATUS_AUTHED,    &WorldSession::onQuery},
                                  {TS_CS_UPDATE,                STATUS_AUTHED,    &WorldSession::onUpdate},
                                  {TS_CS_JOB_LEVEL_UP,          STATUS_AUTHED,    &WorldSession::onJobLevelUp},
                                  {TS_CS_LEARN_SKILL,           STATUS_AUTHED,    &WorldSession::onLearnSkill},
                                  {TS_EQUIP_SUMMON,             STATUS_AUTHED,    &WorldSession::onEquipSummon},
                                  {TS_CS_SELL_ITEM,             STATUS_AUTHED,    &WorldSession::onSellItem},
                                  {TS_CS_SKILL,                 STATUS_AUTHED,    &WorldSession::onSkill},
                                  {TS_CS_SET_PROPERTY,          STATUS_AUTHED,    &WorldSession::onSetProperty}
                          };

const int tableSize = (sizeof(packetHandler) / sizeof(GameHandler));

/// Handler for incoming packets
void WorldSession::ProcessIncoming(XPacket *pRecvPct)
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
        MX_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), _socket->GetRemoteAddress().c_str());
        return;
    }
    aptr.release();
}

/*void WorldSession::Decrypt(void *pBuf, size_t size, bool isPeek)
{
    _rc4decode.Decode(pBuf, pBuf, size, isPeek);
}

void WorldSession::Encrypt(void *pBuf, size_t size, bool isPeek)
{
    _rc4encode.Encode(pBuf, pBuf, size, isPeek);
}*/

/// TODO: The whole stuff needs a rework, it is working as intended but it's just a dirty hack
bool WorldSession::onAccountWithAuth(XPacket *pGamePct)
{
    s_ClientWithAuth_CS *result = ((s_ClientWithAuth_CS *) (pGamePct)->contents());
    sAuthNetwork->SendAccountToAuth(*this, result->account, result->one_time_key);
    return true;
}

void WorldSession::_SendResultMsg(uint16 _msg, uint16 _result, int _value)
{
    XPacket packet(TS_SC_RESULT);
    packet << (uint16) _msg;
    packet << (uint16) _result;
    packet << (int32) _value;
    _socket->SendPacket(packet);
    _socket->handle_output();
}

bool WorldSession::onCharacterList(XPacket *pGamePct)
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
    _socket->SendPacket(packet);
    return true;
}

/// TODO: Might need to put this in player class?
std::vector<LobbyCharacterInfo> WorldSession::_PrepareCharacterList(uint32 account_id)
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

bool WorldSession::onAuthResult(XPacket *pGamePct)
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

bool WorldSession::onLogin(XPacket *pRecvPct)
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
    GetSocket()->SendPacket(packet);

    _player->SendLoginProperties();
    return true;
}

bool WorldSession::onMoveRequest(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::vector<Position> vPctInfo{ }, vMoveInfo{ };

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
    speed = 25;
    auto mover = dynamic_cast<Unit *>(_player);

    if (handle == 0 || handle == _player->GetHandle()) {
        // Set Speed if ride
    } else {
        mover = _player->GetSummonByHandle(handle);
        if(mover != nullptr && mover->GetHandle() == handle) {
            npos.m_positionX = x;
            npos.m_positionY = y;
            npos.m_positionZ = 0;

            distance = npos.GetExactDist2d(_player);
            if(distance >= 1800.0f) {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_FAR, 0);
                return true;
            }

            if(distance < 120.0f) {
                speed = (int)((float)speed * 1.1f);
            } else {
                speed = (int)((float)speed * 2.0f);
            }

        }
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

void WorldSession::_SendMoveMsg(Object &obj, Position nPos, std::vector<Position> vMoveInfo)
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

bool WorldSession::onReturnToLobby(XPacket *pRecvPct)
{
    sWorld->RemoveSession(GetAccountId());
    if (_player != nullptr) {
        _player->LogoutNow(2);
        _player->Save(false);
        delete _player;
        _player = nullptr;
    }
    if(pRecvPct != nullptr)
        _SendResultMsg(pRecvPct->GetPacketID(), 0, 0);
    return true;
}

bool WorldSession::onCreateCharacter(XPacket *pRecvPct)
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

bool WorldSession::checkCharacterName(std::string szName)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_NAMECHECK);
    stmt->setString(0, szName);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        return false;
    }
    return true;
}

bool WorldSession::onCharacterName(XPacket *pRecvPct)
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

bool WorldSession::onChatRequest(XPacket *_packet)
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
            _player->PendWarp((float) std::stoi(tokenizer[1], nullptr, 0), (float) std::stoi(tokenizer[2], nullptr, 0), 0);
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
        GetSocket()->SendPacket(result);
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

bool WorldSession::onLogoutTimerRequest(XPacket *pRecvPct)
{
    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
    return true;
}

bool WorldSession::onPutOnItem(XPacket *_packet)
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
                Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_ACCESS_DENIED, 0);
                return true;
            }

            if (_player->putonItem((ItemWearType) position, ci) == 0) {
                _player->CalculateStat();
                Messages::SendStatInfo(_player, _player);
                Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_SUCCESS, 0);
                if (true) { // TODO: isPlayer()
                    _player->SendWearInfo();
                }
            }
        }
    }
    return true;
}

bool WorldSession::onPutOffItem(XPacket *_packet)
{
    _packet->read_skip(7);
    auto position      = _packet->read<uint8_t>();
    auto target_handle = _packet->read<uint>();

    if (position == WearTwoHand)
        position = WearWeapon;

    //if(!_player->isAlive())
    if (false)
        Messages::SendResult(_player, TS_CS_PUTOFF_ITEM, 5, 0);

    Item *curitem = _player->GetWornItem((ItemWearType) position);
    if (curitem == nullptr) {
        Messages::SendResult(_player, TS_CS_PUTOFF_ITEM, 1, 0);
    } else {
        uint16_t por = _player->putoffItem((ItemWearType) position);
        _player->CalculateStat();
        Messages::SendStatInfo(_player, _player);
        Messages::SendResult(_player, _packet->GetPacketID(), 0, 0);
        if (por == 0) {
            if (true) {// TODO IsPlayer
                _player->SendWearInfo();
            }
        }
    }
    return true;
}

bool WorldSession::onRegionUpdate(XPacket *pRecvPct)
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

bool WorldSession::onGetSummonSetupInfo(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    bool showDialog = pRecvPct->read<uint8_t>() == 1;
    Messages::SendCreatureEquipMessage(_player, showDialog);
    return true;
}

bool WorldSession::onContact(XPacket *pRecvPct)
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

bool WorldSession::onDialog(XPacket *pRecvPct)
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

bool WorldSession::onBuyItem(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto item_code = pRecvPct->read<int>();
    auto buy_count = pRecvPct->read<uint16_t>();

    auto szMarketName = _player->GetLastContactStr("market");
    if (buy_count == 0) {
        MX_LOG_TRACE("network", "onBuyItem - %s: buy_count was 0!", _player->GetName());
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return false;
    }

    auto market = sObjectMgr->GetMarketInfo(szMarketName);
    if (market.empty()) {
        MX_LOG_TRACE("network", "onBuyItem - %s: market was empty!", _player->GetName());
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
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
                Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
                return true;
            }
            // TODO Add Huntaholic Check
            // TODO Add Weight Check
            uint32_t uid = 0;

            auto result = _player->ChangeGold(_player->GetGold() - nTotalPrice);
            if (result != TS_RESULT_SUCCESS) {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), result, 0);
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
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item_code);
            XPacket resultPct(TS_SC_NPC_TRADE_INFO);
            resultPct << (uint8_t) 0;
            resultPct << item_code;
#if EPIC > 4
            resultPct << (int64) mt.huntaholic_ratio;
#endif
            resultPct << (uint32_t) _player->GetLastContactLong("npc");
            GetSocket()->SendPacket(resultPct);
        }
    }

}

bool WorldSession::onDeleteCharacter(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto name = pRecvPct->ReadString(19);
    auto stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_CHARACTER);
    stmt->setString(0, name);
    stmt->setInt32(1, _accountId);
    CharacterDatabase.Execute(stmt);
    //Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

bool WorldSession::onChangeLocation(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();

    _player->ChangeLocation(x, y, true, true);
    return true;
}

bool WorldSession::onTimeSync(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto packet_time = pRecvPct->read<int>();
    uint ct          = sWorld->GetArTime();
    _player->m_TS.onEcho(ct - packet_time);
    if (_player->m_TS.m_vT.size() >= 4) {
        XPacket result(TS_SC_SET_TIME);
        result << (uint32_t) _player->m_TS.GetInterval();
        GetSocket()->SendPacket(result);
        return true;
    } else {
        Messages::SendTimeSynch(_player);
        return true;
    }
    ACE_NOTREACHED(return true);
}

bool WorldSession::onGameTime(XPacket *pRecvPct)
{
    Messages::SendGameTime(_player);
    return true;
}

bool WorldSession::onQuery(XPacket *pRecvPct)
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

bool WorldSession::onUpdate(XPacket *pRecvPct)
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

bool WorldSession::onJobLevelUp(XPacket *pRecvPct)
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
    Messages::SendPropertyMessage(_player, cr, "jp", cr->GetJP());
    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, target);
    return true;
}

bool WorldSession::onLearnSkill(XPacket *pRecvPct)
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

bool WorldSession::onEquipSummon(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return false;

    pRecvPct->read_skip(7);

    auto bShowDialog = pRecvPct->read<bool>();
    int card_handle[6] = {0};
    for (int &i : card_handle) {
        i = pRecvPct->read<uint>();
    }

    if(false /*IsItemUseable()*/)
        return true;

    int nCFL = _player->GetCurrentSkillLevel(SkillId::CreatureControl);
    if(nCFL < 0)
        return true;

    if(nCFL > 6)
        nCFL = 6;

    Item* pItem = nullptr;
    Summon* summon = nullptr;
    for(int i = 0; i < 6; ++i) {
        bool bFound = false;
        pItem = nullptr;
        if(card_handle[i] != 0) {
            pItem = _player->FindItemByHandle(card_handle[i]);
            if(pItem != nullptr) {
                if(pItem->m_pItemBase.group != 13 ||
                        _player->GetHandle() != pItem->m_Instance.OwnerHandle ||
                        (pItem->m_Instance.Flag & (uint)FlagBits::FB_Summon) == 0)
                    continue;
            }
        }
        for(int j = 0; j < 6; j++) {
            if(pItem != nullptr) {
                // Belt Slot Card
            }
        }
        if(bFound)
            continue;

        if(_player->m_aBindSummonCard[i] != nullptr) {
            if(pItem == nullptr || _player->m_aBindSummonCard[i]->m_nHandle != pItem->m_nHandle)
            {
                summon = _player->m_aBindSummonCard[i]->m_pSummon;
                if(card_handle[i] == 0)
                    _player->m_aBindSummonCard[i] = nullptr;
                if(summon != nullptr && !summon->IsInWorld()) {
                    for(int k = 0; k < 24; ++k) {
                        if(summon->GetWornItem((ItemWearType)k) != nullptr)
                            summon->putoffItem((ItemWearType)k);
                    }
                }
            }
        }

        if(pItem != nullptr) {
            if((pItem->m_Instance.Flag & FlagBits::FB_Summon) != 0) {
                summon = pItem->m_pSummon;
                if(summon == nullptr) {
                    summon = sMemoryPool->AllocNewSummon(_player, pItem);
                    summon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete);
                    _player->AddSummon(summon, true);
                    Messages::SendItemMessage(_player, pItem);

                    Summon::DB_InsertSummon(_player, summon);
                    auto strCode = std::to_string(summon->GetSummonCode());
                    auto strHandle = std::to_string(summon->GetHandle());
                    sScriptingMgr->RunString(_player, "on_first_summon(" + strCode + ","s + strHandle +")"s);
                    summon->CalculateStat();
                }
                summon->m_cSlotIdx = (uint8_t)i;
                summon->CalculateStat();
            }
            _player->m_aBindSummonCard[i] = pItem;
        }
    }
    if(nCFL > 1) {
        for(int i = 0; i < 6; ++i) {
            if(_player->m_aBindSummonCard[i] == nullptr) {
                for(int x = i+1; x < 6; ++x) {
                    if(_player->m_aBindSummonCard[x] != nullptr) {
                        _player->m_aBindSummonCard[i] = _player->m_aBindSummonCard[x];
                        _player->m_aBindSummonCard[x] = nullptr;
                    }
                }
            }
        }
    }
    Messages::SendCreatureEquipMessage(_player, bShowDialog);
    return true;
}

bool WorldSession::onSellItem(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return false;

    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();
    auto sell_count = pRecvPct->read<uint16>();

    auto item = _player->FindItemByHandle(handle);
    if(item == nullptr || item->m_Instance.OwnerHandle != _player->GetHandle()) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return true;
    }
    if(sell_count == 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return false;
    }
    //if(!_player.IsSelllable) @todo

    auto nPrice = sObjectMgr->GetItemSellPrice(item->m_pItemBase.price, item->m_pItemBase.rank, item->m_Instance.nLevel, item->m_Instance.Code >= 602700 && item->m_Instance.Code <= 602799);
    auto nResultCount = item->m_Instance.nCount - sell_count;
    auto nEnhanceLevel = (item->m_Instance.nLevel + 100 * item->m_Instance.nEnhance);
    if(nResultCount < 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item->m_Instance.Code);
        return false;
    }
    if(_player->GetGold() + sell_count * nPrice > MAX_GOLD_FOR_INVENTORY) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return true;
    }
    if(!_player->Erase(item, sell_count, false)) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item->m_Instance.Code);
        return true;
    }
    auto nPrevGold = _player->GetGold();
    if(_player->ChangeGold(_player->GetGold() + sell_count * nPrice) != 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return false;
    }

    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item->m_Instance.Code);
    XPacket tradePct(TS_SC_NPC_TRADE_INFO);
    tradePct << (uint8)1;
    tradePct << item->m_Instance.Code;
    tradePct << (int64)sell_count;
    tradePct << (int64)sell_count * nPrice;
    tradePct << (uint)_player->GetLastContactLong("npc");
    _player->SendPacket(tradePct);
    return true;
}

bool WorldSession::onSkill(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return false;

    pRecvPct->read_skip(7);
    auto skill_id = pRecvPct->read<uint16>();
    auto caster = pRecvPct->read<uint32>();
    auto target = pRecvPct->read<uint32>();
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();
    auto z = pRecvPct->read<float>();
    auto layer = pRecvPct->read<uint8>();
    auto skill_level = pRecvPct->read<uint8>();

    if(_player->GetHealth() == 0)
        return true;

    WorldObject* pTarget{nullptr};
    Position pos{};
    pos.Relocate(x, y, z);

    auto pCaster = dynamic_cast<Unit*>(_player);
    if(caster != _player->GetHandle())
        pCaster = _player->GetSummonByHandle(caster);

    if(pCaster == nullptr || !pCaster->IsInWorld()) {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
        return false;
    }
    auto base = sObjectMgr->GetSkillBase(skill_id);
    if(base.id == 0 || base.is_valid == 0 || base.is_valid == 2) {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_ACCESS_DENIED);
        return false;
    }
    /// @todo isCastable
    if(target != 0) {
        pTarget = dynamic_cast<WorldObject*>(sMemoryPool->getPtrFromId(target));
        if(pTarget == nullptr) {
            Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
            return false;
        }
    }

    auto ct = sWorld->GetArTime();
    if(pCaster->IsMoving(ct)) {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_ACTABLE);
        return true;
    }

    // @todo is_spell_act
    auto skill = pCaster->GetSkill(skill_id);
    if(skill != nullptr && skill->m_nSkillUID != -1) {
        //if(skill_level > skill->skill_level /* +skill.m_nSkillLevelAdd*/)
            //skill_level = skill_level + skill.m_nSkillLevelAdd;
        int res = pCaster->CastSkill(skill_id, skill_level, target, pos, pCaster->GetLayer(), false);
        if(res != 0)
            Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, res);
    }
    return true;
}

bool WorldSession::onSetProperty(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::string key = pRecvPct->ReadString(16);
    if(key != "client_info"s)
        return false;

    std::string value = pRecvPct->ReadString(pRecvPct->size() - 16 - 7);
    _player->SetClientInfo(value);

    return true;
}
