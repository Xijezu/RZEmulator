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
#include "WorldSession.h"
#include "World.h"
#include "ClientPackets.h"
#include "AuthNetwork.h"
#include "MemPool.h"
#include "Messages.h"
#include "Scripting/XLua.h"

#include "Encryption/MD5.h"
#include "RegionContainer.h"
#include "NPC.h"
#include "ObjectMgr.h"
#include "Skill.h"
#include "GameRule.h"
#include "AllowedCommandInfo.h"
#include "MixManager.h"
#include "GroupManager.h"
#include "GameContent.h"

// Constructo - give it a socket
WorldSession::WorldSession(XSocket *socket) : _socket(socket)
{
}

// Close patch file descriptor before leaving
WorldSession::~WorldSession()
{
    if (m_pPlayer)
        onReturnToLobby(nullptr);
}

void WorldSession::OnClose()
{
    if (_accountName.length() > 0)
        sAuthNetwork.SendClientLogoutToAuth(_accountName);
    if (m_pPlayer)
        onReturnToLobby(nullptr);
    if (_socket)
        _socket->CloseSocket();
}

enum eStatus
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED
};

typedef struct WorldSessionHandler
{
    uint16_t cmd;
    uint8_t  status;
    void (WorldSession::*handler)(XPacket *);
} GameHandler;

constexpr WorldSessionHandler packetHandler[] =
                                      {
                                              {TS_CS_VERSION,               STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                              {TS_CS_VERSION2,              STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                              {TS_CS_PING,                  STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                              {TS_CS_UNKN,                  STATUS_CONNECTED, &WorldSession::HandleNullPacket},
                                              {TS_CS_REPORT,                STATUS_CONNECTED, &WorldSession::HandleNullPacket},
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
                                              {TS_TRADE,                    STATUS_AUTHED,    &WorldSession::onTrade},
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
											  {TS_CS_DROP_ITEM,				STATUS_AUTHED,	  &WorldSession::onDropItem },
                                              {TS_CS_RESURRECTION,          STATUS_AUTHED,    &WorldSession::onRevive},
                                              {TS_CS_SOULSTONE_CRAFT,       STATUS_AUTHED,    &WorldSession::onSoulStoneCraft},
                                              {TS_CS_STORAGE,               STATUS_AUTHED,    &WorldSession::onStorage},
                                              {TS_CS_BIND_SKILLCARD,        STATUS_AUTHED,    &WorldSession::onBindSkillCard},
                                              {TS_CS_UNBIND_SKILLCARD,      STATUS_AUTHED,    &WorldSession::onUnBindSkilLCard},
                                              {TS_CS_TARGETING,             STATUS_AUTHED,    &WorldSession::HandleNullPacket}, // @Todo: Do proper handling here
                                              {TS_CS_DROP_QUEST,            STATUS_AUTHED,    &WorldSession::onDropQuest},	
                                      };

constexpr int tableSize = (sizeof(packetHandler) / sizeof(WorldSessionHandler));

/// Handler for incoming packets
ReadDataHandlerResult WorldSession::ProcessIncoming(XPacket *pRecvPct)
{
            ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int  i    = 0;

    for (i = 0; i < tableSize; i++)
    {
        if ((uint16_t)packetHandler[i].cmd == _cmd && (packetHandler[i].status == STATUS_CONNECTED || (_isAuthed && packetHandler[i].status == STATUS_AUTHED)))
        {
            //pRecvPct->read_skip(7); // Ignoring packet header
            (*this.*packetHandler[i].handler)(pRecvPct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == tableSize)
    {
        NG_LOG_DEBUG("network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), _socket->GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Ok;
    }
    return ReadDataHandlerResult::Ok;
}

/// TODO: The whole stuff needs a rework, it is working as intended but it's just a dirty hack
void WorldSession::onAccountWithAuth(XPacket *pGamePct)
{
    s_ClientWithAuth_CS *result = ((s_ClientWithAuth_CS *)(pGamePct)->contents());
    std::transform(std::begin(result->account), std::end(result->account), std::begin(result->account), ::tolower);
    sAuthNetwork.SendAccountToAuth(*this, result->account, result->one_time_key);
}

void WorldSession::_SendResultMsg(uint16 _msg, uint16 _result, int _value)
{
    XPacket packet(TS_SC_RESULT);
    packet << (uint16)_msg;
    packet << (uint16)_result;
    packet << (int32)_value;
    _socket->SendPacket(packet);
}

void WorldSession::onCharacterList(XPacket */*pGamePct*/)
{
    XPacket packet(TS_SC_CHARACTER_LIST);
    packet << (uint32)time(nullptr);
    packet << (uint16)0;
    auto info = _PrepareCharacterList(_accountId);
    packet << (uint16)info.size();
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
        packet << (uint8)0;
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
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        do
        {
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
            for (int i = 0; i < 5; i++)
            {
                info.model_id[i] = (*result)[12 + i].GetInt32();
            }
            info.szCreateTime = (*result)[17].GetString();
            info.szDeleteTime = (*result)[18].GetString();
            PreparedStatement *wstmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_WEARINFO);
            wstmt->setInt32(0, sid);
            if (PreparedQueryResult wresult = CharacterDatabase.Query(wstmt))
            {
                do
                {
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

    auto szAccount  = pGamePct->ReadString(61);
    auto nAccountID = pGamePct->read<uint>();
    auto result     = pGamePct->read<uint16>();
    m_nPermission = pGamePct->read<int>();
    if (result == TS_RESULT_SUCCESS)
    {
        _isAuthed    = true;
        _accountId   = nAccountID;
        _accountName = szAccount;
        sWorld.AddSession(this);
    }
    _SendResultMsg(TS_CS_ACCOUNT_WITH_AUTH, result, 0);
}

void WorldSession::onLogin(XPacket *pRecvPct)
{
    s_ClientLogin_CS *result = ((s_ClientLogin_CS *)(pRecvPct)->contents());

    //m_pPlayer = new Player(this);
    m_pPlayer = sMemoryPool.AllocPlayer();
    m_pPlayer->SetSession(this);
    if (!m_pPlayer->ReadCharacter(result->szName, _accountId))
    {
        m_pPlayer->DeleteThis();
        return;
    }

    Messages::SendTimeSynch(m_pPlayer);

    sScriptingMgr.RunString(m_pPlayer, string_format("on_login('%s')", m_pPlayer->GetName()));

    XPacket packet(TS_SC_LOGIN_RESULT); // Login Result
    packet << (uint8)1;
    packet << m_pPlayer->GetHandle();
    packet << m_pPlayer->GetPositionX();
    packet << m_pPlayer->GetPositionY();
    packet << m_pPlayer->GetPositionZ();
    packet << (uint8)m_pPlayer->GetLayer();
    packet << (uint32)m_pPlayer->GetOrientation();
    packet << (uint32)sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE);
    packet << (uint32)m_pPlayer->GetHealth();
    packet << (uint16)m_pPlayer->GetMana();
    packet << (uint32)m_pPlayer->GetMaxHealth();
    packet << (uint16)m_pPlayer->GetMaxMana();
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_HAVOC);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_HAVOC);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_SEX);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_RACE);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_MODEL + 1);
    packet << m_pPlayer->GetUInt32Value(UNIT_FIELD_MODEL);
    packet.fill(result->szName, 19);
    packet << (uint32)sWorld.getIntConfig(CONFIG_CELL_SIZE);
    packet << m_pPlayer->GetUInt32Value(PLAYER_FIELD_GUILD_ID);
    GetSocket()->SendPacket(packet);

    m_pPlayer->SendLoginProperties();
}

void WorldSession::onMoveRequest(XPacket *pRecvPct)
{

    std::vector<Position> vPctInfo{ }, vMoveInfo{ };

    auto handle     = pRecvPct->read<uint32_t>();
    auto x          = pRecvPct->read<float>();
    auto y          = pRecvPct->read<float>();
    auto curr_time  = pRecvPct->read<uint32_t>();
    auto speed_sync = pRecvPct->read<uint8_t>() != 0;
    auto count      = pRecvPct->read<uint16_t>();

    if (m_pPlayer == nullptr || m_pPlayer->GetHealth() == 0 || !m_pPlayer->IsInWorld() || count == 0)
        return;

    for (int i = 0; i < count; i++)
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

    uint ct = sWorld.GetArTime();
    speed = (int)m_pPlayer->GetMoveSpeed() / 7;
    auto mover = dynamic_cast<Unit *>(m_pPlayer);

    if (handle == 0 || handle == m_pPlayer->GetHandle())
    {
        // Set Speed if ride
    }
    else
    {
        mover = m_pPlayer->GetSummonByHandle(handle);
        if (mover != nullptr && mover->GetHandle() == handle)
        {
            npos.m_positionX = x;
            npos.m_positionY = y;
            npos.m_positionZ = 0;

            distance = npos.GetExactDist2d(m_pPlayer);
            if (distance >= 1800.0f)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_FAR, 0);
                return;
            }

            if (distance < 120.0f)
            {
                speed = (int)((float)speed * 1.1f);
            }
            else
            {
                speed = (int)((float)speed * 2.0f);
            }
        }
    }

    if (mover == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return;
    }

    npos.m_positionX = x;
    npos.m_positionY = y;
    npos.m_positionZ = 0.0f;

    if (x < 0.0f || sWorld.getIntConfig(CONFIG_MAP_WIDTH) < x || y < 0.0f || sWorld.getIntConfig(CONFIG_MAP_HEIGHT) < y || mover->GetExactDist2d(&npos) > 525.0f)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
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
        if (mover->IsPlayer() && GameContent::CollisionToLine(wayPoint.GetPositionX(), wayPoint.GetPositionY(), mi.GetPositionX(), mi.GetPositionY()))
        {
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        curPosFromClient.m_positionX = mi.m_positionX;
        curPosFromClient.m_positionY = mi.m_positionY;
        curPosFromClient.m_positionZ = 0.0f;
        wayPoint.m_positionX         = curPosFromClient.m_positionX;
        wayPoint.m_positionY         = curPosFromClient.m_positionY;
        wayPoint.m_positionZ         = curPosFromClient.m_positionZ;
        wayPoint._orientation        = curPosFromClient._orientation;
        if (mi.m_positionX < 0.0f || sWorld.getIntConfig(CONFIG_MAP_WIDTH) < mi.m_positionX ||
            mi.m_positionY < 0.0f || sWorld.getIntConfig(CONFIG_MAP_HEIGHT) < mi.m_positionY)
        {
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        vMoveInfo.emplace_back(wayPoint);
    }

    if (vMoveInfo.empty())
        return;

    Position cp = vMoveInfo.back();
    if (mover->IsPlayer() && GameContent::IsBlocked(cp.GetPositionX(), cp.GetPositionY()))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
        return;
    }

    if (mover->IsInWorld())
    {
        if (mover->GetTargetHandle() != 0)
            mover->CancelAttack();
        if (mover->m_nMovableTime <= ct)
        {
            if (mover->IsActable() && mover->IsMovable() && mover->IsInWorld())
            {
                auto tpos2 = mover->GetCurrentPosition(curr_time);
                if (!vMoveInfo.empty())
                {
                    cp = vMoveInfo.back();
                    npos.m_positionX  = cp.GetPositionX();
                    npos.m_positionY  = cp.GetPositionY();
                    npos.m_positionZ  = cp.GetPositionZ();
                    npos._orientation = cp.GetOrientation();
                }
                else
                {
                    npos.m_positionX  = 0.0f;
                    npos.m_positionY  = 0.0f;
                    npos.m_positionZ  = 0.0f;
                    npos._orientation = 0.0f;
                }
                if (mover->GetHandle() != m_pPlayer->GetHandle()
                    || sWorld.getFloatConfig(CONFIG_MAP_LENGTH) / 5.0 >= tpos2.GetExactDist2d(&cp)
                    /*|| !m_pPlayer.m_bAutoUsed*/
                    || m_pPlayer->m_nWorldLocationId != 110900)
                {
                    if (vMoveInfo.empty() || sWorld.getFloatConfig(CONFIG_MAP_LENGTH) >= m_pPlayer->GetCurrentPosition(ct).GetExactDist2d(&npos))
                    {
                        if (mover->HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
                            mover->RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED);
                        npos.m_positionX = x;
                        npos.m_positionY = y;
                        npos.m_positionZ = 0.0f;

                        sWorld.SetMultipleMove(mover, npos, vMoveInfo, speed, true, ct, true);
                        // TODO: Mount
                    }
                }
                return;
            } //if (true /* IsActable() && IsMovable() && isInWorld*/)
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return;
        }
        if (!mover->SetPendingMove(vMoveInfo, (uint8_t)speed))
        {
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
            return;
        }
    } // is in world
}

