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
#include "AuthNetwork.h"
#include "MemPool.h"
#include "Messages.h"
#include "Scripting/XLua.h"

#include "Encryption/MD5.h"
#include "Map/ArRegion.h"
#include "NPC.h"
#include "ObjectMgr.h"
#include "Skill.h"
#include "GameRule.h"
#include "AllowedCommandInfo.h"
#include "MixManager.h"

// Constructo - give it a socket
WorldSession::WorldSession(WorldSocket<WorldSession> *socket) : _socket(socket)
{
    if(socket)
    {
        socket->AddReference();
    }
}

// Close patch file descriptor before leaving
WorldSession::~WorldSession()
{
    if(_player)
        onReturnToLobby(nullptr);
}

void WorldSession::OnClose()
{
    if (_accountName.length() > 0)
        sAuthNetwork->SendClientLogoutToAuth(_accountName);
    if(_player)
        onReturnToLobby(nullptr);
}

enum eStatus {
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct AuthGameSession {
    uint16_t cmd;
    uint8_t  status;
    void (WorldSession::*handler)(XPacket *);
} GameHandler;

const AuthGameSession packetHandler[] =
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
                                      {TS_CS_MIX,                   STATUS_AUTHED,    &WorldSession::onMixRequest},
                                      {TS_CS_UPDATE,                STATUS_AUTHED,    &WorldSession::onUpdate},
                                      {TS_CS_JOB_LEVEL_UP,          STATUS_AUTHED,    &WorldSession::onJobLevelUp},
                                      {TS_CS_LEARN_SKILL,           STATUS_AUTHED,    &WorldSession::onLearnSkill},
                                      {TS_EQUIP_SUMMON,             STATUS_AUTHED,    &WorldSession::onEquipSummon},
                                      {TS_CS_SELL_ITEM,             STATUS_AUTHED,    &WorldSession::onSellItem},
                                      {TS_CS_SKILL,                 STATUS_AUTHED,    &WorldSession::onSkill},
                                      {TS_CS_SET_PROPERTY,          STATUS_AUTHED,    &WorldSession::onSetProperty},
                                      {TS_CS_ATTACK_REQUEST,        STATUS_AUTHED,    &WorldSession::onAttackRequest},
                                      {TS_CS_CANCEL_ACTION,         STATUS_AUTHED,    &WorldSession::onCancelAction},
                                      {TS_CS_TAKE_ITEM,             STATUS_AUTHED,    &WorldSession::onTakeItem},
                                      {TS_CS_USE_ITEM,              STATUS_AUTHED,    &WorldSession::onUseItem},
                                      {TS_CS_RESURRECTION,          STATUS_AUTHED,    &WorldSession::onRevive},
                                      {TS_CS_DROP_ITEM,             STATUS_AUTHED,    &WorldSession::onDropItem},
                              };