void WorldSession::onReturnToLobby(XPacket *pRecvPct)
{
    if (m_pPlayer != nullptr)
    {
        m_pPlayer->LogoutNow(2);
        m_pPlayer->CleanupsBeforeDelete();
        m_pPlayer->DeleteThis();
        m_pPlayer = nullptr;
    }
    if (pRecvPct != nullptr)
        _SendResultMsg(pRecvPct->GetPacketID(), 0, 0);
}

void WorldSession::onCreateCharacter(XPacket *pRecvPct)
{

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
        auto     playerUID      = sWorld.GetPlayerIndex();
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
        }
        else
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
        itemStmt->setInt32(0, sWorld.GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultWeaponCode);
        itemStmt->setInt32(3, WEAR_WEAPON);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld.GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultArmorCode);
        itemStmt->setInt32(3, WEAR_ARMOR);
        CharacterDatabase.Execute(itemStmt);

        itemStmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_DEFAULT_ITEM);
        itemStmt->setInt32(0, sWorld.GetItemIndex());
        itemStmt->setInt32(1, playerUID);
        itemStmt->setInt32(2, nDefaultBagCode);
        itemStmt->setInt32(3, WEAR_BAG_SLOT);
        CharacterDatabase.Execute(itemStmt);

        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
        return;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
}

bool WorldSession::checkCharacterName(const std::string &szName)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_NAMECHECK);
    stmt->setString(0, szName);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
    {
        return false;
    }
    return true;
}

void WorldSession::onCharacterName(XPacket *pRecvPct)
{

    std::string szName = pRecvPct->read<std::string>();
    if (!checkCharacterName(szName))
    {
        _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
        return;
    }
    _SendResultMsg(pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onChatRequest(XPacket *_packet)
{
    CS_CHATREQUEST request = { };

    _packet->read((uint8 *)&request.szTarget[0], 21);
    *_packet >> request.request_id;
    *_packet >> request.len;
    *_packet >> request.type;
    request.szMsg = _packet->ReadString(request.len);

    if (request.type != 3 && request.szMsg[0] == 47)
    {
        sAllowedCommandInfo.Run(m_pPlayer, request.szMsg);
        return;
    }

    switch (request.type)
    {
        // local chat message: msg
        case 0:
            Messages::SendLocalChatMessage(0, m_pPlayer->GetHandle(), request.szMsg, 0);
            break;
            // Ad chat message: $msg
        case 2:
            Messages::SendGlobalChatMessage(2, m_pPlayer->GetName(), request.szMsg, 0);
            break;

            // Whisper chat message: "player msg
        case 3:
        {
            auto target = Player::FindPlayer(request.szTarget);
            if (target != nullptr)
            {
                // Todo: Denal check
                Messages::SendChatMessage((m_pPlayer->GetPermission() > 0 ? 7 : 3), m_pPlayer->GetName(), target, request.szMsg);
                Messages::SendResult(m_pPlayer, _packet->GetPacketID(), TS_RESULT_SUCCESS, 0);
                return;
            }
            Messages::SendResult(m_pPlayer, _packet->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        }
            break;
            // Global chat message: !msg
        case 4:
            Messages::SendGlobalChatMessage(m_pPlayer->GetPermission() > 0 ? 6 : 4, m_pPlayer->GetName(), request.szMsg, 0);
            break;
        case 0xA:
        {
            if (m_pPlayer->GetPartyID() != 0)
            {
                sGroupManager.DoEachMemberTag(m_pPlayer->GetPartyID(), [=, &request](PartyMemberTag &tag) {
                    if (tag.bIsOnline)
                    {
                        auto player = Player::FindPlayer(tag.strName);
                        if (player != nullptr)
                        {
                            Messages::SendChatMessage(0xA, m_pPlayer->GetName(), player, request.szMsg);
                        }
                    }
                });
            }
        }
            break;
        default:
            break;
    }
}

void WorldSession::onLogoutTimerRequest(XPacket *pRecvPct)
{
    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onPutOnItem(XPacket *_packet)
{
    auto position      = _packet->read<uint8_t>();
    auto item_handle   = _packet->read<uint>();
    auto target_handle = _packet->read<uint>();

    if (m_pPlayer->GetHealth() != 0)
    {
        //Item *ci = sMemoryPool.FindItem(item_handle);
        auto ci = sMemoryPool.GetObjectInWorld<Item>(item_handle);

        if (ci != nullptr)
        {
            if (!ci->IsWearable() || m_pPlayer->FindItemBySID(ci->m_Instance.UID) == nullptr)
            {
                Messages::SendResult(m_pPlayer, TS_CS_PUTON_ITEM, TS_RESULT_ACCESS_DENIED, 0);
                return;
            }

            auto *unit = (Unit *)m_pPlayer;
            if (target_handle != 0)
            {
                auto summon = sMemoryPool.GetObjectInWorld<Summon>(target_handle);
                if (summon == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle())
                {
                    Messages::SendResult(m_pPlayer, TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
                    return;
                }
                unit = summon;
            }

            if (unit->Puton((ItemWearType)position, ci) == 0)
            {
                unit->CalculateStat();
                Messages::SendStatInfo(m_pPlayer, unit);
                Messages::SendResult(m_pPlayer, TS_CS_PUTON_ITEM, TS_RESULT_SUCCESS, 0);
                if (unit->IsPlayer())
                {
                    m_pPlayer->SendWearInfo();
                }
            }
        }
    }
}

void WorldSession::onPutOffItem(XPacket *_packet)
{
    auto position      = _packet->read<uint8_t>();
    auto target_handle = _packet->read<uint>();

    if (m_pPlayer->GetHealth() == 0)
    {
        Messages::SendResult(m_pPlayer, TS_CS_PUTOFF_ITEM, 5, 0);
        return;
    }

    auto *unit = (Unit *)m_pPlayer;
    if (target_handle != 0)
    {
        auto summon = sMemoryPool.GetObjectInWorld<Summon>(target_handle);
        if (summon == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle())
        {
            Messages::SendResult(m_pPlayer, TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
            return;
        }
        unit = summon;
    }

    Item *curitem = unit->GetWornItem((ItemWearType)position);
    if (curitem == nullptr)
    {
        Messages::SendResult(m_pPlayer, TS_CS_PUTOFF_ITEM, 1, 0);
    }
    else
    {
        uint16_t por = unit->Putoff((ItemWearType)position);
        unit->CalculateStat();
        Messages::SendStatInfo(m_pPlayer, unit);
        Messages::SendResult(m_pPlayer, _packet->GetPacketID(), 0, 0);
        if (por == 0)
        {
            if (unit->IsPlayer())
            {
                m_pPlayer->SendWearInfo();
            }
        }
    }
}

void WorldSession::onRegionUpdate(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto update_time    = pRecvPct->read<uint>();
    auto x              = pRecvPct->read<float>();
    auto y              = pRecvPct->read<float>();
    auto z              = pRecvPct->read<float>();
    auto bIsStopMessage = pRecvPct->read<bool>();

    if (m_pPlayer->IsInWorld())
    {
        sWorld.onRegionChange(m_pPlayer, update_time, bIsStopMessage);
    }
}

void WorldSession::onGetSummonSetupInfo(XPacket *pRecvPct)
{

    bool showDialog = pRecvPct->read<uint8_t>() == 1;
    Messages::SendCreatureEquipMessage(m_pPlayer, showDialog);
}

void WorldSession::onContact(XPacket *pRecvPct)
{

    auto handle = pRecvPct->read<uint32_t>();
    auto npc    = sMemoryPool.GetObjectInWorld<NPC>(handle);

    if (npc != nullptr)
    {
        m_pPlayer->SetLastContact("npc", handle);
        sScriptingMgr.RunString(m_pPlayer, npc->m_pBase->contact_script);
    }
}

void WorldSession::onDialog(XPacket *pRecvPct)
{

    auto        size    = pRecvPct->read<uint16_t>();
    std::string trigger = pRecvPct->ReadString(size);

    if (trigger.empty())
        return;

    if (!m_pPlayer->IsValidTrigger(trigger))
    {
        if (!m_pPlayer->IsFixedDialogTrigger(trigger))
        {
            NG_LOG_ERROR("scripting", "INVALID SCRIPT TRIGGER!!! [%s][%s]", m_pPlayer->GetName(), trigger.c_str());
            return;
        }
    }

    //auto npc = dynamic_cast<NPC *>(sMemoryPool.getPtrFromId(m_pPlayer->GetLastContactLong("npc")));
    auto npc = sMemoryPool.GetObjectInWorld<NPC>(m_pPlayer->GetLastContactLong("npc"));
    if (npc == nullptr)
    {
        NG_LOG_TRACE("scripting", "onDialog: NPC not found!");
        return;
    }

    sScriptingMgr.RunString(m_pPlayer, trigger);
    if (m_pPlayer->HasDialog())
        m_pPlayer->ShowDialog();
}

void WorldSession::onBuyItem(XPacket *pRecvPct)
{

    auto item_code = pRecvPct->read<uint>();
    auto buy_count = pRecvPct->read<uint16_t>();

    auto szMarketName = m_pPlayer->GetLastContactStr("market");
    if (buy_count == 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    auto market = sObjectMgr.GetMarketInfo(szMarketName);
    if (market->empty())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    bool bJoinable{false};
    Item *pNewItem{nullptr};

    for (auto &mt : *market)
    {
        if (mt.code == item_code)
        {
            auto ibs = sObjectMgr.GetItemBase((uint)item_code);
            if (ibs == nullptr)
                continue;
            if (ibs->flaglist[FLAG_DUPLICATE] == 1)
            {
                bJoinable = true;
            }
            else
            {
                bJoinable     = false;
                if (buy_count != 1)
                    buy_count = 1;
            }

            auto nTotalPrice = (int)floor(buy_count * mt.price_ratio);
            if (nTotalPrice / buy_count != mt.price_ratio || m_pPlayer->GetGold() < nTotalPrice || nTotalPrice < 0)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
                return;
            }

            if (m_pPlayer->m_Attribute.nMaxWeight - m_pPlayer->GetFloatValue(PLAYER_FIELD_WEIGHT) < ibs->weight * buy_count)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_HEAVY, item_code);
                return;
            }
            uint32_t uid = 0;

            auto result = m_pPlayer->ChangeGold(m_pPlayer->GetGold() - nTotalPrice);
            if (result != TS_RESULT_SUCCESS)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), result, 0);
                return;
            }

            if (bJoinable)
            {
                auto item = Item::AllocItem(0, mt.code, buy_count, BY_MARKET, -1, -1, -1, 0, 0, 0, 0, 0);
                if (item == nullptr)
                {
                    NG_LOG_ERROR("entities.item", "ItemID Invalid! %d", mt.code);
                    return;
                }
                pNewItem = m_pPlayer->PushItem(item, buy_count, false);

                if (pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
                    Item::PendFreeItem(item);
            }
            else
            {
                for (int i = 0; i < buy_count; i++)
                {
                    auto item = Item::AllocItem(0, mt.code, 1, BY_MARKET, -1, -1, -1, 0, 0, 0, 0, 0);
                    if (item == nullptr)
                    {
                        NG_LOG_ERROR("entities.item", "ItemID Invalid! %d", mt.code);
                        return;
                    }
                    pNewItem = m_pPlayer->PushItem(item, buy_count, false);
                    if (pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
                        Item::PendFreeItem(item);
                }
            }
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item_code);
            XPacket resultPct(TS_SC_NPC_TRADE_INFO);
            resultPct << (uint8_t)0;
            resultPct << item_code;
            resultPct << (uint64)buy_count;
            resultPct << (uint64)nTotalPrice;
#if EPIC > 4
            resultPct << (int64) mt.huntaholic_ratio;
#endif
            resultPct << (uint32_t)m_pPlayer->GetLastContactLong("npc");
            GetSocket()->SendPacket(resultPct);
        }
    }

}

void WorldSession::onDeleteCharacter(XPacket *pRecvPct)
{

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

    auto x = pRecvPct->read<float>();
    auto y = pRecvPct->read<float>();

    m_pPlayer->ChangeLocation(x, y, true, true);
}

void WorldSession::onTimeSync(XPacket *pRecvPct)
{

    auto packet_time = pRecvPct->read<int>();
    uint ct          = sWorld.GetArTime();
    m_pPlayer->m_TS.onEcho(ct - packet_time);
    if (m_pPlayer->m_TS.m_vT.size() >= 4)
    {
        XPacket result(TS_SC_SET_TIME);
        result << (uint32_t)m_pPlayer->m_TS.GetInterval();
        GetSocket()->SendPacket(result);
    }
    else
    {
        Messages::SendTimeSynch(m_pPlayer);
    }
}

void WorldSession::onGameTime(XPacket */*pRecvPct*/)
{
    Messages::SendGameTime(m_pPlayer);
}

void WorldSession::onQuery(XPacket *pRecvPct)
{

    auto handle = pRecvPct->read<uint>();

    //WorldObject* obj = sMemoryPool.getPtrFromId(handle);
    auto obj = sMemoryPool.GetObjectInWorld<WorldObject>(handle);
    if (obj != nullptr && obj->IsInWorld() && obj->GetLayer() == m_pPlayer->GetLayer() && sRegion.IsVisibleRegion(obj, m_pPlayer) != 0)
    {
        Messages::sendEnterMessage(m_pPlayer, obj, false);
    }
}

void WorldSession::onUpdate(XPacket *pRecvPct)
{

    auto handle = pRecvPct->read<uint>();

    auto unit = dynamic_cast<Unit *>(m_pPlayer);
    if (handle != m_pPlayer->GetHandle())
    {
        // Do Summon stuff here
    }
    if (unit != nullptr)
    {
        unit->OnUpdate();
        return;
    }
}

void WorldSession::onJobLevelUp(XPacket *pRecvPct)
{

    auto target = pRecvPct->read<uint>();

    Unit *cr = dynamic_cast<Player *>(m_pPlayer);
    if (cr == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, target);
        return;
    }
    if (cr->IsPlayer() && cr->GetHandle() != m_pPlayer->GetHandle())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, target);
        return;
    }
    int jp = sObjectMgr.GetNeedJpForJobLevelUp(cr->GetCurrentJLv(), m_pPlayer->GetJobDepth());
    if (cr->GetJP() < jp)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_JP, target);
        return;
    }
    if (jp == 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_LIMIT_MAX, target);
        return;
    }
    cr->SetJP(cr->GetJP() - jp);
    cr->SetCurrentJLv(cr->GetCurrentJLv() + 1);
    cr->CalculateStat();
    if (cr->IsPlayer())
    {
        dynamic_cast<Player *>(cr)->Save(true);
    }
    else
    {
        // Summon
    }

    m_pPlayer->Save(true);
    Messages::SendPropertyMessage(m_pPlayer, cr, "job_level", cr->GetCurrentJLv());
    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, target);
}