const int tableSize = (sizeof(packetHandler) / sizeof(AuthGameSession));

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
            (*this.*packetHandler[i].handler)(pRecvPct);
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
void WorldSession::onAccountWithAuth(XPacket *pGamePct)
{
    s_ClientWithAuth_CS *result = ((s_ClientWithAuth_CS *) (pGamePct)->contents());
    sAuthNetwork->SendAccountToAuth(*this, result->account, result->one_time_key);
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

void WorldSession::onCharacterList(XPacket */*pGamePct*/)
{
    XPacket packet(TS_SC_CHARACTER_LIST);
    packet << (uint32) time(nullptr);
    packet << (uint16) 0;
    auto info = _PrepareCharacterList(_accountId);
    packet << (uint16) info.size();
    for (auto &i : info)
    {
        packet << i.sex;
        packet << i.race;
        for (int j : i.model_id)
        {
            packet << j;
        }
        for (int j : i.wear_info)
        {
            packet << j;
        }
        packet << i.level;
        packet << i.job;
        packet << i.job_level;
        packet << i.exp;
        packet << i.hp;
        packet << i.mp;
        packet << i.permission;
        packet << (uint8) 0;
        packet.fill(i.name, 19);
        packet << i.skin_color;
        packet.fill(i.szCreateTime, 30);
        packet.fill(i.szDeleteTime, 30);
        for (int j : i.wear_item_enhance_info)
        {
            packet << j;
        }
        for (int j : i.wear_item_level_info)
        {
            packet << j;
        }
    }
    _socket->SendPacket(packet);
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

void WorldSession::onAuthResult(XPacket *pGamePct)
{
    pGamePct->read_skip(7);
    auto szAccount = pGamePct->ReadString(61);
    auto nAccountID = pGamePct->read<uint>();
    auto result = pGamePct->read<uint16>();
    if (result == TS_RESULT_SUCCESS) {
        _isAuthed    = true;
        _accountId   = nAccountID;
        _accountName = szAccount;
        sWorld->AddSession(this);
    }
    _SendResultMsg(TS_CS_ACCOUNT_WITH_AUTH, result, 0);
}

void WorldSession::onLogin(XPacket *pRecvPct)
{
    s_ClientLogin_CS *result = ((s_ClientLogin_CS *) (pRecvPct)->contents());

    //_player = new Player(this);
    _player = sMemoryPool->AllocPlayer();
    _player->SetSession(this);
    if (!_player->ReadCharacter(result->szName, _accountId)) {
        _player->DeleteThis();
        return;
    }

    Messages::SendTimeSynch(_player);

    sScriptingMgr->RunString(_player, string_format("on_login('%s')", _player->GetName()));

    XPacket packet(TS_SC_LOGIN_RESULT); // Login Result
    packet << (uint8) 1;
    packet << _player->GetHandle();
    packet << _player->GetPositionX();
    packet << _player->GetPositionY();
    packet << _player->GetPositionZ();
    packet << (uint8) _player->GetLayer();
    packet << (uint32) _player->GetOrientation();
    packet << (uint32) g_nRegionSize;
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
}

void WorldSession::onMoveRequest(XPacket *pRecvPct)
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
        return;

    for (int i   = 0; i < count; i++)
    {
        Position pos{ };
        pos.m_positionX = pRecvPct->read<float>();
        pos.m_positionY = pRecvPct->read<float>();
        vPctInfo.push_back(pos);
    }

    int      speed;
    float    distance;
    Position npos{ };
    Position curPosFromClient{ };
    Position wayPoint{ };

    uint ct = sWorld->GetArTime();
    speed = (int)_player->GetMoveSpeed() / 7;
    auto mover = dynamic_cast<Unit *>(_player);

    if (handle == 0 || handle == _player->GetHandle())
    {
        // Set Speed if ride
    } else
    {
        mover = _player->GetSummonByHandle(handle);
        if (mover != nullptr && mover->GetHandle() == handle)
        {
            npos.m_positionX = x;
            npos.m_positionY = y;
            npos.m_positionZ = 0;

            distance = npos.GetExactDist2d(_player);
            if (distance >= 1800.0f)
            {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_FAR, 0);
                return;
            }

            if (distance < 120.0f)
            {
                speed = (int)((float)speed * 1.1f);
            } else
            {
                speed = (int)((float)speed * 2.0f);
            }
        }
    }

    if (mover == nullptr)
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return;
    }
    npos.m_positionX = x;
    npos.m_positionY = y;
    npos.m_positionZ = 0.0f;

    if (x < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapWidth", 700000) < x || y < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapHeight", 1000000) < y || mover->GetExactDist2d(&npos) > 525.0f)
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
        return;
    }
    if (speed < 1)
        speed = 1;

    wayPoint.m_positionX  = x;
    wayPoint.m_positionY  = y;
    wayPoint.m_positionZ  = 0.0f;
    wayPoint._orientation = 0.0f;

    for (auto &mi : vPctInfo)
    {
        if (mover->IsPlayer() && sObjectMgr->CollisionToLine(wayPoint.GetPositionX(), wayPoint.GetPositionY(), mi.GetPositionX(), mi.GetPositionY()))
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        curPosFromClient.m_positionX = mi.m_positionX;
        curPosFromClient.m_positionY = mi.m_positionY;
        curPosFromClient.m_positionZ = 0.0f;
        wayPoint.m_positionX         = curPosFromClient.m_positionX;
        wayPoint.m_positionY         = curPosFromClient.m_positionY;
        wayPoint.m_positionZ         = curPosFromClient.m_positionZ;
        wayPoint._orientation        = curPosFromClient._orientation;
        if (mi.m_positionX < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapWidth", 700000) < mi.m_positionX ||
            mi.m_positionY < 0.0f || sConfigMgr->GetFloatDefault("GameContent.MapHeight", 1000000) < mi.m_positionY)
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        vMoveInfo.emplace_back(wayPoint);
    }

    if (vMoveInfo.empty())
        return;

    Position cp = vMoveInfo.back();
    if (sObjectMgr->IsBlocked(cp.GetPositionX(), cp.GetPositionY()))
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
        return;
    }

    if (_player->IsInWorld())
    {
        if(mover->GetTargetHandle() != 0)
            mover->CancelAttack();
        if (mover->m_nMovableTime <= ct)
        {
            if (mover->GetHealth() != 0 && mover->IsInWorld())
            {
                auto tpos2 = mover->GetCurrentPosition(curr_time);
                if (!vMoveInfo.empty())
                {
                    cp = vMoveInfo.back();
                    npos.m_positionX  = cp.GetPositionX();
                    npos.m_positionY  = cp.GetPositionY();
                    npos.m_positionZ  = cp.GetPositionZ();
                    npos._orientation = cp.GetOrientation();
                } else
                {
                    npos.m_positionX  = 0.0f;
                    npos.m_positionY  = 0.0f;
                    npos.m_positionZ  = 0.0f;
                    npos._orientation = 0.0f;
                }
                if (mover->GetHandle() != _player->GetHandle() || sConfigMgr->GetFloatDefault("GameContent.MapLength", 16128.0f) / 5.0 >= tpos2.GetExactDist2d(&cp)
                    /*|| !_player.m_bAutoUsed*/ || !_player->m_nWorldLocationId != 110900)
                {
                    if (vMoveInfo.empty() || sConfigMgr->GetFloatDefault("GameContent.MapLength", 16128.0f) >= _player->GetCurrentPosition(ct).GetExactDist2d(&npos))
                    {
                        if(mover->HasFlag(UNIT_FIELD_STATUS, StatusFlags::MovePending))
                            mover->RemoveFlag(UNIT_FIELD_STATUS, StatusFlags::MovePending);
                        npos.m_positionX = x;
                        npos.m_positionY = y;
                        npos.m_positionZ = 0.0f;

                        sWorld->SetMultipleMove(mover, npos, vMoveInfo, speed, true, ct, true);
                        // TODO: Mount
                    }
                }
                return;
            } //if (true /* IsActable() && IsMovable() && isInWorld */)
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return;
        }
        if (!mover->SetPendingMove(vMoveInfo, (uint8_t)speed))
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return;
        }
    } // is in world
}

void WorldSession::onReturnToLobby(XPacket *pRecvPct)
{
    if (_player != nullptr)
    {
        _player->LogoutNow(2);
        _player->Save(false);
        _player->CleanupsBeforeDelete();
        _player->DeleteThis();
        _player = nullptr;
    }
    if (pRecvPct != nullptr)
        _SendResultMsg(pRecvPct->GetPacketID(), 0, 0);
}

void WorldSession::onCreateCharacter(XPacket *pRecvPct)
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
    if (checkCharacterName(info.name))
    {
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
        for (int i : info.model_id)
        {
            stmt->setUInt32(j++, i);
        }
        auto     playerUID      = sWorld->GetPlayerIndex();
        stmt->setUInt32(j, playerUID);
        CharacterDatabase.Query(stmt);

        int m_wear_item       = info.wear_info[2];
        int nDefaultBagCode   = 490001;
        int nDefaultArmorCode = 220100;
        if (m_wear_item == 602)
            nDefaultArmorCode = 220109;

        int nDefaultWeaponCode = 106100;
        if (info.race == 3)
        {
            nDefaultArmorCode     = 240100;
            if (m_wear_item == 602)
                nDefaultArmorCode = 240109;
            nDefaultWeaponCode    = 112100;
        } else
        {
            if (info.race == 5)
            {
                nDefaultArmorCode     = 230100;
                if (m_wear_item == 602)
                    nDefaultArmorCode = 230109;
                nDefaultWeaponCode    = 103100;
            }
        }

        auto itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultWeaponCode);
        itemStmt->setInt32(3, WearWeapon);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultArmorCode);
        itemStmt->setInt32(3, WearArmor);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld->GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultBagCode);
        itemStmt->setInt32(3, WearBagSlot);
        CharacterDatabase.Execute(itemStmt);

        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
        return;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
}

bool WorldSession::checkCharacterName(const std::string& szName)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_NAMECHECK);
    stmt->setString(0, szName);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        return false;
    }
    return true;
}