void WorldSession::onLearnSkill(XPacket *pRecvPct)
{

    auto target_handle = pRecvPct->read<uint>();
    auto skill_id      = pRecvPct->read<int>();
    //auto skill_level = pRecvPct->read<int16>();

    if (m_pPlayer == nullptr)
        return;

    auto   target       = dynamic_cast<Unit *>(m_pPlayer);
    ushort result       = 0;
    int    jobID        = 0;
    int    value        = 0;

    if (m_pPlayer->GetHandle() != target_handle)
    {
        auto summon = sMemoryPool.GetObjectInWorld<Summon>(target_handle);
        if (summon == nullptr || !summon->IsSummon() || summon->GetMaster() == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle())
        {
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        target = summon;
    }
    int    currentLevel = target->GetBaseSkillLevel(skill_id) + 1;
    //if(skill_level == currentLevel)
    //{
    result = GameContent::IsLearnableSkill(target, skill_id, currentLevel, jobID);
    if (result == TS_RESULT_SUCCESS)
    {
        target->RegisterSkill(skill_id, currentLevel, 0, jobID);
        target->CalculateStat();
    }
    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), result, value);
    //}
}

void WorldSession::onEquipSummon(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto     bShowDialog    = pRecvPct->read<bool>();
    int      card_handle[6] = {0};
    for (int &i : card_handle)
    {
        i = pRecvPct->read<uint>();
    }

    if (false /*IsItemUseable()*/)
        return;

    int nCFL = m_pPlayer->GetCurrentSkillLevel(SKILL_CREATURE_CONTROL);
    if (nCFL < 0)
        return;

    if (nCFL > 6)
        nCFL = 6;

    Item     *pItem  = nullptr;
    Summon   *summon = nullptr;
    for (int i       = 0; i < nCFL; ++i)
    {
        bool bFound = false;
        pItem = nullptr;
        if (card_handle[i] != 0)
        {
            pItem = m_pPlayer->FindItemByHandle(card_handle[i]);
            if (pItem != nullptr && pItem->m_pItemBase != nullptr)
            {
                if (pItem->m_pItemBase->group != 13 ||
                    m_pPlayer->GetHandle() != pItem->m_Instance.OwnerHandle ||
                    (pItem->m_Instance.Flag & (uint)ITEM_FLAG_SUMMON) == 0)
                    continue;
            }
        }
        for (int j = 0; j < nCFL; j++)
        {
            if (pItem != nullptr)
            {
                // Belt Slot Card
            }
        }
        if (bFound)
            continue;

        if (m_pPlayer->m_aBindSummonCard[i] != nullptr)
        {
            if (pItem == nullptr || m_pPlayer->m_aBindSummonCard[i]->m_nHandle != pItem->m_nHandle)
            {
                summon = m_pPlayer->m_aBindSummonCard[i]->m_pSummon;
                if (card_handle[i] == 0)
                    m_pPlayer->m_aBindSummonCard[i] = nullptr;
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
            if ((pItem->m_Instance.Flag & ITEM_FLAG_SUMMON) != 0)
            {
                summon = pItem->m_pSummon;
                if (summon == nullptr)
                {
                    summon = sMemoryPool.AllocNewSummon(m_pPlayer, pItem);
                    summon->SetFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE);
                    m_pPlayer->AddSummon(summon, true);
                    Messages::SendItemMessage(m_pPlayer, pItem);

                    Summon::DB_InsertSummon(m_pPlayer, summon);
                    sScriptingMgr.RunString(m_pPlayer, string_format("on_first_summon( %d, %d)", summon->GetSummonCode(), summon->GetHandle()));
                    summon->SetCurrentJLv(summon->GetLevel());
                    summon->CalculateStat();
                }
                summon->m_cSlotIdx = (uint8_t)i;
                summon->CalculateStat();
            }
            m_pPlayer->m_aBindSummonCard[i] = pItem;
        }
    }
    if (nCFL > 1)
    {
        for (int i = 0; i < nCFL; ++i)
        {
            if (m_pPlayer->m_aBindSummonCard[i] == nullptr)
            {
                for (int x = i + 1; x < nCFL; ++x)
                {
                    if (m_pPlayer->m_aBindSummonCard[x] != nullptr)
                    {
                        m_pPlayer->m_aBindSummonCard[i] = m_pPlayer->m_aBindSummonCard[x];
                        m_pPlayer->m_aBindSummonCard[x] = nullptr;
                    }
                }
            }
        }
    }
    Messages::SendCreatureEquipMessage(m_pPlayer, bShowDialog);
}

void WorldSession::onSellItem(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto handle     = pRecvPct->read<uint>();
    auto sell_count = pRecvPct->read<uint16>();

    auto item = m_pPlayer->FindItemByHandle(handle);
    if (item == nullptr || item->m_pItemBase == nullptr || item->m_Instance.OwnerHandle != m_pPlayer->GetHandle() || !item->IsInInventory())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, 0);
        return;
    }
    if (sell_count == 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_UNKNOWN, 0);
        return;
    }
    //if(!m_pPlayer.IsSelllable) @todo

    auto nPrice        = GameContent::GetItemSellPrice(item->m_pItemBase->price, item->m_pItemBase->rank, item->m_Instance.nLevel, item->m_Instance.Code >= 602700 && item->m_Instance.Code <= 602799);
    auto nResultCount  = item->m_Instance.nCount - sell_count;
    auto nEnhanceLevel = (item->m_Instance.nLevel + 100 * item->m_Instance.nEnhance);

    if (!m_pPlayer->IsSellable(item) || nResultCount < 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item->m_Instance.Code);
        return;
    }
    if (m_pPlayer->GetGold() + sell_count * nPrice > MAX_GOLD_FOR_INVENTORY)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return;
    }
    if (m_pPlayer->GetGold() + sell_count * nPrice < 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item->m_Instance.Code);
        return;
    }
    auto code = item->m_Instance.Code;
    if (!m_pPlayer->EraseItem(item, sell_count))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item->GetHandle());
        return;
    }
    if (m_pPlayer->ChangeGold(m_pPlayer->GetGold() + sell_count * nPrice) != 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item->m_Instance.Code);
        return;
    }

    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, item->GetHandle());
    XPacket tradePct(TS_SC_NPC_TRADE_INFO);
    tradePct << (uint8)1;
    tradePct << code;
    tradePct << (int64)sell_count;
    tradePct << (int64)sell_count * nPrice;
    tradePct << (uint)m_pPlayer->GetLastContactLong("npc");
    m_pPlayer->SendPacket(tradePct);
}