void WorldSession::onCharacterName(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::string szName = pRecvPct->read<std::string>();
    if (!checkCharacterName(szName)) {
        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
        return;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onChatRequest(XPacket *_packet)
{
    CS_CHATREQUEST request = { };

    _packet->read_skip(7);
    _packet->read((uint8 *) &request.szTarget[0], 21);
    *_packet >> request.request_id;
    *_packet >> request.len;
    *_packet >> request.type;
    request.szMsg = _packet->ReadString(request.len);

    if (request.type != 3 && request.szMsg[0] == 47) {
        sAllowedCommandInfo->Run(_player, request.szMsg);
        return;
    }

    switch(request.type) {
        // local chat message: msg
        case 0:
            Messages::SendLocalChatMessage(0, _player->GetHandle(), request.szMsg, 0);
            break;
        // Ad chat message: $msg
        case 2:
            Messages::SendGlobalChatMessage(2, _player->GetName(), request.szMsg, 0);
            break;
        // Global chat message: !msg
        case 4:
            Messages::SendGlobalChatMessage(_player->GetPermission() > 0 ? 6 : 4, _player->GetName(), request.szMsg, 0);
            break;
        default:
            break;
    }
}

void WorldSession::onLogoutTimerRequest(XPacket *pRecvPct)
{
    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onPutOnItem(XPacket *_packet)
{
    _packet->read_skip(7);
    auto position      = _packet->read<uint8_t>();
    auto item_handle   = _packet->read<uint>();
    auto target_handle = _packet->read<uint>();

    if (_player->GetHealth() != 0)
    {
        //Item *ci = sMemoryPool->FindItem(item_handle);
        auto ci = sMemoryPool->GetObjectInWorld<Item>(item_handle);

        if (ci != nullptr)
        {
            if (!ci->IsWearable() || _player->FindItemBySID(ci->m_Instance.UID) == nullptr)
            {
                Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_ACCESS_DENIED, 0);
                return;
            }

            auto *unit = (Unit *)_player;
            if (target_handle != 0)
            {
                auto summon = sMemoryPool->GetObjectInWorld<Summon>(target_handle);
                if (summon == nullptr || summon->GetMaster()->GetHandle() != _player->GetHandle())
                {
                    Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
                    return;
                }
                unit = summon;
            }

            if (unit->Puton((ItemWearType)position, ci) == 0)
            {
                unit->CalculateStat();
                Messages::SendStatInfo(_player, unit);
                Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_SUCCESS, 0);
                if (unit->IsPlayer())
                {
                    _player->SendWearInfo();
                }
            }
        }
    }
}

void WorldSession::onPutOffItem(XPacket *_packet)
{
    _packet->read_skip(7);
    auto position      = _packet->read<uint8_t>();
    auto target_handle = _packet->read<uint>();

    if (_player->GetHealth() == 0)
    {
        Messages::SendResult(_player, TS_CS_PUTOFF_ITEM, 5, 0);
        return;
    }

    auto *unit = (Unit *)_player;
    if (target_handle != 0)
    {
        auto summon = sMemoryPool->GetObjectInWorld<Summon>(target_handle);
        if (summon == nullptr || summon->GetMaster()->GetHandle() != _player->GetHandle())
        {
            Messages::SendResult(_player, TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
            return;
        }
        unit = summon;
    }

    Item *curitem = unit->GetWornItem((ItemWearType)position);
    if (curitem == nullptr)
    {
        Messages::SendResult(_player, TS_CS_PUTOFF_ITEM, 1, 0);
    }
    else
    {
        uint16_t por = unit->Putoff((ItemWearType)position);
        unit->CalculateStat();
        Messages::SendStatInfo(_player, unit);
        Messages::SendResult(_player, _packet->GetPacketID(), 0, 0);
        if (por == 0)
        {
            if (unit->IsPlayer())
            {
                _player->SendWearInfo();
            }
        }
    }
}

void WorldSession::onRegionUpdate(XPacket *pRecvPct)
{
    if (_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto update_time = pRecvPct->read<uint>();
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();
    auto z = pRecvPct->read<float>();
    auto bIsStopMessage = pRecvPct->read<bool>();

    sWorld->onRegionChange(_player, update_time, bIsStopMessage);
}

void WorldSession::onGetSummonSetupInfo(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    bool showDialog = pRecvPct->read<uint8_t>() == 1;
    Messages::SendCreatureEquipMessage(_player, showDialog);
}

void WorldSession::onContact(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint32_t>();
    auto npc    = sMemoryPool->GetObjectInWorld<NPC>(handle);

    if (npc != nullptr)
    {
        _player->SetLastContact("npc", handle);
        sScriptingMgr->RunString(_player, npc->m_pBase->contact_script);
    }
}

void WorldSession::onDialog(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto        size    = pRecvPct->read<uint16_t>();
    std::string trigger = pRecvPct->ReadString(size);

    if (trigger.empty())
        return;

    if (!_player->IsValidTrigger(trigger))
    {
        if (!_player->IsFixedDialogTrigger(trigger))
        {
            MX_LOG_ERROR("scripting", "INVALID SCRIPT TRIGGER!!! [%s][%s]", _player->GetName(), trigger.c_str());
            return;
        }
    }

    //auto npc = dynamic_cast<NPC *>(sMemoryPool->getPtrFromId(_player->GetLastContactLong("npc")));
    auto npc = sMemoryPool->GetObjectInWorld<NPC>(_player->GetLastContactLong("npc"));
    if (npc == nullptr)
    {
        MX_LOG_TRACE("scripting", "onDialog: NPC not found!");
        return;
    }

    sScriptingMgr->RunString(_player, trigger);
    if (_player->HasDialog())
        _player->ShowDialog();
}

void WorldSession::onBuyItem(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto item_code = pRecvPct->read<uint>();
    auto buy_count = pRecvPct->read<uint16_t>();

    auto szMarketName = _player->GetLastContactStr("market");
    if (buy_count == 0) {
        MX_LOG_TRACE("network", "onBuyItem - %s: buy_count was 0!", _player->GetName());
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    auto market = sObjectMgr->GetMarketInfo(szMarketName);
    if (market->empty()) {
        MX_LOG_TRACE("network", "onBuyItem - %s: market was empty!", _player->GetName());
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    bool bJoinable{false};

    for (auto& mt : *market) {
        if (mt.code == item_code) {
            auto ibs = sObjectMgr->GetItemBase((uint)item_code);
            if(ibs == nullptr)
                continue;
            if (ibs->flaglist[FLAG_DUPLICATE] == 1) {
                bJoinable = true;
            } else {
                bJoinable     = false;
                if (buy_count != 1)
                    buy_count = 1;
            }

            auto nTotalPrice = (int) floor(buy_count * mt.price_ratio);
            if (nTotalPrice / buy_count != mt.price_ratio || _player->GetGold() < nTotalPrice || nTotalPrice < 0) {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
                return;
            }
            // TODO Add Weight Check
            uint32_t uid = 0;

            auto result = _player->ChangeGold(_player->GetGold() - nTotalPrice);
            if (result != TS_RESULT_SUCCESS) {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), result, 0);
                return;
            }

            if (bJoinable) {
                auto item = Item::AllocItem(0, mt.code, buy_count, GenerateCode::ByMarket, -1, -1, -1, 0, 0, 0, 0, 0);
                _player->PushItem(item, buy_count, false);
            } else
            {
                for (int i = 0; i < buy_count; i++)
                {
                    auto item = Item::AllocItem(0, mt.code, 1, GenerateCode::ByMarket, -1, -1, -1, 0, 0, 0, 0, 0);
                    _player->PushItem(item, buy_count, false);
                }
            }
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item_code);
            XPacket resultPct(TS_SC_NPC_TRADE_INFO);
            resultPct << (uint8_t) 0;
            resultPct << item_code;
            resultPct << (uint64)buy_count;
            resultPct << (uint64)nTotalPrice;
#if EPIC > 4
            resultPct << (int64) mt.huntaholic_ratio;
#endif
            resultPct << (uint32_t) _player->GetLastContactLong("npc");
            GetSocket()->SendPacket(resultPct);
        }
    }

}

void WorldSession::onDeleteCharacter(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto name = pRecvPct->ReadString(19);
    auto stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_CHARACTER);
    stmt->setString(0, name);
    stmt->setInt32(1, _accountId);
    CharacterDatabase.Execute(stmt);
    // Send result message with WorldSession, player is not set yet
    Messages::SendResult(this, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onChangeLocation(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();

    _player->ChangeLocation(x, y, true, true);
}

void WorldSession::onTimeSync(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto packet_time = pRecvPct->read<int>();
    uint ct          = sWorld->GetArTime();
    _player->m_TS.onEcho(ct - packet_time);
    if (_player->m_TS.m_vT.size() >= 4)
    {
        XPacket result(TS_SC_SET_TIME);
        result << (uint32_t)_player->m_TS.GetInterval();
        GetSocket()->SendPacket(result);
    } else
    {
        Messages::SendTimeSynch(_player);
    }
}

void WorldSession::onGameTime(XPacket */*pRecvPct*/)
{
    Messages::SendGameTime(_player);
}

void WorldSession::onQuery(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();

    //WorldObject* obj = sMemoryPool->getPtrFromId(handle);
    auto obj = sMemoryPool->GetObjectInWorld<WorldObject>(handle);
    if (obj != nullptr)
    {
        if (!sArRegion->IsVisibleRegion((uint)(obj->GetPositionX() / g_nRegionSize),
                                        (uint)(obj->GetPositionY() / g_nRegionSize),
                                        (uint)(_player->GetPositionX() / g_nRegionSize),
                                        (uint)(_player->GetPositionY() / g_nRegionSize)))
        {
            MX_LOG_DEBUG("network", "onQuery failed: Not visible region!");
            return;
        }
        Messages::sendEnterMessage(_player, obj, false);
    }
}

void WorldSession::onUpdate(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();

    auto unit = dynamic_cast<Unit*>(_player);
    if(handle != _player->GetHandle()) {
        // Do Summon stuff here
    }
    if(unit != nullptr) {
        unit->OnUpdate();
        return;
    }
}

void WorldSession::onJobLevelUp(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto target = pRecvPct->read<uint>();

    Unit* cr = dynamic_cast<Player*>(_player);
    if(cr == nullptr) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, target);
        return;
    }
    if(cr->IsPlayer() && cr->GetHandle() != _player->GetHandle()) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, target);
        return;
    }
    int jp = sObjectMgr->GetNeedJpForJobLevelUp(cr->GetCurrentJLv(), _player->GetJobDepth());
    if(cr->GetJP() < jp) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_JP, target);
        return;
    }
    if(jp == 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_LIMIT_MAX, target);
        return;
    }
    cr->SetJP(cr->GetJP() -jp);
    cr->SetCurrentJLv(cr->GetCurrentJLv() + 1);
    cr->CalculateStat();
    if(cr->IsPlayer()) {
        dynamic_cast<Player*>(cr)->Save(true);
    } else {
        // Summon
    }

    _player->Save(true);
    Messages::SendPropertyMessage(_player, cr, "job_level", cr->GetCurrentJLv());
    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, target);
}

void WorldSession::onLearnSkill(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto target_handle = pRecvPct->read<uint>();
    auto skill_id = pRecvPct->read<int>();
    //auto skill_level = pRecvPct->read<int16>();

    if(_player == nullptr)
        return;

    auto target = dynamic_cast<Unit*>(_player);
    ushort result = 0;
    int jobID = 0;
    int value = 0;

    if(_player->GetHandle() != target_handle)
    {
        auto summon = sMemoryPool->GetObjectInWorld<Summon>(target_handle);
        if(summon == nullptr || !summon->IsSummon() || summon->GetMaster() == nullptr || summon->GetMaster()->GetHandle() != _player->GetHandle())
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        target = summon;
    }
    int currentLevel = target->GetBaseSkillLevel(skill_id)+1;
    //if(skill_level == currentLevel)
    //{
        result = sObjectMgr->IsLearnableSkill(target, skill_id, currentLevel, jobID);
        if(result == TS_RESULT_SUCCESS)
        {
            target->RegisterSkill(skill_id, currentLevel, 0, jobID);
            // TODO: Hack
            _player->CalculateStat();
            Messages::SendStatInfo(_player, target);
        }
        Messages::SendResult(_player,pRecvPct->GetPacketID(), result, value);
    //}
}

void WorldSession::onEquipSummon(XPacket *pRecvPct)
{
    if (_player == nullptr)
        return;

    pRecvPct->read_skip(7);

    auto     bShowDialog    = pRecvPct->read<bool>();
    int      card_handle[6] = {0};
    for (int &i : card_handle)
    {
        i = pRecvPct->read<uint>();
    }

    if (false /*IsItemUseable()*/)
        return;

    int nCFL = _player->GetCurrentSkillLevel(SkillId::CreatureControl);
    if (nCFL < 0)
        return;

    if (nCFL > 6)
        nCFL = 6;

    Item   *pItem  = nullptr;
    Summon *summon = nullptr;
    for (int i = 0; i < 6; ++i)
    {
        bool bFound = false;
        pItem = nullptr;
        if (card_handle[i] != 0)
        {
            pItem = _player->FindItemByHandle(card_handle[i]);
            if (pItem != nullptr && pItem->m_pItemBase != nullptr)
            {
                if (pItem->m_pItemBase->group != 13 ||
                    _player->GetHandle() != pItem->m_Instance.OwnerHandle ||
                    (pItem->m_Instance.Flag & (uint)FlagBits::FB_Summon) == 0)
                    continue;
            }
        }
        for (int j = 0; j < 6; j++)
        {
            if (pItem != nullptr)
            {
                // Belt Slot Card
            }
        }
        if (bFound)
            continue;

        if (_player->m_aBindSummonCard[i] != nullptr)
        {
            if (pItem == nullptr || _player->m_aBindSummonCard[i]->m_nHandle != pItem->m_nHandle)
            {
                summon = _player->m_aBindSummonCard[i]->m_pSummon;
                if (card_handle[i] == 0)
                    _player->m_aBindSummonCard[i] = nullptr;
                if (summon != nullptr && !summon->IsInWorld())
                {
                    for (int k = 0; k < 24; ++k)
                    {
                        if (summon->GetWornItem((ItemWearType)k) != nullptr)
                            summon->Putoff((ItemWearType)k);
                    }
                }
            }
        }

        if (pItem != nullptr)
        {
            if ((pItem->m_Instance.Flag & FlagBits::FB_Summon) != 0)
            {
                summon = pItem->m_pSummon;
                if (summon == nullptr)
                {
                    summon = sMemoryPool->AllocNewSummon(_player, pItem);
                    summon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete);
                    _player->AddSummon(summon, true);
                    Messages::SendItemMessage(_player, pItem);

                    Summon::DB_InsertSummon(_player, summon);
                    sScriptingMgr->RunString(_player, string_format("on_first_summon( %d, %d)", summon->GetSummonCode(), summon->GetHandle()));
                    summon->CalculateStat();
                }
                summon->m_cSlotIdx = (uint8_t)i;
                summon->CalculateStat();
            }
            _player->m_aBindSummonCard[i] = pItem;
        }
    }
    if (nCFL > 1)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (_player->m_aBindSummonCard[i] == nullptr)
            {
                for (int x = i + 1; x < 6; ++x)
                {
                    if (_player->m_aBindSummonCard[x] != nullptr)
                    {
                        _player->m_aBindSummonCard[i] = _player->m_aBindSummonCard[x];
                        _player->m_aBindSummonCard[x] = nullptr;
                    }
                }
            }
        }
    }
    Messages::SendCreatureEquipMessage(_player, bShowDialog);
}