void WorldSession::onSkill(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto skill_id    = pRecvPct->read<uint16>();
    auto caster      = pRecvPct->read<uint32>();
    auto target      = pRecvPct->read<uint32>();
    auto x           = pRecvPct->read<float>();
    auto y           = pRecvPct->read<float>();
    auto z           = pRecvPct->read<float>();
    auto layer       = pRecvPct->read<uint8>();
    auto skill_level = pRecvPct->read<uint8>();

    if (m_pPlayer->GetHealth() == 0)
        return;

    WorldObject *pTarget{nullptr};
    Position    pos{ };
    pos.Relocate(x, y, z);

    auto pCaster = dynamic_cast<Unit *>(m_pPlayer);
    if (caster != m_pPlayer->GetHandle())
        pCaster = m_pPlayer->GetSummonByHandle(caster);

    if (pCaster == nullptr || !pCaster->IsInWorld())
    {
        Messages::SendSkillCastFailMessage(m_pPlayer, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
        return;
    }
    auto base = sObjectMgr.GetSkillBase(skill_id);
    if (base == nullptr || base->id == 0 || base->is_valid == 0 || base->is_valid == 2)
    {
        Messages::SendSkillCastFailMessage(m_pPlayer, caster, target, skill_id, skill_level, pos, TS_RESULT_ACCESS_DENIED);
        return;
    }
    /// @todo isCastable
    if (target != 0)
    {
        //pTarget = dynamic_cast<WorldObject*>(sMemoryPool.getPtrFromId(target));
        pTarget = sMemoryPool.GetObjectInWorld<WorldObject>(target);
        if (pTarget == nullptr)
        {
            Messages::SendSkillCastFailMessage(m_pPlayer, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_EXIST);
            return;
        }
    }

    auto ct = sWorld.GetArTime();
    if (pCaster->IsMoving(ct))
    {
        Messages::SendSkillCastFailMessage(m_pPlayer, caster, target, skill_id, skill_level, pos, TS_RESULT_NOT_ACTABLE);
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
        {
            Messages::SendSkillCastFailMessage(m_pPlayer, caster, target, skill_id, skill_level, pos, res);
            if (skill->m_SkillBase->is_harmful != 0 && pCaster->GetTargetHandle() != 0)
                pCaster->StartAttack(target, false);
        }
    }
}

void WorldSession::onSetProperty(XPacket *pRecvPct)
{

    std::string key = pRecvPct->ReadString(16);
    if (key != "client_info"s)
        return;

    std::string value = pRecvPct->ReadString((uint)(pRecvPct->size() - 16 - 7));
    m_pPlayer->SetClientInfo(value);
}

void WorldSession::KickPlayer()
{
    if (_socket)
        _socket->CloseSocket();
}

void WorldSession::onAttackRequest(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto handle = pRecvPct->read<uint>();
    auto target = pRecvPct->read<uint>();

    if (m_pPlayer->GetHealth() == 0)
        return;

    auto unit = dynamic_cast<Unit *>(m_pPlayer);
    if (handle != m_pPlayer->GetHandle())
        unit = m_pPlayer->GetSummonByHandle(handle);
    if (unit == nullptr)
    {
        Messages::SendCantAttackMessage(m_pPlayer, handle, target, TS_RESULT_NOT_OWN);
        return;
    }

    if (target == 0)
    {
        if (unit->GetTargetHandle() != 0)
            unit->CancelAttack();
        return;
    }

    auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(target);
    if (pTarget == nullptr)
    {
        if (unit->GetTargetHandle() != 0)
        {
            unit->EndAttack();
            return;
        }
        Messages::SendCantAttackMessage(m_pPlayer, handle, target, TS_RESULT_NOT_EXIST);
        return;
    }

    if (!unit->IsEnemy(pTarget, false))
    {
        if (unit->GetTargetHandle() != 0)
        {
            unit->EndAttack();
            return;
        }
        Messages::SendCantAttackMessage(m_pPlayer, pRecvPct->GetPacketID(), target, 0);
        return;
    }

    if ((unit->IsUsingCrossBow() || unit->IsUsingBow()) && unit->IsPlayer() && unit->GetBulletCount() < 1)
    {
        Messages::SendCantAttackMessage(m_pPlayer, handle, target, 0);
        return;
    }

    unit->StartAttack(target, true);
}

void WorldSession::onCancelAction(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto handle     = pRecvPct->read<uint>();
    Unit *cancellor = m_pPlayer->GetSummonByHandle(handle);
    if (cancellor == nullptr || !cancellor->IsInWorld())
        cancellor = dynamic_cast<Unit *>(m_pPlayer);
    if (cancellor->GetHandle() == handle)
    {
        if (cancellor->m_castingSkill != nullptr)
        {
            cancellor->CancelSkill();
        }
        else
        {
            if (cancellor->GetTargetHandle() != 0)
                cancellor->CancelAttack();
        }
    }
}

void WorldSession::onTakeItem(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto item_handle = pRecvPct->read<uint>();

    uint ct = sWorld.GetArTime();

    //auto item = dynamic_cast<Item*>(sMemoryPool.getPtrFromId(item_handle));
    auto item = sMemoryPool.GetObjectInWorld<Item>(item_handle);
    if (item == nullptr || !item->IsInWorld())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        return;
    }

    // TODO: Weight
    if (item->m_Instance.OwnerHandle != 0)
    {
        NG_LOG_ERROR("WorldSession::onTakeItem(): OwnerHandle not null: %s, handle: %u", m_pPlayer->GetName(), item->GetHandle());
        return;
    }

    auto pos = m_pPlayer->GetPosition();
    if (GameRule::GetPickableRange() < item->GetExactDist2d(&pos))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_FAR, item_handle);
        return;
    }

    if (item->IsQuestItem() && !m_pPlayer->IsTakeableQuestItem(item->m_Instance.Code))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        return;
    }

    auto drop_duration = ct - item->m_nDropTime;
    uint ry            = 3000;

    for (int i = 0; i < 3; i++)
    {
        if (item->m_pPickupOrder.hPlayer[i] == 0 && item->m_pPickupOrder.nPartyID[i] == 0)
            break;

        if (item->m_pPickupOrder.nPartyID[i] <= 0 || item->m_pPickupOrder.nPartyID[i] != m_pPlayer->GetPartyID())
        {
            if (item->m_pPickupOrder.hPlayer[i] != m_pPlayer->GetHandle())
            {
                if (drop_duration < ry)
                {
                    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
                    return;
                }
                ry += 1000;
            }
        }
    }

    if (item->m_Instance.Code == 0)
    {
        if (m_pPlayer->GetPartyID() == 0)
        {
            if (m_pPlayer->GetGold() + item->m_Instance.nCount > MAX_GOLD_FOR_INVENTORY)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, item_handle);
                return;
            }
        }
    }

    XPacket resultPct(TS_SC_TAKE_ITEM_RESULT);
    resultPct << item_handle;
    resultPct << m_pPlayer->GetHandle();
    sWorld.Broadcast((uint)(m_pPlayer->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
                     (uint)(m_pPlayer->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), m_pPlayer->GetLayer(), resultPct);
    if (sWorld.RemoveItemFromWorld(item))
    {
        if (m_pPlayer->GetPartyID() != 0)
        {
            if (item->m_Instance.Code != 0)
            {
                ///- Actual Item
                sWorld.procPartyShare(m_pPlayer, item);
            }
            else
            {
                ///- Gold
                std::vector<Player *> vList{ };
                sGroupManager.GetNearMember(m_pPlayer, 400.0f, vList);
                auto incGold = (int64)(item->m_Instance.nCount / (!vList.empty() ? vList.size() : 1));

                for (auto &np : vList)
                {
                    auto nNewGold = incGold + np->GetGold();
                    np->ChangeGold(nNewGold);
                }
                Item::PendFreeItem(item);
            }
            return;
        }
        uint nih = sWorld.procAddItem(m_pPlayer, item, false);
        if (nih != 0)
        { // nih = new item handle
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, nih);
            return;
        }
        Item::PendFreeItem(item);
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
    }
}