void WorldSession::onSellItem(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();
    auto sell_count = pRecvPct->read<uint16>();

    auto item = _player->FindItemByHandle(handle);
    if(item == nullptr || item->m_pItemBase == nullptr || item->m_Instance.OwnerHandle != _player->GetHandle()) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return;
    }
    if(sell_count == 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }
    //if(!_player.IsSelllable) @todo

    auto nPrice = sObjectMgr->GetItemSellPrice(item->m_pItemBase->price, item->m_pItemBase->rank, item->m_Instance.nLevel, item->m_Instance.Code >= 602700 && item->m_Instance.Code <= 602799);
    auto nResultCount = item->m_Instance.nCount - sell_count;
    auto nEnhanceLevel = (item->m_Instance.nLevel + 100 * item->m_Instance.nEnhance);
    if(nResultCount < 0) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item->m_Instance.Code);
        return;
    }
    if(_player->GetGold() + sell_count * nPrice > MAX_GOLD_FOR_INVENTORY) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return;
    }
    if(_player->GetGold() + sell_count * nPrice < 0)
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item->m_Instance.Code);
        return;
    }
    auto code = item->m_Instance.Code;
    if(!_player->Erase(item, sell_count, false)) {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item->m_Instance.Code);
        return;
    }
    int64 nPrevGold = _player->GetGold();
    int64 nNewGold = _player->GetGold() + sell_count * nPrice;
    if(_player->ChangeGold(_player->GetGold() + sell_count * nPrice) != 0) {
        MX_LOG_TRACE("entities", "Sold [%d]x [%d] for a total of %d gold [Prev: %d, New: %d]", sell_count, code, sell_count * nPrice, nPrevGold, nNewGold);
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return;
    }

    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item->m_Instance.Code);
    XPacket tradePct(TS_SC_NPC_TRADE_INFO);
    tradePct << (uint8)1;
    tradePct << code;
    tradePct << (int64)sell_count;
    tradePct << (int64)sell_count * nPrice;
    tradePct << (uint)_player->GetLastContactLong("npc");
    _player->SendPacket(tradePct);
}

void WorldSession::onSkill(XPacket *pRecvPct)
{
    if (_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto skill_id    = pRecvPct->read<uint16>();
    auto caster      = pRecvPct->read<uint32>();
    auto target      = pRecvPct->read<uint32>();
    auto x           = pRecvPct->read<float>();
    auto y           = pRecvPct->read<float>();
    auto z           = pRecvPct->read<float>();
    auto layer       = pRecvPct->read<uint8>();
    auto skill_level = pRecvPct->read<uint8>();

    if (_player->GetHealth() == 0)
        return;

    WorldObject *pTarget{nullptr};
    Position pos{ };
    pos.Relocate(x, y, z);

    auto pCaster = dynamic_cast<Unit *>(_player);
    if (caster != _player->GetHandle())
        pCaster = _player->GetSummonByHandle(caster);

    if (pCaster == nullptr || !pCaster->IsInWorld())
    {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
        return;
    }
    auto base = sObjectMgr->GetSkillBase(skill_id);
    if (base == nullptr || base->id == 0 || base->is_valid == 0 || base->is_valid == 2)
    {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_ACCESS_DENIED);
        return;
    }
    /// @todo isCastable
    if (target != 0)
    {
        //pTarget = dynamic_cast<WorldObject*>(sMemoryPool->getPtrFromId(target));
        pTarget = sMemoryPool->GetObjectInWorld<WorldObject>(target);
        if (pTarget == nullptr)
        {
            Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
            return;
        }
    }

    auto ct = sWorld->GetArTime();
    if (pCaster->IsMoving(ct))
    {
        Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_ACTABLE);
        return;
    }

    // @todo is_spell_act
    auto skill = pCaster->GetSkill(skill_id);
    if (skill != nullptr && skill->m_nSkillUID != -1)
    {
        //if(skill_level > skill->skill_level /* +skill.m_nSkillLevelAdd*/)
        //skill_level = skill_level + skill.m_nSkillLevelAdd;
        int res = pCaster->CastSkill(skill_id, skill_level, target, pos, pCaster->GetLayer(), false);
        if (res != 0)
            Messages::SendSkillCastFailMessage(_player, caster, target, skill_id, skill_level, pos, res);
    }
}

void WorldSession::onSetProperty(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    std::string key = pRecvPct->ReadString(16);
    if (key != "client_info"s)
        return;

    std::string value = pRecvPct->ReadString((uint)(pRecvPct->size() - 16 - 7));
    _player->SetClientInfo(value);
}

void WorldSession::KickPlayer()
{
    if(_socket)
        _socket->CloseSocket();
}

void WorldSession::onAttackRequest(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();
    auto target = pRecvPct->read<uint>();

    if(_player->GetHealth() == 0)
        return;

    auto unit = dynamic_cast<Unit*>(_player);
    if(handle != _player->GetHandle())
        unit = _player->GetSummonByHandle(handle);
    if(unit == nullptr) {
        Messages::SendCantAttackMessage(_player, handle, target, TS_RESULT_NOT_OWN);
        return;
    }

    if(target == 0) { // Todo
        return;
    }

    //auto pTarget = sMemoryPool->getPtrFromId(target);
    auto pTarget = sMemoryPool->GetObjectInWorld<WorldObject>(target);
    if(pTarget == nullptr){
        // Todo same as above
        Messages::SendCantAttackMessage(_player, handle, target, TS_RESULT_NOT_EXIST);
        return;
    }

    // TODO isEnemy
    // Todo various checks

    unit->StartAttack(target, true);
}

void WorldSession::onCancelAction(XPacket *pRecvPct)
{
    if(_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto handle = pRecvPct->read<uint>();
    Unit* cancellor = _player->GetSummonByHandle(handle);
    if(cancellor == nullptr || !cancellor->IsInWorld())
        cancellor = dynamic_cast<Unit*>(_player);
    if(cancellor->GetHandle() == handle) {
        if (cancellor->m_castingSkill != nullptr) {
            cancellor->CancelSkill();
        } else {
            if (cancellor->GetTargetHandle() != 0)
                cancellor->CancelAttack();
        }
    }
}

void WorldSession::onTakeItem(XPacket *pRecvPct)
{
    if (_player == nullptr)
        return;

    pRecvPct->read_skip(7);
    auto item_handle = pRecvPct->read<uint>();

    uint ct = sWorld->GetArTime();

    //auto item = dynamic_cast<Item*>(sMemoryPool->getPtrFromId(item_handle));
    auto item = sMemoryPool->GetObjectInWorld<Item>(item_handle);
    if (item == nullptr || !item->IsInWorld())
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        return;
    }

    // TODO: Weight
    if (item->m_Instance.OwnerHandle != 0)
    {
        MX_LOG_ERROR("WorldSession::onTakeItem(): OwnerHandle not null: %s, handle: %u", _player->GetName(), item->GetHandle());
        return;
    }

    auto pos = _player->GetPosition();
    if (GameRule::GetPickableRange() < item->GetExactDist2d(&pos))
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_FAR, item_handle);
        return;
    }

    // Quest Item Check TODO
    if(item->IsQuestItem() && !_player->IsTakeableQuestItem(item->m_Instance.Code))
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        return;
    }

    auto drop_duration = ct - item->m_nDropTime;
    int  ry            = 3000;

    for (int i = 0; i < 3; i++)
    {
        if (item->m_pPickupOrder.hPlayer[i] == 0 && item->m_pPickupOrder.nPartyID[i] == 0)
            break;

        if (item->m_pPickupOrder.nPartyID[i] <= 0 || item->m_pPickupOrder.nPartyID[i] != _player->GetInt32Value(UNIT_FIELD_PARTY_ID))
        {
            if (item->m_pPickupOrder.hPlayer[i] != _player->GetHandle())
            {
                if (drop_duration < ry)
                {
                    Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
                    return;
                }
                ry += 1000;
            }
        }
    }

    if (item->m_Instance.Code == 0)
    {
        if (_player->GetInt32Value(UNIT_FIELD_PARTY_ID) == 0)
        {
            if (_player->GetGold() + item->m_Instance.nCount > MAX_GOLD_FOR_INVENTORY)
            {
                Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item_handle);
                return;
            }
        }
    }

    XPacket resultPct(TS_SC_TAKE_ITEM_RESULT);
    resultPct << item_handle;
    resultPct << _player->GetHandle();
    sWorld->Broadcast((uint)(_player->GetPositionX() / g_nRegionSize), (uint)(_player->GetPositionY() / g_nRegionSize), _player->GetLayer(), resultPct);
    if (sWorld->RemoveItemFromWorld(item))
    {
        // TODO: Party
        uint nih = sWorld->procAddItem(_player, item, false);
        if (nih != 0)
        { // nih = new item handle
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, nih);
            return;
        }
        Item::PendFreeItem(item);
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
    }
}