void WorldSession::onUseItem(XPacket *pRecvPct)
{

    auto item_handle   = pRecvPct->read<uint>();
    auto target_handle = pRecvPct->read<uint>();
    auto szParameter   = pRecvPct->ReadString(32);

    uint ct = sWorld.GetArTime();

    auto item = m_pPlayer->FindItemByHandle(item_handle);
    if (item == nullptr || item->m_Instance.OwnerHandle != m_pPlayer->GetHandle())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        return;
    }

    if (item->m_pItemBase->type != TYPE_USE && false /*!item->IsUsingItem()*/)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        return;
    }

    if ((item->m_pItemBase->flaglist[FLAG_MOVE] == 0 && m_pPlayer->IsMoving(ct)))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, item_handle);
        return;
    }

    // IsUsableSecroute

    // IsUsableraid

    // Eventmap

    uint16 nResult = m_pPlayer->IsUseableItem(item, nullptr);
    if (nResult != TS_RESULT_SUCCESS)
    {
        if (nResult == TS_RESULT_COOL_TIME)
            Messages::SendItemCoolTimeInfo(m_pPlayer);
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), nResult, item_handle);
        return;
    }

    if (item->m_pItemBase->flaglist[FLAG_TARGET_USE] == 0)
    {
        nResult = m_pPlayer->UseItem(item, nullptr, szParameter);
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), nResult, item_handle);
        if (nResult != 0)
            return;
    }
    else
    {
        //auto unit = dynamic_cast<Unit*>(sMemoryPool.getPtrFromId(target_handle));
        auto unit = sMemoryPool.GetObjectInWorld<Unit>(target_handle);
        if (unit == nullptr || unit->GetHandle() == 0)
        {
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
            return;
        }
        nResult = m_pPlayer->IsUseableItem(item, unit);
        if (nResult == TS_RESULT_SUCCESS)
        {
            nResult = m_pPlayer->UseItem(item, unit, szParameter);
            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), nResult, item_handle);
        }

        if (nResult != TS_RESULT_SUCCESS)
        {
            return;
        }
    }

    XPacket resPct(TS_SC_USE_ITEM_RESULT);
    resPct << item_handle;
    resPct << target_handle;
    m_pPlayer->SendPacket(resPct);
}

bool WorldSession::Update(uint /*diff*/)
{
    return _socket != nullptr;
}

void WorldSession::onRevive(XPacket *)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->GetHealth() != 0)
        return;

    sScriptingMgr.RunString(m_pPlayer, string_format("revive_in_town(%d)", 0));
}

void WorldSession::onDropItem(XPacket *pRecvPct)
{
    auto target = pRecvPct->read<uint>();
	auto count = pRecvPct->read<uint16>();
    auto item = sMemoryPool.GetObjectInWorld<Item>(target);
    if (item != nullptr && item->IsDropable() && count > 0 && (item->m_pItemBase->group != GROUP_SUMMONCARD || !(item->m_Instance.Flag & ITEM_FLAG_SUMMON)))
    {
		m_pPlayer->DropItem(m_pPlayer,item,count);
		Messages::SendDropResult(m_pPlayer, target, true);
    }
	else
	{
		Messages::SendDropResult(m_pPlayer, target, false);
	}
}

void WorldSession::onMixRequest(XPacket *pRecvPct)
{

    struct MixInfo
    {
        uint   handle;
        uint16 count;
    };
    MixInfo main_item{ };
    main_item.handle = pRecvPct->read<uint>();
    main_item.count  = pRecvPct->read<uint16>();
    int                  count = pRecvPct->read<uint16>();
    std::vector<MixInfo> vSubItems{ };
    for (int             i     = 0; i < count; ++i)
    {
        MixInfo mi{ };
        mi.handle = pRecvPct->read<uint>();
        mi.count  = pRecvPct->read<uint16>();
        vSubItems.emplace_back(mi);
    }

    if (count > 9)
    {
        KickPlayer();
        return;
    }

    auto pMainItem         = sMixManager.check_mixable_item(m_pPlayer, main_item.handle, 1);
    if (main_item.handle != 0 && pMainItem == nullptr)
        return;

    std::vector<Item *> pSubItem{ };
    std::vector<uint16> pCountList{ };
    if (count != 0)
    {
        for (auto &mixInfo : vSubItems)
        {
            auto item = sMixManager.check_mixable_item(m_pPlayer, mixInfo.handle, mixInfo.count);
            if (item == nullptr)
                return;
            pSubItem.emplace_back(item);
            pCountList.emplace_back(mixInfo.count);
        }
    }
    auto                mb = sMixManager.GetProperMixInfo(pMainItem, count, pSubItem, pCountList);

    if (mb == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_INVALID_ARGUMENT, 0);
        return;
    }

    switch (mb->type)
    {
        case 0:
            break;
        case MIX_ENHANCE: //EnchantItem without E-Protect Powder
            sMixManager.EnhanceItem(mb, m_pPlayer, pMainItem, count, pSubItem, pCountList);
            return;
        case MIX_ENHANCE_SKILL_CARD:
            sMixManager.EnhanceSkillCard(mb, m_pPlayer, count, pSubItem);
            return;
        case MIX_ENHANCE_WITHOUT_FAIL: //EnchantItem WITH E-Protect Powder
            sMixManager.EnhanceItem(mb, m_pPlayer, pMainItem, count, pSubItem, pCountList);
            return;
        case MIX_ADD_LEVEL_SET_FLAG:
            sMixManager.MixItem(mb, m_pPlayer, pMainItem, count, pSubItem, pCountList);
            return;
        case MIX_RESTORE_ENHANCE_SET_FLAG:
            sMixManager.RepairItem(m_pPlayer, pMainItem, count, pSubItem, pCountList);
            return;
        default:
            break;
    }
}