void WorldSession::onUseItem(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    auto item_handle   = pRecvPct->read<uint>();
    auto target_handle = pRecvPct->read<uint>();
    auto szParameter   = pRecvPct->ReadString(32);

    uint ct = sWorld->GetArTime();

    auto item = _player->FindItemByHandle(item_handle);
    if (item == nullptr || item->m_Instance.OwnerHandle != _player->GetHandle())
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        MX_LOG_TRACE("network", "onUseItem: Not own item!!!");
        return;
    }

    if (item->m_pItemBase->type != ItemType::TypeUse && false /*!item->IsUsingItem()*/)
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        MX_LOG_TRACE("network", "onUseItem: Not usable");
        return;
    }

    if ((item->m_pItemBase->flaglist[FLAG_MOVE] == 0 && _player->IsMoving(ct)))
    {
        MX_LOG_TRACE("network", "onUseItem: Not usable while moving");
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item_handle);
        return;
    }

    // IsUsableSecroute

    // IsUsableraid

    // Eventmap

    uint16 nResult = _player->IsUseableItem(item, nullptr);
    if (nResult != TS_RESULT_SUCCESS)
    {
        /*if(nResult == TS_RESULT_COOL_TIME)
            Messages::SendItemCoolTimeInfo(_player);*/
        MX_LOG_TRACE("network", "onUseItem: Not usable item: %d", nResult);
        Messages::SendResult(_player, pRecvPct->GetPacketID(), nResult, item_handle);
        return;
    }

    if (item->m_pItemBase->flaglist[FLAG_TARGET_USE] == 0)
    {
        nResult = _player->UseItem(item, nullptr, szParameter);
        MX_LOG_TRACE("network", "onUseItem: nResult: %d", nResult);
        Messages::SendResult(_player, pRecvPct->GetPacketID(), nResult, item_handle);
        if (nResult != 0)
            return;
    }
    else
    {
        //auto unit = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(target_handle));
        auto unit = sMemoryPool->GetObjectInWorld<Unit>(target_handle);
        if (unit == nullptr || unit->GetHandle() == 0)
        {
            Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
            return;
        }
        nResult = _player->IsUseableItem(item, unit);
        if (nResult == TS_RESULT_SUCCESS)
        {
            nResult = _player->UseItem(item, unit, szParameter);
            Messages::SendResult(_player, pRecvPct->GetPacketID(), nResult, item_handle);
        }

        if (nResult != TS_RESULT_SUCCESS)
        {
            MX_LOG_TRACE("network", "onItemUse: IsUseableItem failed: %d", nResult);
            return;
        }
    }

    MX_LOG_TRACE("network", "onItemUse: Final nResult: %d", nResult);
    XPacket resPct(TS_SC_USE_ITEM_RESULT);
    resPct << item_handle;
    resPct << target_handle;
    _player->SendPacket(resPct);
}

bool WorldSession::Update(uint diff)
{
    if(_socket && _socket->IsClosed())
    {
        _socket->RemoveReference();
        _socket = nullptr;
    }

    if(!_socket)
        return false;

    return true;
}

void WorldSession::onRevive(XPacket *)
{
    if(_player == nullptr)
        return;

    if(_player->GetHealth() != 0)
        return;

    sScriptingMgr->RunString(_player, string_format("revive_in_town(%d)", 0));
}

void WorldSession::onDropItem(XPacket * pRecvPct)
{
    return;
    pRecvPct->read_skip(7);
    uint target = pRecvPct->read<uint>();

    auto item = sMemoryPool->GetObjectInWorld<Item>(target);
    if(item != nullptr)
    {
        item->SetOwnerInfo(0, 0, 0);
        item->Relocate(_player->GetPosition());
        sWorld->AddItemToWorld(item);
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, target);
    }
}

void WorldSession::onMixRequest(XPacket *pRecvPct)
{
    pRecvPct->read_skip(7);
    struct MixInfo
    {
        uint handle;
        uint16 count;
    };
    MixInfo main_item{};
    main_item.handle = pRecvPct->read<uint>();
    main_item.count = pRecvPct->read<uint16>();
    int count = pRecvPct->read<uint16>();
    std::vector<MixInfo> vSubItems{};
    for(int i = 0; i < count; ++i)
    {
        MixInfo mi{};
        mi.handle = pRecvPct->read<uint>();
        mi.count = pRecvPct->read<uint16>();
        vSubItems.emplace_back(mi);
    }

    if(count > 9)
    {
        KickPlayer();
        return;
    }

    auto pMainItem = sMixManager->check_mixable_item(_player, main_item.handle, 1);
    if(main_item.handle != 0 && pMainItem == nullptr)
        return;

    std::vector<Item*> pSubItem{};
    std::vector<uint16> pCountList{};
    if(count != 0)
    {
        for(auto& mixInfo : vSubItems)
        {
            auto item = sMixManager->check_mixable_item(_player, mixInfo.handle, mixInfo.count);
            if(item == nullptr)
                return;
            pSubItem.emplace_back(item);
            pCountList.emplace_back(mixInfo.count);
        }
    }
    auto mb = sMixManager->GetProperMixInfo(pMainItem, count, pSubItem, pCountList);

    if(mb == nullptr)
    {
        Messages::SendResult(_player, pRecvPct->GetPacketID(), TS_RESULT_INVALID_ARGUMENT, 0);
        return;
    }

    switch(mb->type)
    {
        case 0:
            break;
        case 101:
        case 103:
            sMixManager->EnhanceItem(mb, _player, pMainItem, count, pSubItem, pCountList);
            return;
        case 311:
            sMixManager->MixItem(mb, _player, pMainItem, count, pSubItem, pCountList);
            return;
        default:
            break;
    }
}