void WorldSession::onSoulStoneCraft(XPacket *pRecvPct)
{

    if (m_pPlayer->GetLastContactLong("SoulStoneCraft") == 0)
        return;

    auto              craft_item_handle = pRecvPct->read<uint>();
    uint              soulstone_handle[4]{0};
    Item              *pSoulStoneList[4]{nullptr};
    for (unsigned int &i : soulstone_handle)
        i = pRecvPct->read<uint>();

    auto nPrevGold = m_pPlayer->GetGold();
    auto pItem     = m_pPlayer->FindItemByHandle(craft_item_handle);
    if (pItem == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, craft_item_handle);
        return;
    }

    int nSocketCount = pItem->m_pItemBase->socket;
    if (nSocketCount < 1 || nSocketCount > 4)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, craft_item_handle);
        return;
    }

    int      nMaxReplicatableCount = nSocketCount == 4 ? 2 : 1;
    int      nCraftCost            = 0;
    bool     bIsValid              = false;
    for (int i                     = 0; i < nSocketCount; ++i)
    {
        if (soulstone_handle[i] != 0)
        {
            pSoulStoneList[i] = m_pPlayer->FindItemByHandle(soulstone_handle[i]);
            if (pSoulStoneList[i] == nullptr)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, soulstone_handle[i]);
                return;
            }
            if (pSoulStoneList[i]->m_pItemBase->type != TYPE_SOULSTONE
                || pSoulStoneList[i]->m_pItemBase->group != GROUP_SOULSTONE
                || pSoulStoneList[i]->m_pItemBase->iclass != CLASS_SOULSTONE)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, soulstone_handle[i]);
                return;
            }

            int      nReplicatedCount = 0;
            for (int k                = 0; k < pItem->m_pItemBase->socket; ++k)
            {
                if (pItem->m_Instance.Socket[k] != 0 && k != i)
                {
                    auto ibs = sObjectMgr.GetItemBase(pItem->m_Instance.Socket[k]);
                    if (ibs->base_type[0] == pSoulStoneList[i]->m_pItemBase->base_type[0]
                        && ibs->base_type[1] == pSoulStoneList[i]->m_pItemBase->base_type[1]
                        && ibs->base_type[2] == pSoulStoneList[i]->m_pItemBase->base_type[2]
                        && ibs->base_type[3] == pSoulStoneList[i]->m_pItemBase->base_type[3]
                        && ibs->base_var[0][0] == pSoulStoneList[i]->m_pItemBase->base_var[0][0]
                        && ibs->base_var[1][0] == pSoulStoneList[i]->m_pItemBase->base_var[1][0]
                        && ibs->base_var[2][0] == pSoulStoneList[i]->m_pItemBase->base_var[2][0]
                        && ibs->base_var[3][0] == pSoulStoneList[i]->m_pItemBase->base_var[3][0]
                        && ibs->opt_type[0] == pSoulStoneList[i]->m_pItemBase->opt_type[0]
                        && ibs->opt_type[1] == pSoulStoneList[i]->m_pItemBase->opt_type[1]
                        && ibs->opt_type[2] == pSoulStoneList[i]->m_pItemBase->opt_type[2]
                        && ibs->opt_type[3] == pSoulStoneList[i]->m_pItemBase->opt_type[3]
                        && ibs->opt_var[0][0] == pSoulStoneList[i]->m_pItemBase->opt_var[0][0]
                        && ibs->opt_var[1][0] == pSoulStoneList[i]->m_pItemBase->opt_var[1][0]
                        && ibs->opt_var[2][0] == pSoulStoneList[i]->m_pItemBase->opt_var[2][0]
                        && ibs->opt_var[3][0] == pSoulStoneList[i]->m_pItemBase->opt_var[3][0])
                    {
                        nReplicatedCount++;
                        if (nReplicatedCount >= nMaxReplicatableCount)
                        {
                            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ALREADY_EXIST, 0);
                            return;
                        }
                    }
                }
            }
            nCraftCost += pSoulStoneList[i]->m_pItemBase->price / 10;
            bIsValid                  = true;
        }
    }
    if (!bIsValid)
    {
        // maybe log here?
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_INVALID_ARGUMENT, 0);
        return;
    }
    if (nPrevGold < nCraftCost || m_pPlayer->ChangeGold(nPrevGold - nCraftCost) != TS_RESULT_SUCCESS)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
        return;
    }

    int      nEndurance = 0;
    for (int i          = 0; i < 4; ++i)
    {
        if (pItem->m_Instance.Socket[i] != 0 || pSoulStoneList[i] != nullptr)
        {
            if (pSoulStoneList[i] != nullptr)
            {
                nEndurance += pSoulStoneList[i]->m_Instance.nCurrentEndurance;
                pItem->m_Instance.Socket[i] = pSoulStoneList[i]->m_Instance.Code;
                m_pPlayer->EraseItem(pSoulStoneList[i], 1);
            }
            else
            {
                nEndurance += pItem->m_Instance.nCurrentEndurance;
            }
        }
    }

    pItem->SetCurrentEndurance(nEndurance);
    m_pPlayer->Save(false);
    pItem->DBUpdate();
    m_pPlayer->SetLastContact("SoulStoneCraft", 0);
    Messages::SendItemMessage(m_pPlayer, pItem);
    m_pPlayer->CalculateStat();
    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onStorage(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto  handle = pRecvPct->read<uint>();
    auto  mode   = pRecvPct->read<uint8>();
    int64 count  = (int64)pRecvPct->read<int32>();

    if (!m_pPlayer->m_bIsUsingStorage || m_pPlayer->m_castingSkill != nullptr || m_pPlayer->GetUInt32Value(PLAYER_FIELD_TRADE_TARGET) != 0 || !m_pPlayer->IsActable())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, handle);
        return;
    }

    switch ((STORAGE_MODE)mode)
    {
        case ITEM_INVENTORY_TO_STORAGE:
        case ITEM_STORAGE_TO_INVENTORY:
        {
            if (count <= 0)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ENOUGH_MONEY, handle);
                return;
            }

            auto *pItem = sMemoryPool.GetObjectInWorld<Item>(handle);
            if (pItem == nullptr)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, handle);
                return;
            }
            if (pItem->m_Instance.OwnerHandle != m_pPlayer->GetHandle())
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, handle);
                return;
            }
            if (pItem->IsInInventory() && mode == ITEM_INVENTORY_TO_STORAGE)
            {
                if (pItem->m_pSummon != nullptr)
                {
                    for (const auto &v : m_pPlayer->m_aBindSummonCard)
                    {
                        if (v == pItem)
                        {
                            Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, pItem->GetHandle());
                            return;
                        }
                    }
                }
                /*if((pItem->m_Instance.Flag & 0x40) == 0 || m_pPlayer->FindStorageItem(pItem->m_Instance.Code) == nullptr)
                {
                    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_HEAVY, handle); // too heavy??
                    return;
                }*/
                m_pPlayer->MoveInventoryToStorage(pItem, count);
            }
            else if (pItem->IsInStorage() && mode == ITEM_STORAGE_TO_INVENTORY)
            {
                m_pPlayer->MoveStorageToInventory(pItem, count);
            }
            m_pPlayer->Save(true);
            return;
        }
        case GOLD_INVENTORY_TO_STORAGE: // 2
        {
            if (m_pPlayer->GetGold() < count)
                return;
            if (m_pPlayer->GetStorageGold() + count > MAX_GOLD_FOR_STORAGE)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, handle);
                return;
            }
            auto nGold = m_pPlayer->GetGold();
            if (m_pPlayer->ChangeGold(nGold - count) == TS_RESULT_SUCCESS && m_pPlayer->ChangeStorageGold(m_pPlayer->GetStorageGold() + count) == TS_RESULT_SUCCESS)
            {
                m_pPlayer->Save(true);
                return;
            }
        }
            return;
        case GOLD_STORAGE_TO_INVENTORY:
        {
            if (m_pPlayer->GetStorageGold() < count)
                return;
            if (m_pPlayer->GetGold() + count > MAX_GOLD_FOR_INVENTORY)
            {
                Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_MUCH_MONEY, handle);
                return;
            }
            auto nGold = m_pPlayer->GetStorageGold();
            if (m_pPlayer->ChangeStorageGold(nGold - count) == TS_RESULT_SUCCESS && m_pPlayer->ChangeGold(m_pPlayer->GetGold() + count) == TS_RESULT_SUCCESS)
            {
                m_pPlayer->Save(true);
                return;
            }
            return;
        }
        case STORAGE_CLOSE:
            m_pPlayer->m_bIsUsingStorage = false;
            return;
        default:
            break;
    }
}

void WorldSession::onBindSkillCard(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto item_handle   = pRecvPct->read<uint>();
    auto target_handle = pRecvPct->read<uint>();

    auto pItem = sMemoryPool.GetObjectInWorld<Item>(item_handle);
    if (pItem == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        return;
    }
    if (target_handle != m_pPlayer->GetHandle())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, target_handle);
        return;
    }
    if (!pItem->IsInInventory() || pItem->m_Instance.OwnerHandle != m_pPlayer->GetHandle() || pItem->m_pItemBase->group != GROUP_SKILLCARD || pItem->m_hBindedTarget != 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        return;
    }

    auto pSkill = m_pPlayer->GetSkill(pItem->m_pItemBase->skill_id);
    if(pSkill != nullptr && pSkill->GetSkillEnhance() == 0)
    {
        m_pPlayer->BindSkillCard(pItem);
    }
}

void WorldSession::onUnBindSkilLCard(XPacket *pRecvPct)
{

    auto item_handle   = pRecvPct->read<uint>();
    auto target_handle = pRecvPct->read<uint>();

    auto pItem = sMemoryPool.GetObjectInWorld<Item>(item_handle);
    if (pItem == nullptr)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_EXIST, item_handle);
        return;
    }
    if (target_handle != m_pPlayer->GetHandle())
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, target_handle);
        return;
    }
    if (!pItem->IsInInventory() || pItem->m_Instance.OwnerHandle != m_pPlayer->GetHandle() || pItem->m_pItemBase->group != GROUP_SKILLCARD || pItem->m_hBindedTarget == 0)
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_ACCESS_DENIED, item_handle);
        return;
    }

    m_pPlayer->UnBindSkillCard(pItem);
}

void WorldSession::onTrade(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto target_handle = pRecvPct->read<uint>();
    auto mode          = pRecvPct->read<uint8>();

    if (m_pPlayer->m_bIsUsingStorage)
        return;

    if (m_pPlayer->bIsMoving && m_pPlayer->IsInWorld())
    {
        m_pPlayer->CancelTrade(false);
        return;
    }

    if (target_handle == m_pPlayer->GetHandle())
    {
        if (m_pPlayer->GetTargetHandle() != 0)
        {
            auto pTarget = sMemoryPool.GetObjectInWorld<Player>(m_pPlayer->GetTargetHandle());
            pTarget->CancelTrade(false);
        }
        m_pPlayer->CancelTrade(false);
        return;
    }

    switch (mode)
    {
        case TM_REQUEST_TRADE:
            onRequestTrade(target_handle);
            break;
        case TM_ACCEPT_TRADE:
            onAcceptTrade(target_handle);
            break;
        case TM_CANCEL_TRADE:
            onCancelTrade();
            break;
        case TM_REJECT_TRADE:
            onRejectTrade(target_handle);
            break;
        case TM_ADD_ITEM:
            onAddItem(target_handle, pRecvPct);
            break;
        case TM_REMOVE_ITEM:
            onRemoveItem(target_handle, pRecvPct);
            break;
        case TM_ADD_GOLD:
            onAddGold(target_handle, pRecvPct);
            break;
        case TM_FREEZE_TRADE:
            onFreezeTrade();
            break;
        case TM_CONFIRM_TRADE:
            onConfirmTrade(target_handle);
            break;
        default:
            return;
    }
}

void WorldSession::onRequestTrade(uint32 hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (tradeTarget->m_bTrading || m_pPlayer->m_bTrading)
        Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, tradeTarget->GetHandle());
    else if (!m_pPlayer->IsTradableWith(tradeTarget))
        Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_PK_LIMIT, tradeTarget->GetHandle());
    else
    {
        XPacket tradePct(TS_TRADE);
        tradePct << m_pPlayer->GetHandle();
        tradePct << (uint8)TM_REQUEST_TRADE; // mode
        tradeTarget->SendPacket(tradePct);
    }
}

void WorldSession::onAcceptTrade(uint32 hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (m_pPlayer->m_bTrading || tradeTarget->m_bTrading || m_pPlayer->m_hTamingTarget || tradeTarget->m_hTamingTarget)
    {
        Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, 0);
    }
    else
    {
        m_pPlayer->StartTrade(tradeTarget->GetHandle());
        tradeTarget->StartTrade(m_pPlayer->GetHandle());

        XPacket tradePlayerPct(TS_TRADE);
        tradePlayerPct << tradeTarget->GetHandle();
        tradePlayerPct << (uint8)TM_BEGIN_TRADE; // mode
        m_pPlayer->SendPacket(tradePlayerPct);

        XPacket tradeTargetPct(TS_TRADE);
        tradeTargetPct << m_pPlayer->GetHandle();
        tradeTargetPct << (uint8)TM_BEGIN_TRADE; // mode
        tradeTarget->SendPacket(tradeTargetPct);
    }
}

void WorldSession::onCancelTrade()
{
    if (!m_pPlayer->m_bTrading)
        return;

    auto tradeTarget = m_pPlayer->GetTradeTarget();
    if (tradeTarget != nullptr)
        tradeTarget->CancelTrade(true);

    m_pPlayer->CancelTrade(true);
}

void WorldSession::onRejectTrade(uint32 hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    XPacket tradePct(TS_TRADE);
    tradePct << m_pPlayer->GetHandle();
    tradePct << (uint8)TM_REJECT_TRADE;
    tradeTarget->SendPacket(tradePct);
}

void WorldSession::onAddItem(uint32 hTradeTarget, XPacket *pRecvPct)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (!m_pPlayer->m_bTradeFreezed)
    {
        auto handle = pRecvPct->read<uint32>();
        pRecvPct->read<int32>(); // Code
        pRecvPct->read<int64>(); // ID
        auto count = pRecvPct->read<int32>();

        auto item = m_pPlayer->FindItemByHandle(handle);

        if (item == nullptr || item->m_pItemBase == nullptr)
            return;

        if (count <= 0 || count > item->m_Instance.nCount)
        {
            NG_LOG_ERROR("trade", "Add Trade Bug [%s:%d]", m_pPlayer->m_szAccount.c_str(), m_pPlayer->GetHandle());
            // Register block account in game rule?
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
            return;
        }

        if (!m_pPlayer->IsTradable(item))
        {
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, 0);
            return;
        }

        if (m_pPlayer->AddItemToTradeWindow(item, count))
        {
            Messages::SendTradeItemInfo(TM_ADD_ITEM, item, count, m_pPlayer, tradeTarget);
        }
    }
}

void WorldSession::onRemoveItem(uint32 hTradeTarget, XPacket *pRecvPct)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
    {
        m_pPlayer->CancelTrade(false);
        return;
    }

    if (!m_pPlayer->m_bTradeFreezed)
    {
        uint32 handle = pRecvPct->read<uint32>();
        pRecvPct->read<int32>(); // Code
        pRecvPct->read<int64>(); // ID
        int32 count = pRecvPct->read<int32>();

        auto item = m_pPlayer->FindItemByHandle(handle);

        if (item == nullptr || item->m_pItemBase == nullptr)
            return;

        if (m_pPlayer->RemoveItemFromTradeWindow(item, count))
        {
            Messages::SendTradeItemInfo(TM_REMOVE_ITEM, item, count, m_pPlayer, tradeTarget);
        }
        else
        {
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
        }
    }
}

void WorldSession::onAddGold(uint32 hTradeTarget, XPacket *pRecvPct)
{
    if (!m_pPlayer->m_bTradeFreezed)
    {
        auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
        if (!isValidTradeTarget(tradeTarget))
            return;

        pRecvPct->read<uint32>(); // Handle
        pRecvPct->read<int32>(); // Code
        pRecvPct->read<int64>(); // ID

        int64 gold = pRecvPct->read<int32>();
        if (gold <= 0)
        {
            NG_LOG_ERROR("trade", "Add gold Trade Bug [%s:%d]", m_pPlayer->m_szAccount.c_str(), m_pPlayer->GetHandle());
            // Register block account in game rule?
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
            return;
        }

        m_pPlayer->AddGoldToTradeWindow(gold);

        XPacket tradePct(TS_TRADE);
        tradePct << m_pPlayer->GetHandle();
        tradePct << (uint8)TM_ADD_GOLD;
        tradePct << (uint32)0; // Handle
        tradePct << (int32)0; // Code
        tradePct << (int64)0; // ID
        tradePct << (int32)gold;
        tradePct.fill("", 44);
        tradeTarget->SendPacket(tradePct);
        m_pPlayer->SendPacket(tradePct);
    }
    else
    {
        m_pPlayer->CancelTrade(false);
    }
}

void WorldSession::onFreezeTrade()
{
    auto tradeTarget = m_pPlayer->GetTradeTarget();
    if (tradeTarget != nullptr)
    {
        m_pPlayer->FreezeTrade();

        XPacket tradePct(TS_TRADE);
        tradePct << m_pPlayer->GetHandle();
        tradePct << (uint8)TM_FREEZE_TRADE;
        tradeTarget->SendPacket(tradePct);
        m_pPlayer->SendPacket(tradePct);
    }
    else
        m_pPlayer->CancelTrade(false);
}

void WorldSession::onConfirmTrade(uint hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (!m_pPlayer->m_bTrading
        || !tradeTarget->m_bTrading
        || !m_pPlayer->m_bTradeFreezed
        || !tradeTarget->m_bTradeFreezed
        || tradeTarget->GetTradeTarget() != m_pPlayer)
    {
        m_pPlayer->CancelTrade(true);
        tradeTarget->CancelTrade(true);
        return;
    }

    if (m_pPlayer->m_bTradeAccepted)
        return;

    m_pPlayer->ConfirmTrade();

    XPacket tradePct(TS_TRADE);
    tradePct << m_pPlayer->GetHandle();
    tradePct << (uint8)TM_CONFIRM_TRADE;
    tradeTarget->SendPacket(tradePct);
    m_pPlayer->SendPacket(tradePct);

    if (!tradeTarget->m_bTradeAccepted)
        return;

    if (!m_pPlayer->CheckTradeWeight()
        || !tradeTarget->CheckTradeWeight())
    {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        Messages::SendResult(m_pPlayer, TS_TRADE, TS_RESULT_TOO_HEAVY, 0);
        Messages::SendResult(tradeTarget, TS_TRADE, TS_RESULT_TOO_HEAVY, 0);

        return;
    }

    if (!m_pPlayer->CheckTradeItem()
        || !tradeTarget->CheckTradeItem())
    {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        Messages::SendResult(m_pPlayer, TS_TRADE, TS_RESULT_ACCESS_DENIED, 0);
        Messages::SendResult(tradeTarget, TS_TRADE, TS_RESULT_ACCESS_DENIED, 0);

        return;
    }

    int64 tradeGold       = m_pPlayer->GetGold() + tradeTarget->GetTradeGold();
    int64 tradeTargetGold = tradeTarget->GetGold() + m_pPlayer->GetTradeGold();

    bool bExceedGold            = tradeGold > MAX_GOLD_FOR_INVENTORY;
    bool bExceedGoldTradeTarget = tradeTargetGold > MAX_GOLD_FOR_INVENTORY;

    if (bExceedGold || bExceedGoldTradeTarget)
    {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        if (bExceedGold)
        {
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, m_pPlayer->GetHandle());
            Messages::SendResult(tradeTarget, TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, m_pPlayer->GetHandle());
        }

        if (bExceedGoldTradeTarget)
        {
            Messages::SendResult(m_pPlayer, TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, tradeTarget->GetHandle());
            Messages::SendResult(tradeTarget, TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, tradeTarget->GetHandle());
        }
    }
    else
    {
        if (m_pPlayer->m_bTrading
            && tradeTarget->m_bTrading
            && m_pPlayer->m_bTradeFreezed
            && tradeTarget->m_bTradeFreezed
            && m_pPlayer->GetTradeTarget() == tradeTarget
            && tradeTarget->GetTradeTarget() == m_pPlayer
            && m_pPlayer->ProcessTrade())
        {
            XPacket tradePct(TS_TRADE);
            tradePct << m_pPlayer->GetHandle();
            tradePct << (uint8)TM_PROCESS_TRADE;
            tradeTarget->SendPacket(tradePct);
            m_pPlayer->SendPacket(tradePct);
        }
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);
    }
}

bool WorldSession::isValidTradeTarget(Player *pTradeTarget)
{
    return !(pTradeTarget == nullptr || !pTradeTarget->IsInWorld() || pTradeTarget->GetExactDist2d(m_pPlayer) > sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE));
}

void WorldSession::onDropQuest(XPacket *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto quest_id = pRecvPct->read<uint>();
    if (m_pPlayer->DropQuest(quest_id))
    {
        Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_SUCCESS, 0);
        return;
    }
    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_NOT_ACTABLE, 0);
}
