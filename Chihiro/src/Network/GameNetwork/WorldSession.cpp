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

#include "WorldSession.h"

#include "AllowedCommandInfo.h"
#include "AuthNetwork.h"
#include "ClientPackets.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "Encryption/MD5.h"
#include "GameContent.h"
#include "GameRule.h"
#include "GroupManager.h"
#include "MemPool.h"
#include "Messages.h"
#include "MixManager.h"
#include "NPC.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RegionContainer.h"
#include "Scripting/XLua.h"
#include "Skill.h"
#include "World.h"

// Constructo - give it a socket
WorldSession::WorldSession(boost::asio::ip::tcp::socket &&socket)
    : XSocket(std::move(socket))
    , m_nLastPing(sWorld.GetArTime())
{
}

// Close patch file descriptor before leaving
WorldSession::~WorldSession()
{
    sWorld.RemoveSession(GetAccountId());
    if (m_pPlayer)
        onReturnToLobby(nullptr);
}

void WorldSession::OnClose()
{
    sWorld.RemoveSession(GetAccountId());
    if (_accountName.length() > 0)
        sAuthNetwork.SendClientLogoutToAuth(_accountName);
    if (m_pPlayer)
        onReturnToLobby(nullptr);
}

std::string WorldSession::GetAccountName() const
{
    return m_pPlayer != nullptr ? m_pPlayer->GetName() : "<null>";
}

enum eStatus { STATUS_CONNECTED = 0, STATUS_AUTHED };

typedef struct WorldSessionHandler {
    int32_t cmd;
    eStatus status;
    std::function<void(WorldSession *, XPacket *)> handler;
} WorldSessionHandler;

template<typename T>
WorldSessionHandler declareHandler(eStatus status, void (WorldSession::*handler)(const T *packet))
{
    WorldSessionHandler handlerData{};
    handlerData.cmd = T::getId(EPIC_4_1_1);
    handlerData.status = status;
    handlerData.handler = [handler](WorldSession *instance, XPacket *packet) -> void {
        T deserializedPacket;
        MessageSerializerBuffer buffer(packet);
        deserializedPacket.deserialize(&buffer);
        (instance->*handler)(&deserializedPacket);
    };
    return handlerData;
}

const WorldSessionHandler worldPacketHandler[] = {
    declareHandler(STATUS_CONNECTED, &WorldSession::onAuthResult),
    declareHandler(STATUS_CONNECTED, &WorldSession::onAccountWithAuth),
    declareHandler(STATUS_CONNECTED, &WorldSession::onPing),
    declareHandler(STATUS_AUTHED, &WorldSession::onLogoutTimerRequest),
    declareHandler(STATUS_AUTHED, &WorldSession::onReturnToLobby),
    declareHandler(STATUS_AUTHED, &WorldSession::onRequestReturnToLobby),
    declareHandler(STATUS_AUTHED, &WorldSession::onCharacterList),
    declareHandler(STATUS_AUTHED, &WorldSession::onLogin),
    declareHandler(STATUS_AUTHED, &WorldSession::onCharacterName),
    declareHandler(STATUS_AUTHED, &WorldSession::onCreateCharacter),
    declareHandler(STATUS_AUTHED, &WorldSession::onDeleteCharacter),
    declareHandler(STATUS_AUTHED, &WorldSession::onMoveRequest),
    declareHandler(STATUS_AUTHED, &WorldSession::onRegionUpdate),
    declareHandler(STATUS_AUTHED, &WorldSession::onChatRequest),
    declareHandler(STATUS_AUTHED, &WorldSession::onPutOnItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onPutOffItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onGetSummonSetupInfo),
    declareHandler(STATUS_AUTHED, &WorldSession::onContact),
    declareHandler(STATUS_AUTHED, &WorldSession::onDialog),
    declareHandler(STATUS_AUTHED, &WorldSession::onBuyItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onChangeLocation),
    declareHandler(STATUS_AUTHED, &WorldSession::onTimeSync),
    declareHandler(STATUS_AUTHED, &WorldSession::onGameTime),
    declareHandler(STATUS_AUTHED, &WorldSession::onQuery),
    declareHandler(STATUS_AUTHED, &WorldSession::onMixRequest),
    declareHandler(STATUS_AUTHED, &WorldSession::onTrade),
    declareHandler(STATUS_AUTHED, &WorldSession::onUpdate),
    declareHandler(STATUS_AUTHED, &WorldSession::onJobLevelUp),
    declareHandler(STATUS_AUTHED, &WorldSession::onLearnSkill),
    declareHandler(STATUS_AUTHED, &WorldSession::onEquipSummon),
    declareHandler(STATUS_AUTHED, &WorldSession::onSellItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onSkill),
    declareHandler(STATUS_AUTHED, &WorldSession::onSetProperty),
    declareHandler(STATUS_AUTHED, &WorldSession::onAttackRequest),
    declareHandler(STATUS_AUTHED, &WorldSession::onTakeItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onUseItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onDropItem),
    declareHandler(STATUS_AUTHED, &WorldSession::onRevive),
    declareHandler(STATUS_AUTHED, &WorldSession::onSoulStoneCraft),
    declareHandler(STATUS_AUTHED, &WorldSession::onStorage),
    declareHandler(STATUS_AUTHED, &WorldSession::onBindSkillCard),
    declareHandler(STATUS_AUTHED, &WorldSession::onUnBindSkilLCard),
    declareHandler(STATUS_AUTHED, &WorldSession::onDropQuest),
    declareHandler(STATUS_AUTHED, &WorldSession::onCancelAction),
};
constexpr int32_t worldTableSize = (sizeof(worldPacketHandler) / sizeof(WorldSessionHandler));

constexpr NGemity::Packets ignoredPackets[] = {
    NGemity::Packets::TS_CS_VERSION, NGemity::Packets::TS_CS_VERSION2, NGemity::Packets::TS_CS_UNKN, NGemity::Packets::TS_CS_REPORT, NGemity::Packets::TS_CS_TARGETING};

/// Handler for incoming packets
ReadDataHandlerResult WorldSession::ProcessIncoming(XPacket *pRecvPct)
{
    ASSERT(pRecvPct);

    auto _cmd = pRecvPct->GetPacketID();
    int32_t i = 0;

    for (i = 0; i < worldTableSize; i++) {
        if ((uint16_t)worldPacketHandler[i].cmd == _cmd && (worldPacketHandler[i].status == STATUS_CONNECTED || (_isAuthed && worldPacketHandler[i].status == STATUS_AUTHED))) {
            worldPacketHandler[i].handler(this, pRecvPct);
            break;
        }
    }

    // Report unknown packets in the error log
    if (i == worldTableSize && std::find(std::begin(ignoredPackets), std::end(ignoredPackets), (NGemity::Packets)_cmd) == std::end(ignoredPackets)) {
        NG_LOG_DEBUG("server.network", "Got unknown packet '%d' from '%s'", pRecvPct->GetPacketID(), GetRemoteIpAddress().to_string().c_str());
        return ReadDataHandlerResult::Ok;
    }
    return ReadDataHandlerResult::Ok;
}

/// TODO: The whole stuff needs a rework, it is working as intended but it's just a dirty hack
void WorldSession::onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH *pGamePct)
{
    auto szAccount = pGamePct->account;
    std::transform(std::begin(szAccount), std::end(szAccount), std::begin(szAccount), ::tolower);
    sAuthNetwork.SendAccountToAuth(*this, szAccount, pGamePct->one_time_key);
}

void WorldSession::_SendResultMsg(uint16_t _msg, uint16_t _result, int32_t _value)
{
    TS_SC_RESULT resultPct{};
    resultPct.request_msg_id = _msg;
    resultPct.result = _result;
    resultPct.value = _value;
    SendPacket(resultPct);
}

void WorldSession::onCharacterList(const TS_CS_CHARACTER_LIST * /*pGamePct*/)
{
    TS_SC_CHARACTER_LIST characterPct{};
    characterPct.current_server_time = sWorld.GetArTime();
    characterPct.last_character_idx = 0;
    _PrepareCharacterList(_accountId, &characterPct.characters);
    SendPacket(characterPct);
}

/// TODO: Might need to put this in player class?
void WorldSession::_PrepareCharacterList(uint32_t account_id, std::vector<LOBBY_CHARACTER_INFO> *_info)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_CHARACTERLIST);
    stmt->setInt32(0, account_id);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        do {
            LOBBY_CHARACTER_INFO info{};
            int32_t sid = (*result)[0].GetInt32();
            info.name = (*result)[1].GetString();
            info.race = (*result)[2].GetInt32();
            info.sex = (*result)[3].GetInt32();
            info.level = (*result)[4].GetInt32();
            info.job_level = (*result)[5].GetInt32();
            info.exp_percentage = (*result)[6].GetInt32();
            info.hp = (*result)[7].GetInt32();
            info.mp = (*result)[8].GetInt32();
            info.job = (*result)[9].GetInt32();
            info.permission = (*result)[10].GetInt32();
            info.skin_color = (*result)[11].GetUInt32();
            for (int32_t i = 0; i < 5; i++) {
                info.model_id[i] = (*result)[12 + i].GetInt32();
            }
            info.szCreateTime = (*result)[17].GetString();
            info.szDeleteTime = (*result)[18].GetString();
            PreparedStatement *wstmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_WEARINFO);
            wstmt->setInt32(0, sid);
            if (PreparedQueryResult wresult = CharacterDatabase.Query(wstmt)) {
                do {
                    int32_t wear_info = (*wresult)[0].GetInt32();
                    info.wear_info[wear_info] = (*wresult)[1].GetInt32();
                    info.wear_item_enhance_info[wear_info] = (*wresult)[2].GetInt32();
                    info.wear_item_level_info[wear_info] = (*wresult)[3].GetInt32();
                } while (wresult->NextRow());
            }
            _info->emplace_back(info);
        } while (result->NextRow());
    }
}

void WorldSession::onAuthResult(const TS_AG_CLIENT_LOGIN *pRecvPct)
{
    if (pRecvPct->result == TS_RESULT_SUCCESS) {
        _isAuthed = true;
        _accountId = pRecvPct->nAccountID;
        _accountName = pRecvPct->account;
        m_nPermission = pRecvPct->permission;
        sWorld.AddSession(this);
    }
    Messages::SendResult(this, NGemity::Packets::TS_CS_ACCOUNT_WITH_AUTH, pRecvPct->result, 0);
}

void WorldSession::onLogin(const TS_CS_LOGIN *pRecvPct)
{
    m_pPlayer = sMemoryPool.AllocPlayer();
    m_pPlayer->SetSession(this);
    if (!m_pPlayer->ReadCharacter(pRecvPct->name, _accountId)) {
        m_pPlayer->DeleteThis();
        return;
    }

    Messages::SendTimeSynch(m_pPlayer);
    sScriptingMgr.RunString(m_pPlayer, NGemity::StringFormat("on_login('{}')", m_pPlayer->GetName()));

    TS_SC_LOGIN_RESULT resultPct{};
    resultPct.result = 1;
    resultPct.handle = m_pPlayer->GetHandle();
    resultPct.x = m_pPlayer->GetPositionX();
    resultPct.y = m_pPlayer->GetPositionY();
    resultPct.z = m_pPlayer->GetPositionZ();
    resultPct.layer = m_pPlayer->GetLayer();
    resultPct.face_direction = m_pPlayer->GetOrientation();
    resultPct.region_size = sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE);
    resultPct.hp = m_pPlayer->GetHealth();
    resultPct.mp = m_pPlayer->GetMana();
    resultPct.max_hp = m_pPlayer->GetMaxHealth();
    resultPct.max_mp = m_pPlayer->GetMaxMana();
    resultPct.havoc = m_pPlayer->GetInt32Value(UNIT_FIELD_HAVOC);
    resultPct.max_havoc = m_pPlayer->GetInt32Value(UNIT_FIELD_HAVOC);
    resultPct.sex = m_pPlayer->GetInt32Value(UNIT_FIELD_SEX);
    resultPct.race = m_pPlayer->GetRace();
    resultPct.skin_color = m_pPlayer->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    resultPct.faceId = m_pPlayer->GetInt32Value(UNIT_FIELD_MODEL + 1);
    resultPct.hairId = m_pPlayer->GetInt32Value(UNIT_FIELD_MODEL);
    resultPct.name = pRecvPct->name;
    resultPct.cell_size = sWorld.getIntConfig(CONFIG_CELL_SIZE);
    resultPct.guild_id = m_pPlayer->GetGuildID();

    SendPacket(resultPct);
    m_pPlayer->SendLoginProperties();
}

bool GetValidWayPoint(Player *pClient, Unit *pMObj, const TS_CS_MOVE_REQUEST *pMsg, std::vector<Position> &vMoveInfo, Position &startPos)
{
    auto curPosFromServer = pMObj->GetCurrentPosition(sWorld.GetArTime());
    Position wayPoint{pMsg->x, pMsg->y};

    if (wayPoint.GetPositionX() < 0 || wayPoint.GetPositionX() > sWorld.getIntConfig(CONFIG_MAP_WIDTH) || wayPoint.GetPositionY() < 0 ||
        wayPoint.GetPositionY() > sWorld.getIntConfig(CONFIG_MAP_HEIGHT)) {
        Messages::SendResult(pClient, pMsg->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);
        return false;
    }

    if (curPosFromServer.GetExactDist2d(&wayPoint) > GameRule::VISIBLE_RANGE) {
        Messages::SendResult(pClient, pMsg->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);
        return false;
    }

    if (pMObj->IsPlayer()) {
        if (GameContent::CollisionToLine(curPosFromServer.GetPositionX(), curPosFromServer.GetPositionY(), wayPoint.GetPositionX(), wayPoint.GetPositionY())) {
            Messages::SendResult(pClient, pMsg->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);

            if (GameContent::IsBlocked(curPosFromServer.GetPositionX(), curPosFromServer.GetPositionY()))
                return false;

            startPos = curPosFromServer;
            vMoveInfo.clear();
            vMoveInfo.emplace_back(curPosFromServer);
            return true;
        }
    }

    for (const auto &mv : pMsg->move_infos) {
        if (mv.tx < 0 || mv.tx > sWorld.getIntConfig(CONFIG_MAP_WIDTH) || mv.ty < 0 || mv.ty > sWorld.getIntConfig(CONFIG_MAP_HEIGHT)) {
            Messages::SendResult(pClient, pMsg->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);
            return false;
        }

        if (pMObj->IsPlayer()) {
            if (GameContent::CollisionToLine(wayPoint.GetPositionX(), wayPoint.GetPositionY(), mv.tx, mv.ty)) {
                Messages::SendResult(pClient, pMsg->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);
                if (GameContent::IsBlocked(curPosFromServer.GetPositionX(), curPosFromServer.GetPositionY()))
                    return false;

                startPos = curPosFromServer;
                vMoveInfo.clear();
                vMoveInfo.emplace_back(curPosFromServer);
                return true;
            }
        }

        wayPoint.Relocate(mv.tx, mv.ty);
        vMoveInfo.emplace_back(wayPoint);
    }

    startPos.Relocate(pMsg->x, pMsg->y);
    return true;
}

void WorldSession::onMoveRequest(const TS_CS_MOVE_REQUEST *pRecvPct)
{
    if (m_pPlayer == nullptr || m_pPlayer->IsDead() || !m_pPlayer->IsInWorld())
        return;

    auto t = sWorld.GetArTime();

    Unit *pMObj = m_pPlayer;
    auto speed = m_pPlayer->GetRealMoveSpeed();
    if (pRecvPct->handle != 0 && pRecvPct->handle != m_pPlayer->GetHandle()) {
        pMObj = m_pPlayer->GetSummonByHandle(pRecvPct->handle);
        if (pMObj != nullptr) {
            if (m_pPlayer->IsRiding() && m_pPlayer->GetRideObject() == pMObj) {
                pMObj = m_pPlayer;
            }
            else {
                speed = pMObj->GetRealMoveSpeed();
                if (pRecvPct->speed_sync != 0) {
                    speed = (m_pPlayer->HasRidingState() ? m_pPlayer->GetRealRidingSpeed() : m_pPlayer->GetRealMoveSpeed());

                    Position pos{pRecvPct->x, pRecvPct->y};
                    auto distance = pos.GetExactDist2d(m_pPlayer);
                    if (distance >= GameRule::SUMMON_FOLLOWING_LIMIT_RANGE) {
                        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_FAR, 0);
                        return;
                    }
                    else if (distance >= GameRule::SUMMON_FOLLOWING_SECOND_SPEED_UP_RANGE) {
                        speed *= GameRule::SUMMON_FOLLOWING_SECOND_SPEED_UP_RATE;
                    }
                    else if (distance >= GameRule::SUMMON_FOLLOWING_FIRST_SPEED_UP_RANGE) {
                        speed *= GameRule::SUMMON_FOLLOWING_FIRST_SPEED_UP_RATE;
                    }
                }
            }
        }
        // Todo Epic 5: Pet handle)
    }

    if (pMObj == m_pPlayer && (m_pPlayer->IsRiding() || m_pPlayer->HasRidingState()))
        speed = m_pPlayer->GetRealRidingSpeed();

    if (pMObj == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, 0);
        return;
    }

    speed = speed < 1 ? 1 : speed;
    speed = speed > UINT8_MAX ? UINT8_MAX : speed;
    auto moveSpeed = static_cast<uint8_t>(speed);

    std::vector<Position> vMoveInfo{};
    Position start_pos{};
    if (!GetValidWayPoint(m_pPlayer, pMObj, pRecvPct, vMoveInfo, start_pos))
        return;

    if (vMoveInfo.empty())
        return;

    if (!m_pPlayer->IsInWorld())
        return;

    if (pMObj->IsAttacking())
        pMObj->CancelAttack();

    if (pMObj->GetNextMovableTime() > t) {
        if (!pMObj->SetPendingMove(vMoveInfo, moveSpeed))
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, 0);
        return;
    }

    if (!pMObj->IsActable() || !pMObj->IsInWorld()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, 0);
        return;
    }

    if (m_pPlayer == pMObj && m_pPlayer->IsSitDown()) {
        m_pPlayer->Standup();
        Messages::BroadcastStatusMessage(m_pPlayer);
    }

    if (!pMObj->IsMovable()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, 0);
        return;
    }

    auto pos = pMObj->GetCurrentPosition(sWorld.GetArTime());
    auto targetPos = vMoveInfo.back();

    if (pMObj == m_pPlayer && pos.GetExactDist2d(&start_pos) > (sWorld.getFloatConfig(CONFIG_MAP_LENGTH) / 5)) {
        // Check for Abyss
    }

    if (!vMoveInfo.empty() && pMObj->GetCurrentPosition(t).GetExactDist2d(&targetPos) > sWorld.getFloatConfig(CONFIG_MAP_LENGTH))
        return;

    if (pMObj->HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
        pMObj->RemoveFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED);

    sWorld.SetMultipleMove(pMObj, start_pos, vMoveInfo, moveSpeed, true, t, true);

    if (pMObj->IsPlayer() && pMObj->As<Player>()->IsRiding())
        sWorld.SetMultipleMove(pMObj->As<Player>()->GetRideObject(), start_pos, vMoveInfo, moveSpeed, true, t, true);

    /*if (!pMObj->SetPendingMove(vMoveInfo, (uint8_t)speed))
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, 0);*/
}

void WorldSession::onReturnToLobby(const TS_CS_RETURN_LOBBY *pRecvPct)
{
    if (m_pPlayer != nullptr) {
        m_pPlayer->LogoutNow(2);
        m_pPlayer->CleanupsBeforeDelete();
        m_pPlayer->DeleteThis();
        m_pPlayer = nullptr;
    }
    if (pRecvPct != nullptr)
        _SendResultMsg(pRecvPct->getReceivedId(), 0, 0);
}

void WorldSession::onRequestReturnToLobby(const TS_CS_REQUEST_RETURN_LOBBY *pRecvPct)
{
    _SendResultMsg(pRecvPct->getReceivedId(), 0, 0);
}

void WorldSession::onCreateCharacter(const TS_CS_CREATE_CHARACTER *pRecvPct)
{
    if (checkCharacterName(pRecvPct->character.name)) {
        uint8_t j = 0;
        PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_CHARACTER);
        stmt->setString(j++, pRecvPct->character.name);
        stmt->setString(j++, _accountName);
        stmt->setInt32(j++, _accountId);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, pRecvPct->character.race);
        stmt->setInt32(j++, pRecvPct->character.sex);
        stmt->setInt32(j++, 0);
        stmt->setInt32(j++, pRecvPct->character.job);
        stmt->setInt32(j++, pRecvPct->character.job_level);
        stmt->setInt32(j++, pRecvPct->character.exp_percentage);
        stmt->setInt32(j++, 320);
        stmt->setInt32(j++, 320);
        stmt->setUInt32(j++, pRecvPct->character.skin_color);
        for (const auto &i : pRecvPct->character.model_id) {
            stmt->setUInt32(j++, i);
        }
        auto playerUID = sWorld.GetPlayerIndex();
        stmt->setUInt32(j, playerUID);
        CharacterDatabase.Query(stmt);

        int32_t m_wear_item = pRecvPct->character.wear_info[2];
        int32_t nDefaultBagCode = 490001;
        int32_t nDefaultArmorCode = 220100;
        if (m_wear_item == 602)
            nDefaultArmorCode = 220109;

        int32_t nDefaultWeaponCode = 106100;
        if (pRecvPct->character.race == 3) {
            nDefaultArmorCode = 240100;
            if (m_wear_item == 602)
                nDefaultArmorCode = 240109;
            nDefaultWeaponCode = 112100;
        }
        else {
            if (pRecvPct->character.race == 5) {
                nDefaultArmorCode = 230100;
                if (m_wear_item == 602)
                    nDefaultArmorCode = 230109;
                nDefaultWeaponCode = 103100;
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

        _SendResultMsg(pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
        return;
    }
    _SendResultMsg(pRecvPct->getReceivedId(), TS_RESULT_ALREADY_EXIST, 0);
}

bool WorldSession::checkCharacterName(const std::string &szName)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_NAMECHECK);
    stmt->setString(0, szName);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        return false;
    }
    return true;
}

void WorldSession::onCharacterName(const TS_CS_CHECK_CHARACTER_NAME *pRecvPct)
{
    if (!checkCharacterName(pRecvPct->name)) {
        _SendResultMsg(pRecvPct->getReceivedId(), TS_RESULT_ALREADY_EXIST, 0);
        return;
    }
    _SendResultMsg(pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onChatRequest(const TS_CS_CHAT_REQUEST *pRectPct)
{
    if (pRectPct->type != CHAT_WHISPER && pRectPct->message[0] == 47) {
        sAllowedCommandInfo.Run(m_pPlayer, pRectPct->message);
        return;
    }

    switch (pRectPct->type) {
    // local chat message: msg
    case CHAT_NORMAL:
        Messages::SendLocalChatMessage(0, m_pPlayer->GetHandle(), pRectPct->message, 0);
        break;
        // Ad chat message: $msg
    case CHAT_ADV:
        Messages::SendGlobalChatMessage(2, m_pPlayer->GetName(), pRectPct->message, 0);
        break;

        // Whisper chat message: "player msg
    case CHAT_WHISPER: {
        auto target = Player::FindPlayer(pRectPct->szTarget);
        if (target != nullptr) {
            // Todo: Denal check
            Messages::SendChatMessage((m_pPlayer->GetPermission() > 0 ? 7 : 3), m_pPlayer->GetName(), target, pRectPct->message);
            Messages::SendResult(m_pPlayer, pRectPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
            return;
        }
        Messages::SendResult(m_pPlayer, pRectPct->getReceivedId(), TS_RESULT_NOT_EXIST, 0);
    } break;
        // Global chat message: !msg
    case CHAT_GLOBAL:
        Messages::SendGlobalChatMessage(m_pPlayer->GetPermission() > 0 ? 6 : 4, m_pPlayer->GetName(), pRectPct->message, 0);
        break;
    case CHAT_PARTY: {
        if (m_pPlayer->GetPartyID() != 0) {
            sGroupManager.DoEachMemberTag(m_pPlayer->GetPartyID(), [this, &pRectPct](PartyMemberTag &tag) {
                if (tag.bIsOnline) {
                    auto player = Player::FindPlayer(tag.strName);
                    if (player != nullptr) {
                        Messages::SendChatMessage(0xA, m_pPlayer->GetName(), player, pRectPct->message);
                    }
                }
            });
        }
    } break;
    default:
        break;
    }
}

void WorldSession::onLogoutTimerRequest(const TS_CS_REQUEST_LOGOUT *pRecvPct)
{
    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onPutOnItem(const TS_CS_PUTON_ITEM *pRecvPct)
{
    if (m_pPlayer->GetHealth() != 0) {
        auto ci = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
        if (ci != nullptr) {
            if (!ci->IsWearable() || m_pPlayer->FindItemBySID(ci->GetItemInstance().GetUID()) == nullptr) {
                Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTON_ITEM, TS_RESULT_ACCESS_DENIED, 0);
                return;
            }

            auto unit = m_pPlayer->As<Unit>();
            if (pRecvPct->target_handle != 0) {
                auto summon = sMemoryPool.GetObjectInWorld<Summon>(pRecvPct->target_handle);
                if (summon == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle()) {
                    Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
                    return;
                }
                unit = summon;
            }

            if (unit->Puton((ItemWearType)pRecvPct->position, ci) == 0) {
                unit->CalculateStat();
                Messages::SendStatInfo(m_pPlayer, unit);
                Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTON_ITEM, TS_RESULT_SUCCESS, 0);
                if (unit->IsPlayer()) {
                    m_pPlayer->SendWearInfo();
                }
            }
        }
    }
}

void WorldSession::onPutOffItem(const TS_CS_PUTOFF_ITEM *pRecvPct)
{
    if (m_pPlayer->GetHealth() == 0) {
        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTOFF_ITEM, 5, 0);
        return;
    }

    auto unit = m_pPlayer->As<Unit>();
    if (pRecvPct->target_handle != 0) {
        auto summon = sMemoryPool.GetObjectInWorld<Summon>(pRecvPct->target_handle);
        if (summon == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle()) {
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTON_ITEM, TS_RESULT_NOT_EXIST, 0);
            return;
        }
        unit = summon;
    }

    Item *curitem = unit->GetWornItem((ItemWearType)pRecvPct->position);
    if (curitem == nullptr) {
        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_CS_PUTOFF_ITEM, 1, 0);
    }
    else {
        uint16_t por = unit->Putoff((ItemWearType)pRecvPct->position);
        unit->CalculateStat();
        Messages::SendStatInfo(m_pPlayer, unit);
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
        if (por == 0) {
            if (unit->IsPlayer()) {
                m_pPlayer->SendWearInfo();
            }
        }
    }
}

void WorldSession::onRegionUpdate(const TS_CS_REGION_UPDATE *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->IsInWorld()) {
        sWorld.onRegionChange(m_pPlayer, pRecvPct->update_time, pRecvPct->bIsStopMessage);
    }
}

void WorldSession::onGetSummonSetupInfo(const TS_CS_GET_SUMMON_SETUP_INFO *pRecvPct)
{
    Messages::SendCreatureEquipMessage(m_pPlayer, pRecvPct->show_dialog);
}

void WorldSession::onContact(const TS_CS_CONTACT *pRecvPct)
{
    auto npc = sMemoryPool.GetObjectInWorld<NPC>(pRecvPct->handle);

    if (npc != nullptr) {
        m_pPlayer->SetLastContact("npc", pRecvPct->handle);
        sScriptingMgr.RunString(m_pPlayer, npc->m_pBase->contact_script);
    }
}

void WorldSession::onDialog(const TS_CS_DIALOG *pRecvPct)
{
    if (pRecvPct->trigger.empty())
        return;

    if (!m_pPlayer->IsValidTrigger(pRecvPct->trigger)) {
        if (!m_pPlayer->IsFixedDialogTrigger(pRecvPct->trigger)) {
            NG_LOG_ERROR("server.scripting", "INVALID SCRIPT TRIGGER!!! [%s][%s]", m_pPlayer->GetName(), pRecvPct->trigger.c_str());
            return;
        }
    }

    // auto npc = dynamic_cast<NPC *>(sMemoryPool.getPtrFromId(m_pPlayer->GetLastContactLong("npc")));
    auto npc = sMemoryPool.GetObjectInWorld<NPC>(m_pPlayer->GetLastContactLong("npc"));
    if (npc == nullptr) {
        NG_LOG_TRACE("server.scripting", "onDialog: NPC not found!");
        return;
    }

    sScriptingMgr.RunString(m_pPlayer, pRecvPct->trigger);
    if (m_pPlayer->HasDialog())
        m_pPlayer->ShowDialog();
}

void WorldSession::onBuyItem(const TS_CS_BUY_ITEM *pRecvPct)
{
    auto szMarketName = m_pPlayer->GetLastContactStr("market");
    auto buy_count = pRecvPct->buy_count;
    if (buy_count == 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    auto market = sObjectMgr.GetMarketInfo(szMarketName);
    if (market->empty()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_UNKNOWN, 0);
        return;
    }

    bool bJoinable{false};
    Item *pNewItem{nullptr};

    for (auto &mt : *market) {
        if (mt.code == static_cast<uint32_t>(pRecvPct->item_code)) {
            auto ibs = sObjectMgr.GetItemBase((uint32_t)pRecvPct->item_code);
            if (ibs == nullptr)
                continue;
            if (ibs->flaglist[FLAG_DUPLICATE] == 1) {
                bJoinable = true;
            }
            else {
                bJoinable = false;
                if (buy_count != 1)
                    buy_count = 1;
            }

            auto nTotalPrice = (int32_t)floor(buy_count * mt.price_ratio);
            if (nTotalPrice / buy_count != mt.price_ratio || m_pPlayer->GetGold() < nTotalPrice || nTotalPrice < 0) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
                return;
            }

            if (m_pPlayer->m_Attribute.nMaxWeight - m_pPlayer->GetFloatValue(PLAYER_FIELD_WEIGHT) < ibs->weight * buy_count) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_HEAVY, pRecvPct->item_code);
                return;
            }
            uint32_t uid{0};

            auto result = m_pPlayer->ChangeGold(m_pPlayer->GetGold() - nTotalPrice);
            if (result != TS_RESULT_SUCCESS) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), result, 0);
                return;
            }

            if (bJoinable) {
                auto item = Item::AllocItem(0, mt.code, buy_count, BY_MARKET, -1, -1, -1, 0, 0, 0, 0, 0);
                if (item == nullptr) {
                    NG_LOG_ERROR("entities.item", "ItemID Invalid! %d", mt.code);
                    return;
                }
                pNewItem = m_pPlayer->PushItem(item, buy_count, false);

                if (pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
                    Item::PendFreeItem(item);
            }
            else {
                for (int32_t i = 0; i < buy_count; i++) {
                    auto item = Item::AllocItem(0, mt.code, 1, BY_MARKET, -1, -1, -1, 0, 0, 0, 0, 0);
                    if (item == nullptr) {
                        NG_LOG_ERROR("entities.item", "ItemID Invalid! %d", mt.code);
                        return;
                    }
                    pNewItem = m_pPlayer->PushItem(item, buy_count, false);
                    if (pNewItem != nullptr && pNewItem->GetHandle() != item->GetHandle())
                        Item::PendFreeItem(item);
                }
            }
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, pRecvPct->item_code);
            TS_SC_NPC_TRADE_INFO resultPct{};
            resultPct.is_sell = false;
            resultPct.code = pRecvPct->item_code;
            resultPct.count = buy_count;
            resultPct.price = nTotalPrice;
            resultPct.huntaholic_point = mt.huntaholic_ratio;
            resultPct.target = m_pPlayer->GetLastContactLong("npc");
            SendPacket(resultPct);
        }
    }
}

void WorldSession::onDeleteCharacter(const TS_CS_DELETE_CHARACTER *pRecvPct)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_CHARACTER);
    stmt->setString(0, pRecvPct->name);
    stmt->setInt32(1, _accountId);
    CharacterDatabase.Execute(stmt);
    // Send result message with WorldSession, player is not set yet
    Messages::SendResult(this, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onChangeLocation(const TS_CS_CHANGE_LOCATION *pRecvPct)
{
    m_pPlayer->ChangeLocation(pRecvPct->x, pRecvPct->y, true, true);
}

void WorldSession::onTimeSync(const TS_TIMESYNC *pRecvPct)
{
    uint32_t ct = sWorld.GetArTime();
    m_pPlayer->m_TS.onEcho(ct - pRecvPct->time);
    if (m_pPlayer->m_TS.m_vT.size() >= 4) {
        TS_SC_SET_TIME timePct{};
        timePct.gap = m_pPlayer->m_TS.GetInterval();
        SendPacket(timePct);
    }
    else {
        Messages::SendTimeSynch(m_pPlayer);
    }
}

void WorldSession::onGameTime(const TS_CS_GAME_TIME * /*pRecvPct*/)
{
    Messages::SendGameTime(m_pPlayer);
}

void WorldSession::onQuery(const TS_CS_QUERY *pRecvPct)
{
    auto obj = sMemoryPool.GetObjectInWorld<WorldObject>(pRecvPct->handle);
    if (obj != nullptr && obj->IsInWorld() && obj->GetLayer() == m_pPlayer->GetLayer() && sRegion.IsVisibleRegion(obj, m_pPlayer) != 0) {
        Messages::sendEnterMessage(m_pPlayer, obj, false);
    }
}

void WorldSession::onUpdate(const TS_CS_UPDATE *pRecvPct)
{
    auto unit = dynamic_cast<Unit *>(m_pPlayer);
    if (pRecvPct->handle != m_pPlayer->GetHandle()) {
        // Do Summon stuff here
    }
    if (unit != nullptr) {
        unit->OnUpdate();
        return;
    }
}

void WorldSession::onJobLevelUp(const TS_CS_JOB_LEVEL_UP *pRecvPct)
{
    auto cr = m_pPlayer->As<Unit>();
    if (cr == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->target);
        return;
    }
    if (cr->IsPlayer() && cr->GetHandle() != m_pPlayer->GetHandle()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->target);
        return;
    }
    int32_t jp = sObjectMgr.GetNeedJpForJobLevelUp(cr->GetCurrentJLv(), m_pPlayer->GetJobDepth());
    if (cr->GetJP() < jp) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ENOUGH_JP, pRecvPct->target);
        return;
    }
    if (jp == 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_LIMIT_MAX, pRecvPct->target);
        return;
    }
    cr->SetJP(cr->GetJP() - jp);
    cr->SetCurrentJLv(cr->GetCurrentJLv() + 1);
    cr->CalculateStat();
    if (cr->IsPlayer()) {
        dynamic_cast<Player *>(cr)->Save(true);
    }
    else {
        // Summon
    }

    m_pPlayer->Save(true);
    Messages::SendPropertyMessage(m_pPlayer, cr, "job_level", cr->GetCurrentJLv());
    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, pRecvPct->target);
}

void WorldSession::onLearnSkill(const TS_CS_LEARN_SKILL *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto target = m_pPlayer->As<Unit>();
    uint16_t result{};
    int32_t jobID{};
    int32_t value{};

    if (m_pPlayer->GetHandle() != pRecvPct->target) {
        auto summon = sMemoryPool.GetObjectInWorld<Summon>(pRecvPct->target);
        if (summon == nullptr || !summon->IsSummon() || summon->GetMaster() == nullptr || summon->GetMaster()->GetHandle() != m_pPlayer->GetHandle()) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, 0);
            return;
        }
        target = summon;
    }
    int32_t currentLevel = target->GetBaseSkillLevel(pRecvPct->skill_id) + 1;
    // if(skill_level == currentLevel)
    //{
    result = GameContent::IsLearnableSkill(target, pRecvPct->skill_id, currentLevel, jobID);
    if (result == TS_RESULT_SUCCESS) {
        target->RegisterSkill(pRecvPct->skill_id, currentLevel, 0, jobID);
        m_pPlayer->Save(true);
    }
    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), result, value);
    //}
}

void WorldSession::onEquipSummon(const TS_EQUIP_SUMMON *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (false /*IsItemUseable()*/)
        return;

    int32_t nCFL = m_pPlayer->GetCurrentSkillLevel(SKILL_CREATURE_CONTROL);
    if (nCFL < 0)
        return;

    if (nCFL > 6)
        nCFL = 6;

    Item *pItem = nullptr;
    Summon *summon = nullptr;
    for (int32_t i = 0; i < nCFL; ++i) {
        bool bFound = false;
        pItem = nullptr;
        if (pRecvPct->card_handle[i] != 0) {
            pItem = m_pPlayer->FindItemByHandle(pRecvPct->card_handle[i]);
            if (pItem != nullptr && pItem->GetItemTemplate() != nullptr) {
                if (pItem->GetItemGroup() != ItemGroup::GROUP_SUMMONCARD || m_pPlayer->GetHandle() != pItem->GetItemInstance().GetOwnerHandle() ||
                    (pItem->GetItemInstance().GetFlag() & (uint32_t)FlagBits::ITEM_FLAG_SUMMON) == 0)
                    continue;
            }
        }
        for (int32_t j = 0; j < nCFL; j++) {
            if (pItem != nullptr) {
                // Belt Slot Card
            }
        }
        if (bFound)
            continue;

        if (m_pPlayer->m_aBindSummonCard[i] != nullptr) {
            if (pItem == nullptr || m_pPlayer->m_aBindSummonCard[i]->m_nHandle != pItem->m_nHandle) {
                summon = m_pPlayer->m_aBindSummonCard[i]->m_pSummon;
                if (pRecvPct->card_handle[i] == 0)
                    m_pPlayer->m_aBindSummonCard[i] = nullptr;
                if (summon != nullptr && !summon->IsInWorld()) {
                    for (int32_t k = 0; k < 24; ++k) {
                        if (summon->GetWornItem((ItemWearType)k) != nullptr)
                            summon->Putoff((ItemWearType)k);
                    }
                }
            }
        }

        if (pItem != nullptr) {
            if ((pItem->GetItemInstance().GetFlag() & ITEM_FLAG_SUMMON) != 0) {
                summon = pItem->m_pSummon;
                if (summon == nullptr) {
                    summon = sMemoryPool.AllocNewSummon(m_pPlayer, pItem);
                    summon->SetFlag(UNIT_FIELD_STATUS, STATUS_LOGIN_COMPLETE);
                    m_pPlayer->AddSummon(summon, true);
                    Messages::SendItemMessage(m_pPlayer, pItem);

                    Summon::DB_InsertSummon(m_pPlayer, summon);
                    sScriptingMgr.RunString(m_pPlayer, NGemity::StringFormat("on_first_summon( {}, {})", summon->GetSummonCode(), summon->GetHandle()));
                    summon->SetCurrentJLv(summon->GetLevel());
                    summon->CalculateStat();
                }
                summon->m_cSlotIdx = (uint8_t)i;
                summon->CalculateStat();
            }
            m_pPlayer->m_aBindSummonCard[i] = pItem;
        }
    }
    if (nCFL > 1) {
        for (int32_t i = 0; i < nCFL; ++i) {
            if (m_pPlayer->m_aBindSummonCard[i] == nullptr) {
                for (int32_t x = i + 1; x < nCFL; ++x) {
                    if (m_pPlayer->m_aBindSummonCard[x] != nullptr) {
                        m_pPlayer->m_aBindSummonCard[i] = m_pPlayer->m_aBindSummonCard[x];
                        m_pPlayer->m_aBindSummonCard[x] = nullptr;
                    }
                }
            }
        }
    }
    Messages::SendCreatureEquipMessage(m_pPlayer, pRecvPct->open_dialog);
}

void WorldSession::onSellItem(const TS_CS_SELL_ITEM *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto item = m_pPlayer->FindItemByHandle(pRecvPct->handle);
    if (item == nullptr || item->GetItemTemplate() == nullptr || item->GetItemInstance().GetOwnerHandle() != m_pPlayer->GetHandle() || !item->IsInInventory()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, 0);
        return;
    }
    if (pRecvPct->sell_count == 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_UNKNOWN, 0);
        return;
    }
    // if(!m_pPlayer.IsSelllable) @todo

    auto nPrice = GameContent::GetItemSellPrice(
        item->GetItemTemplate()->price, item->GetItemTemplate()->rank, item->GetItemInstance().GetLevel(), item->GetItemInstance().GetCode() >= 602700 && item->GetItemInstance().GetCode() <= 602799);
    auto nResultCount = item->GetItemInstance().GetCount() - pRecvPct->sell_count;
    auto nEnhanceLevel = (item->GetItemInstance().GetLevel() + 100 * item->GetItemInstance().GetEnhance());

    if (!m_pPlayer->IsSellable(item) || nResultCount < 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, item->GetItemInstance().GetCode());
        return;
    }
    if (m_pPlayer->GetGold() + pRecvPct->sell_count * nPrice > MAX_GOLD_FOR_INVENTORY) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_MUCH_MONEY, item->GetItemInstance().GetCode());
        return;
    }
    if (m_pPlayer->GetGold() + pRecvPct->sell_count * nPrice < 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, item->GetItemInstance().GetCode());
        return;
    }
    auto code = item->GetItemCode();
    if (!m_pPlayer->EraseItem(item, pRecvPct->sell_count)) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, item->GetHandle());
        return;
    }
    if (m_pPlayer->ChangeGold(m_pPlayer->GetGold() + pRecvPct->sell_count * nPrice) != 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_MUCH_MONEY, item->GetItemInstance().GetCode());
        return;
    }

    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, item->GetHandle());
    TS_SC_NPC_TRADE_INFO tradePct{};
    tradePct.is_sell = true;
    tradePct.code = code;
    tradePct.code = pRecvPct->sell_count;
    tradePct.price = pRecvPct->sell_count * nPrice;
    tradePct.target = m_pPlayer->GetLastContactLong("npc");
    m_pPlayer->SendPacket(tradePct);
}

void WorldSession::onSkill(const TS_CS_SKILL *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->IsDead())
        return;

    WorldObject *pTarget{nullptr};
    Position pos{};
    pos.Relocate(pRecvPct->x, pRecvPct->y, pRecvPct->z);

    auto pCaster = m_pPlayer->As<Unit>();
    if (pRecvPct->caster != m_pPlayer->GetHandle())
        pCaster = m_pPlayer->GetSummonByHandle(pRecvPct->caster);

    if (pCaster == nullptr || !pCaster->IsInWorld()) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_NOT_EXIST);
        return;
    }
    auto base = sObjectMgr.GetSkillBase(pRecvPct->skill_id);
    if (base == nullptr || !base->IsValid() || base->IsSystemSkill()) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_ACCESS_DENIED);
        return;
    }

    if ((base->IsPhysicalSkill() && !pCaster->IsSkillCastable()) || (!base->IsPhysicalSkill() && !pCaster->IsMagicCastable())) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_NOT_ACTABLE);
        return;
    }

    if (pRecvPct->target != 0) {
        pTarget = sMemoryPool.GetObjectInWorld<WorldObject>(pRecvPct->target);
        if (pTarget == nullptr) {
            Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_NOT_EXIST);
            return;
        }
    }

    auto ct = sWorld.GetArTime();
    if (pCaster->IsMoving(ct)) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_NOT_ACTABLE);
        return;
    }

    // Once again
    if ((base->IsPhysicalSkill() && !pCaster->IsSkillCastable()) || (!base->IsPhysicalSkill() && !pCaster->IsMagicCastable())) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_NOT_ACTABLE);
        return;
    }

    auto pSkill = pCaster->GetSkill(pRecvPct->skill_id);
    if (pSkill == nullptr || (pSkill != nullptr && pSkill->m_nSkillUID == -1))
        return;

    if (pSkill->GetCurrentSkillLevel() <= 0) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, TS_RESULT_ACCESS_DENIED);
        return;
    }

    int32_t nSkillLv = pRecvPct->skill_level;
    if (pRecvPct->skill_level <= 0 || pRecvPct->skill_level > pSkill->GetCurrentSkillLevel())
        nSkillLv = pSkill->GetCurrentSkillLevel();

    int32_t nResult = pCaster->CastSkill(pRecvPct->skill_id, nSkillLv, pRecvPct->target, pos, pCaster->GetLayer(), false);
    if (nResult != TS_RESULT_SUCCESS) {
        Messages::SendSkillCastFailMessage(m_pPlayer, pRecvPct->caster, pRecvPct->target, pRecvPct->skill_id, pRecvPct->skill_level, pos, nResult);
        return;
    }
}

void WorldSession::onSetProperty(const TS_CS_SET_PROPERTY *pRecvPct)
{
    if (pRecvPct->name != "client_info"s)
        return;
    std::string value = pRecvPct->string_value;
    m_pPlayer->SetClientInfo(value);
}

void WorldSession::KickPlayer()
{
    CloseSocket();
}

void WorldSession::onAttackRequest(const TS_CS_ATTACK_REQUEST *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->GetHealth() == 0)
        return;

    auto unit = dynamic_cast<Unit *>(m_pPlayer);
    if (pRecvPct->handle != m_pPlayer->GetHandle())
        unit = m_pPlayer->GetSummonByHandle(pRecvPct->handle);
    if (unit == nullptr) {
        Messages::SendCantAttackMessage(m_pPlayer, pRecvPct->handle, pRecvPct->target_handle, TS_RESULT_NOT_OWN);
        return;
    }

    if (pRecvPct->target_handle == 0) {
        if (unit->GetTargetHandle() != 0)
            unit->CancelAttack();
        return;
    }

    auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(pRecvPct->target_handle);
    if (pTarget == nullptr) {
        if (unit->GetTargetHandle() != 0) {
            unit->EndAttack();
            return;
        }
        Messages::SendCantAttackMessage(m_pPlayer, pRecvPct->handle, pRecvPct->target_handle, TS_RESULT_NOT_EXIST);
        return;
    }

    if (!unit->IsEnemy(pTarget, false)) {
        if (unit->GetTargetHandle() != 0) {
            unit->EndAttack();
            return;
        }
        Messages::SendCantAttackMessage(m_pPlayer, pRecvPct->getReceivedId(), pRecvPct->target_handle, 0);
        return;
    }

    if ((unit->IsUsingCrossBow() || unit->IsUsingBow()) && unit->IsPlayer() && unit->GetBulletCount() < 1) {
        Messages::SendCantAttackMessage(m_pPlayer, pRecvPct->getReceivedId(), pRecvPct->target_handle, 0);
        return;
    }

    unit->StartAttack(pRecvPct->target_handle, true);
}

void WorldSession::onCancelAction(const TS_CS_CANCEL_ACTION *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    Unit *cancellor = m_pPlayer->GetSummonByHandle(pRecvPct->handle);
    if (cancellor == nullptr || !cancellor->IsInWorld())
        cancellor = dynamic_cast<Unit *>(m_pPlayer);
    if (cancellor->GetHandle() == pRecvPct->handle) {
        if (cancellor->m_castingSkill != nullptr) {
            cancellor->CancelSkill();
        }
        else {
            if (cancellor->GetTargetHandle() != 0)
                cancellor->CancelAttack();
        }
    }
}

void WorldSession::onTakeItem(const TS_CS_TAKE_ITEM *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;
    uint32_t ct = sWorld.GetArTime();

    auto item = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
    if (item == nullptr || !item->IsInWorld()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->item_handle);
        return;
    }

    // TODO: Weight
    if (item->GetItemInstance().GetOwnerHandle() != 0) {
        NG_LOG_ERROR("entities.item", "WorldSession::onTakeItem(): OwnerHandle not null: %s, handle: %u", m_pPlayer->GetName(), item->GetHandle());
        return;
    }

    auto pos = m_pPlayer->GetPosition();
    if (GameRule::GetPickableRange() < item->GetExactDist2d(&pos)) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_FAR, pRecvPct->item_handle);
        return;
    }

    if (item->IsQuestItem() && !m_pPlayer->IsTakeableQuestItem(item->GetItemInstance().GetCode())) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
        return;
    }

    auto drop_duration = ct - item->m_nDropTime;
    uint32_t ry = 3000;

    for (int32_t i = 0; i < 3; i++) {
        if (item->m_pPickupOrder.hPlayer[i] == 0 && item->m_pPickupOrder.nPartyID[i] == 0)
            break;

        if (item->m_pPickupOrder.nPartyID[i] <= 0 || item->m_pPickupOrder.nPartyID[i] != m_pPlayer->GetPartyID()) {
            if (item->m_pPickupOrder.hPlayer[i] != m_pPlayer->GetHandle()) {
                if (drop_duration < ry) {
                    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
                    return;
                }
                ry += 1000;
            }
        }
    }

    if (item->GetItemInstance().GetCode() == 0) {
        if (m_pPlayer->GetPartyID() == 0) {
            if (m_pPlayer->GetGold() + item->GetItemInstance().GetCount() > MAX_GOLD_FOR_INVENTORY) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_MUCH_MONEY, pRecvPct->item_handle);
                return;
            }
        }
    }

    TS_SC_TAKE_ITEM_RESULT resultPct{};
    resultPct.item_handle = pRecvPct->item_handle;
    resultPct.item_taker = m_pPlayer->GetHandle();
    sWorld.Broadcast((uint32_t)(m_pPlayer->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint32_t)(m_pPlayer->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
        m_pPlayer->GetLayer(), resultPct);
    if (sWorld.RemoveItemFromWorld(item)) {
        if (m_pPlayer->GetPartyID() != 0) {
            if (item->GetItemInstance().GetCode() != 0) {
                ///- Actual Item
                sWorld.procPartyShare(m_pPlayer, item);
            }
            else {
                ///- Gold
                std::vector<Player *> vList{};
                sGroupManager.GetNearMember(m_pPlayer, 400.0f, vList);
                auto incGold = (int64_t)(item->GetItemInstance().GetCount() / (!vList.empty() ? vList.size() : 1));

                for (auto &np : vList) {
                    auto nNewGold = incGold + np->GetGold();
                    np->ChangeGold(nNewGold);
                }
                Item::PendFreeItem(item);
            }
            return;
        }
        uint32_t nih = sWorld.procAddItem(m_pPlayer, item, false);
        if (nih != 0) { // nih = new item handle
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, nih);
            return;
        }
        Item::PendFreeItem(item);
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
    }
}

void WorldSession::onUseItem(const TS_CS_USE_ITEM *pRecvPct)
{
    uint32_t ct = sWorld.GetArTime();

    auto item = m_pPlayer->FindItemByHandle(pRecvPct->item_handle);
    if (item == nullptr || item->GetItemInstance().GetOwnerHandle() != m_pPlayer->GetHandle()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->item_handle);
        return;
    }

    if (item->GetItemTemplate()->eType != ItemType::TYPE_USE && false /*!item->IsUsingItem()*/) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
        return;
    }

    if ((item->GetItemTemplate()->flaglist[ItemFlag::FLAG_MOVE] == 0 && m_pPlayer->IsMoving(ct))) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, pRecvPct->item_handle);
        return;
    }

    // IsUsableSecroute

    // IsUsableraid

    // Eventmap

    uint16_t nResult = m_pPlayer->IsUseableItem(item, nullptr);
    if (nResult != TS_RESULT_SUCCESS) {
        if (nResult == TS_RESULT_COOL_TIME)
            Messages::SendItemCoolTimeInfo(m_pPlayer);
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), nResult, pRecvPct->item_handle);
        return;
    }

    if (item->GetItemTemplate()->flaglist[FLAG_TARGET_USE] == 0) {
        nResult = m_pPlayer->UseItem(item, nullptr, pRecvPct->szParameter);
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), nResult, pRecvPct->item_handle);
        if (nResult != 0)
            return;
    }
    else {
        // auto unit = dynamic_cast<Unit*>(sMemoryPool.getPtrFromId(target_handle));
        auto unit = sMemoryPool.GetObjectInWorld<Unit>(pRecvPct->target_handle);
        if (unit == nullptr || unit->GetHandle() == 0) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->target_handle);
            return;
        }
        nResult = m_pPlayer->IsUseableItem(item, unit);
        if (nResult == TS_RESULT_SUCCESS) {
            nResult = m_pPlayer->UseItem(item, unit, pRecvPct->szParameter);
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), nResult, pRecvPct->item_handle);
        }

        if (nResult != TS_RESULT_SUCCESS) {
            return;
        }
    }

    TS_SC_USE_ITEM_RESULT resultPct{};
    resultPct.item_handle = pRecvPct->item_handle;
    resultPct.target_handle = pRecvPct->target_handle;
    m_pPlayer->SendPacket(resultPct);
}

bool WorldSession::Update(uint32_t /*diff*/)
{
    if (_accountId != 0 && (m_nLastPing > 0 && m_nLastPing + 30000 < sWorld.GetArTime())) {
        NG_LOG_DEBUG("server.worldserver", "Kicking Account [%d : %s] due to inactivity.", _accountId, _accountName.c_str());
        return false;
    }

    return true;
}

void WorldSession::onRevive(const TS_CS_RESURRECTION *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (pRecvPct->use_state) {
        Unit *pTarget = m_pPlayer;
        if (pRecvPct->handle != m_pPlayer->GetHandle())
            pTarget = m_pPlayer->GetSummonByHandle(pRecvPct->handle);
        if (pTarget == nullptr)
            return;

        auto nResult = TS_RESULT_SUCCESS;
        if (!pTarget->IsInWorld() || !pTarget->IsDead())
            nResult = TS_RESULT_NOT_ACTABLE;
        if (!pTarget->ResurrectByState())
            nResult = TS_RESULT_NOT_ACTABLE;

        Messages::SendResult(this, NGemity::Packets::TS_CS_RESURRECTION, nResult, 0);
        return;
    }
    else if (pRecvPct->use_potion) {
    }

    if (!m_pPlayer->IsDead())
        return;

    enum _REVIVE_TYPE {
        REVIVE_NORMAL = 0,
        REVIVE_BATTLE = 1,
        REVIVE_COMPETE = 2,
        REVIVE_DUNGEON_SIEGE = 3,
    };

    _REVIVE_TYPE eReviveType = REVIVE_NORMAL;
    if (m_pPlayer->IsInSiegeDungeon())
        eReviveType = REVIVE_DUNGEON_SIEGE;

    sScriptingMgr.RunString(m_pPlayer, NGemity::StringFormat("revive_in_town({})", static_cast<int32_t>(eReviveType)));
    m_pPlayer->ClearRemovedStateByDeath();
}

void WorldSession::onDropItem(const TS_CS_DROP_ITEM *pRecvPct)
{
    auto item = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
    if (item != nullptr && item->IsDropable() && pRecvPct->count > 0 && (item->GetItemGroup() != ItemGroup::GROUP_SUMMONCARD || !(item->GetItemInstance().GetFlag() & FlagBits::ITEM_FLAG_SUMMON))) {
        m_pPlayer->DropItem(m_pPlayer, item, pRecvPct->count);
        Messages::SendDropResult(m_pPlayer, pRecvPct->item_handle, true);
    }
    else {
        Messages::SendDropResult(m_pPlayer, pRecvPct->item_handle, false);
    }
}

void WorldSession::onMixRequest(const TS_CS_MIX *pRecvPct)
{
    if (pRecvPct->sub_items.size() > 9) {
        KickPlayer();
        return;
    }

    auto pMainItem = sMixManager.check_mixable_item(m_pPlayer, pRecvPct->main_item.handle, 1);
    if (pRecvPct->main_item.handle != 0 && pMainItem == nullptr)
        return;

    std::vector<Item *> pSubItem{};
    std::vector<uint16_t> pCountList{};
    if (pRecvPct->sub_items.size() != 0) {
        for (auto &mixInfo : pRecvPct->sub_items) {
            auto item = sMixManager.check_mixable_item(m_pPlayer, mixInfo.handle, mixInfo.count);
            if (item == nullptr)
                return;
            pSubItem.emplace_back(item);
            pCountList.emplace_back(mixInfo.count);
        }
    }

    auto mb = sMixManager.GetProperMixInfo(pMainItem, pRecvPct->sub_items.size(), pSubItem, pCountList);
    if (mb == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_INVALID_ARGUMENT, 0);
        return;
    }

    switch (mb->type) {
    case 0:
        break;
    case MIX_ENHANCE: // EnchantItem without E-Protect Powder
        sMixManager.EnhanceItem(mb, m_pPlayer, pMainItem, pRecvPct->sub_items.size(), pSubItem, pCountList);
        return;
    case MIX_ENHANCE_SKILL_CARD:
        sMixManager.EnhanceSkillCard(mb, m_pPlayer, pRecvPct->sub_items.size(), pSubItem);
        return;
    case MIX_ENHANCE_WITHOUT_FAIL: // EnchantItem WITH E-Protect Powder
        sMixManager.EnhanceItem(mb, m_pPlayer, pMainItem, pRecvPct->sub_items.size(), pSubItem, pCountList);
        return;
    case MIX_ADD_LEVEL_SET_FLAG:
        sMixManager.MixItem(mb, m_pPlayer, pMainItem, pRecvPct->sub_items.size(), pSubItem, pCountList);
        return;
    case MIX_RESTORE_ENHANCE_SET_FLAG:
        sMixManager.RepairItem(m_pPlayer, pMainItem, pRecvPct->sub_items.size(), pSubItem, pCountList);
        return;
    default:
        break;
    }
}

void WorldSession::onSoulStoneCraft(const TS_CS_SOULSTONE_CRAFT *pRecvPct)
{
    if (m_pPlayer->GetLastContactLong("SoulStoneCraft") == 0)
        return;

    auto nPrevGold = m_pPlayer->GetGold();
    auto pItem = m_pPlayer->FindItemByHandle(pRecvPct->craft_item_handle);
    if (pItem == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->craft_item_handle);
        return;
    }

    int32_t nSocketCount = pItem->GetItemTemplate()->socket;
    if (nSocketCount < 1 || nSocketCount > 4) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->craft_item_handle);
        return;
    }

    int32_t nMaxReplicatableCount = nSocketCount == 4 ? 2 : 1;
    int32_t nCraftCost = 0;
    bool bIsValid = false;
    Item *pSoulStoneList[sizeof(pRecvPct->soulstone_handle)]{nullptr};

    for (int32_t i = 0; i < nSocketCount; ++i) {
        if (pRecvPct->soulstone_handle[i] != 0) {
            pSoulStoneList[i] = m_pPlayer->FindItemByHandle(pRecvPct->soulstone_handle[i]);
            if (pSoulStoneList[i] == nullptr) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->soulstone_handle[i]);
                return;
            }
            if (pSoulStoneList[i]->GetItemTemplate()->eType != ItemType::TYPE_SOULSTONE || pSoulStoneList[i]->GetItemGroup() != ItemGroup::GROUP_SOULSTONE ||
                pSoulStoneList[i]->GetItemTemplate()->eClass != ItemClass::CLASS_SOULSTONE) {
                Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, pRecvPct->soulstone_handle[i]);
                return;
            }

            int32_t nReplicatedCount = 0;
            for (int32_t k = 0; k < pItem->GetItemTemplate()->socket; ++k) {
                if (pItem->GetItemInstance().GetSocketIndex(k) != 0 && k != i) {
                    auto ibs = sObjectMgr.GetItemBase(pItem->GetItemInstance().GetSocketIndex(k));
                    if (ibs->base_type[0] == pSoulStoneList[i]->GetItemTemplate()->base_type[0] && ibs->base_type[1] == pSoulStoneList[i]->GetItemTemplate()->base_type[1] &&
                        ibs->base_type[2] == pSoulStoneList[i]->GetItemTemplate()->base_type[2] && ibs->base_type[3] == pSoulStoneList[i]->GetItemTemplate()->base_type[3] &&
                        ibs->base_var[0][0] == pSoulStoneList[i]->GetItemTemplate()->base_var[0][0] && ibs->base_var[1][0] == pSoulStoneList[i]->GetItemTemplate()->base_var[1][0] &&
                        ibs->base_var[2][0] == pSoulStoneList[i]->GetItemTemplate()->base_var[2][0] && ibs->base_var[3][0] == pSoulStoneList[i]->GetItemTemplate()->base_var[3][0] &&
                        ibs->opt_type[0] == pSoulStoneList[i]->GetItemTemplate()->opt_type[0] && ibs->opt_type[1] == pSoulStoneList[i]->GetItemTemplate()->opt_type[1] &&
                        ibs->opt_type[2] == pSoulStoneList[i]->GetItemTemplate()->opt_type[2] && ibs->opt_type[3] == pSoulStoneList[i]->GetItemTemplate()->opt_type[3] &&
                        ibs->opt_var[0][0] == pSoulStoneList[i]->GetItemTemplate()->opt_var[0][0] && ibs->opt_var[1][0] == pSoulStoneList[i]->GetItemTemplate()->opt_var[1][0] &&
                        ibs->opt_var[2][0] == pSoulStoneList[i]->GetItemTemplate()->opt_var[2][0] && ibs->opt_var[3][0] == pSoulStoneList[i]->GetItemTemplate()->opt_var[3][0]) {
                        nReplicatedCount++;
                        if (nReplicatedCount >= nMaxReplicatableCount) {
                            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ALREADY_EXIST, 0);
                            return;
                        }
                    }
                }
            }
            nCraftCost += pSoulStoneList[i]->GetItemTemplate()->price / 10;
            bIsValid = true;
        }
    }
    if (!bIsValid) {
        // maybe log here?
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_INVALID_ARGUMENT, 0);
        return;
    }
    if (nPrevGold < nCraftCost || m_pPlayer->ChangeGold(nPrevGold - nCraftCost) != TS_RESULT_SUCCESS) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ENOUGH_MONEY, 0);
        return;
    }

    int32_t nEndurance = 0;
    for (int32_t i = 0; i < 4; ++i) {
        if (pItem->GetItemInstance().GetSocketIndex(i) != 0 || pSoulStoneList[i] != nullptr) {
            if (pSoulStoneList[i] != nullptr) {
                nEndurance += pSoulStoneList[i]->GetItemInstance().GetCurrentEndurance();
                pItem->GetItemInstance().SetSocketIndex(i, pSoulStoneList[i]->GetItemInstance().GetCode());
                m_pPlayer->EraseItem(pSoulStoneList[i], 1);
            }
            else {
                nEndurance += pItem->GetItemInstance().GetCurrentEndurance();
            }
        }
    }

    pItem->SetCurrentEndurance(nEndurance);
    m_pPlayer->Save(false);
    pItem->DBUpdate();
    m_pPlayer->SetLastContact("SoulStoneCraft", 0);
    Messages::SendItemMessage(m_pPlayer, pItem);
    m_pPlayer->CalculateStat();
    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
}

void WorldSession::onStorage(const TS_CS_STORAGE *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (!m_pPlayer->m_bIsUsingStorage || m_pPlayer->m_castingSkill != nullptr || m_pPlayer->GetUInt32Value(PLAYER_FIELD_TRADE_TARGET) != 0 || !m_pPlayer->IsActable()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, pRecvPct->item_handle);
        return;
    }

    switch ((STORAGE_MODE)pRecvPct->mode) {
    case ITEM_INVENTORY_TO_STORAGE:
    case ITEM_STORAGE_TO_INVENTORY: {
        if (pRecvPct->count <= 0) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ENOUGH_MONEY, pRecvPct->item_handle);
            return;
        }

        auto *pItem = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
        if (pItem == nullptr) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->item_handle);
            return;
        }
        if (pItem->GetOwnerHandle() != m_pPlayer->GetHandle()) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
            return;
        }

        if (pItem->IsInInventory() && pRecvPct->mode == ITEM_INVENTORY_TO_STORAGE) {
            if (pItem->GetItemBase()->flaglist[ITEM_FLAG_EVENT])
                if (pItem->m_pSummon != nullptr) {
                    for (const auto &v : m_pPlayer->m_aBindSummonCard) {
                        if (v == pItem) {
                            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pItem->GetHandle());
                            return;
                        }
                    }
                }
            /*if((pItem->m_Instance.Flag & 0x40) == 0 || m_pPlayer->FindStorageItem(pItem->m_Instance.Code) == nullptr)
                {
                    Messages::SendResult(m_pPlayer, pRecvPct->GetPacketID(), TS_RESULT_TOO_HEAVY, handle); // too heavy??
                    return;
                }*/
            m_pPlayer->MoveInventoryToStorage(pItem, pRecvPct->count);
        }
        else if (pItem->IsInStorage() && pRecvPct->mode == ITEM_STORAGE_TO_INVENTORY) {
            m_pPlayer->MoveStorageToInventory(pItem, pRecvPct->count);
        }
        m_pPlayer->Save(true);
        return;
    }
    case GOLD_INVENTORY_TO_STORAGE: // 2
    {
        if (m_pPlayer->GetGold() < pRecvPct->count)
            return;
        if (m_pPlayer->GetStorageGold() + pRecvPct->count > MAX_GOLD_FOR_STORAGE) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_MUCH_MONEY, pRecvPct->item_handle);
            return;
        }
        auto nGold = m_pPlayer->GetGold();
        if (m_pPlayer->ChangeGold(nGold - pRecvPct->count) == TS_RESULT_SUCCESS && m_pPlayer->ChangeStorageGold(m_pPlayer->GetStorageGold() + pRecvPct->count) == TS_RESULT_SUCCESS) {
            m_pPlayer->Save(true);
            return;
        }
    }
        return;
    case GOLD_STORAGE_TO_INVENTORY: {
        if (m_pPlayer->GetStorageGold() < pRecvPct->count)
            return;
        if (m_pPlayer->GetGold() + pRecvPct->count > MAX_GOLD_FOR_INVENTORY) {
            Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_TOO_MUCH_MONEY, pRecvPct->item_handle);
            return;
        }
        auto nGold = m_pPlayer->GetStorageGold();
        if (m_pPlayer->ChangeStorageGold(nGold - pRecvPct->count) == TS_RESULT_SUCCESS && m_pPlayer->ChangeGold(m_pPlayer->GetGold() + pRecvPct->count) == TS_RESULT_SUCCESS) {
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

void WorldSession::onBindSkillCard(const TS_CS_BIND_SKILLCARD *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    auto pItem = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
    if (pItem == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->item_handle);
        return;
    }
    if (pRecvPct->target_handle != m_pPlayer->GetHandle()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, pRecvPct->target_handle);
        return;
    }
    if (!pItem->IsInInventory() || pItem->GetItemInstance().GetOwnerHandle() != m_pPlayer->GetHandle() || pItem->GetItemGroup() != ItemGroup::GROUP_SKILLCARD || pItem->m_hBindedTarget != 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
        return;
    }

    auto pSkill = m_pPlayer->GetSkill(pItem->GetItemTemplate()->skill_id);
    if (pSkill != nullptr && pSkill->GetSkillEnhance() == 0) {
        m_pPlayer->BindSkillCard(pItem);
    }
}

void WorldSession::onUnBindSkilLCard(const TS_CS_UNBIND_SKILLCARD *pRecvPct)
{
    auto pItem = sMemoryPool.GetObjectInWorld<Item>(pRecvPct->item_handle);
    if (pItem == nullptr) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_EXIST, pRecvPct->item_handle);
        return;
    }
    if (pRecvPct->target_handle != m_pPlayer->GetHandle()) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, pRecvPct->target_handle);
        return;
    }
    if (!pItem->IsInInventory() || pItem->GetItemInstance().GetOwnerHandle() != m_pPlayer->GetHandle() || pItem->GetItemGroup() != ItemGroup::GROUP_SKILLCARD || pItem->m_hBindedTarget == 0) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_ACCESS_DENIED, pRecvPct->item_handle);
        return;
    }

    m_pPlayer->UnBindSkillCard(pItem);
}

void WorldSession::onTrade(const TS_TRADE *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->m_bIsUsingStorage)
        return;

    if (m_pPlayer->bIsMoving && m_pPlayer->IsInWorld()) {
        m_pPlayer->CancelTrade(false);
        return;
    }

    if (pRecvPct->target_player == m_pPlayer->GetHandle()) {
        if (m_pPlayer->GetTargetHandle() != 0) {
            auto pTarget = sMemoryPool.GetObjectInWorld<Player>(m_pPlayer->GetTargetHandle());
            pTarget->CancelTrade(false);
        }
        m_pPlayer->CancelTrade(false);
        return;
    }

    switch (pRecvPct->mode) {
    case TM_REQUEST_TRADE:
        onRequestTrade(pRecvPct->target_player);
        break;
    case TM_ACCEPT_TRADE:
        onAcceptTrade(pRecvPct->target_player);
        break;
    case TM_CANCEL_TRADE:
        onCancelTrade();
        break;
    case TM_REJECT_TRADE:
        onRejectTrade(pRecvPct->target_player);
        break;
    case TM_ADD_ITEM:
        onAddItem(pRecvPct->target_player, pRecvPct);
        break;
    case TM_REMOVE_ITEM:
        onRemoveItem(pRecvPct->target_player, pRecvPct);
        break;
    case TM_ADD_GOLD:
        onAddGold(pRecvPct->target_player, pRecvPct);
        break;
    case TM_FREEZE_TRADE:
        onFreezeTrade();
        break;
    case TM_CONFIRM_TRADE:
        onConfirmTrade(pRecvPct->target_player);
        break;
    default:
        return;
    }
}

void WorldSession::onRequestTrade(uint32_t hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (tradeTarget->m_bTrading || m_pPlayer->m_bTrading)
        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, tradeTarget->GetHandle());
    else if (!m_pPlayer->IsTradableWith(tradeTarget))
        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_PK_LIMIT, tradeTarget->GetHandle());
    else {
        TS_TRADE tradePct{};
        tradePct.target_player = m_pPlayer->GetHandle();
        tradePct.mode = static_cast<uint8_t>(TM_REQUEST_TRADE);
        tradeTarget->SendPacket(tradePct);
    }
}

void WorldSession::onAcceptTrade(uint32_t hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (m_pPlayer->m_bTrading || tradeTarget->m_bTrading || m_pPlayer->m_hTamingTarget || tradeTarget->m_hTamingTarget) {
        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, 0);
    }
    else {
        m_pPlayer->StartTrade(tradeTarget->GetHandle());
        tradeTarget->StartTrade(m_pPlayer->GetHandle());

        {
            TS_TRADE tradePct{};
            tradePct.target_player = tradeTarget->GetHandle();
            tradePct.mode = static_cast<uint8_t>(TM_BEGIN_TRADE);
            m_pPlayer->SendPacket(tradePct);
        }

        {
            TS_TRADE tradePct{};
            tradePct.target_player = m_pPlayer->GetHandle();
            tradePct.mode = static_cast<uint8_t>(TM_BEGIN_TRADE);
            tradeTarget->SendPacket(tradePct);
        }
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

void WorldSession::onRejectTrade(uint32_t hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    TS_TRADE tradePct{};
    tradePct.target_player = m_pPlayer->GetHandle();
    tradePct.mode = static_cast<uint8_t>(TM_REJECT_TRADE);
    tradeTarget->SendPacket(tradePct);
}

void WorldSession::onAddItem(uint32_t hTradeTarget, const TS_TRADE *pRecvPct)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (!m_pPlayer->m_bTradeFreezed) {
        auto item = m_pPlayer->FindItemByHandle(pRecvPct->item_info.base_info.handle);
        auto count = pRecvPct->item_info.base_info.count;

        if (item == nullptr || item->GetItemTemplate() == nullptr)
            return;

        if (count <= 0 || count > item->GetItemInstance().GetCount()) {
            NG_LOG_ERROR("server.trade", "Add Trade Bug [%s:%d]", m_pPlayer->m_szAccount.c_str(), m_pPlayer->GetHandle());
            // Register block account in game rule?
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
            return;
        }

        if (!m_pPlayer->IsTradable(item)) {
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_ACCESS_DENIED, 0);
            return;
        }

        if (m_pPlayer->AddItemToTradeWindow(item, count)) {
            Messages::SendTradeItemInfo(TM_ADD_ITEM, item, count, m_pPlayer, tradeTarget);
        }
    }
}

void WorldSession::onRemoveItem(uint32_t hTradeTarget, const TS_TRADE *pRecvPct)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget)) {
        m_pPlayer->CancelTrade(false);
        return;
    }

    if (!m_pPlayer->m_bTradeFreezed) {
        auto item = m_pPlayer->FindItemByHandle(pRecvPct->item_info.base_info.handle);
        auto count = pRecvPct->item_info.base_info.count;

        if (item == nullptr || item->GetItemTemplate() == nullptr)
            return;

        if (m_pPlayer->RemoveItemFromTradeWindow(item, count)) {
            Messages::SendTradeItemInfo(TM_REMOVE_ITEM, item, count, m_pPlayer, tradeTarget);
        }
        else {
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
        }
    }
}

void WorldSession::onAddGold(uint32_t hTradeTarget, const TS_TRADE *pRecvPct)
{
    if (!m_pPlayer->m_bTradeFreezed) {
        auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
        if (!isValidTradeTarget(tradeTarget))
            return;

        int64_t gold = pRecvPct->item_info.base_info.count;
        if (gold <= 0) {
            NG_LOG_ERROR("server.trade", "Add gold Trade Bug [%s:%d]", m_pPlayer->m_szAccount.c_str(), m_pPlayer->GetHandle());
            // Register block account in game rule?
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_ResultCode::TS_RESULT_NOT_EXIST, 0);
            return;
        }

        m_pPlayer->AddGoldToTradeWindow(gold);

        TS_TRADE tradePct{};
        tradePct.target_player = m_pPlayer->GetHandle();
        tradePct.mode = static_cast<uint8_t>(TM_ADD_GOLD);
        tradePct.item_info.base_info.count = gold;
        tradeTarget->SendPacket(tradePct);
        m_pPlayer->SendPacket(tradePct);
    }
    else {
        m_pPlayer->CancelTrade(false);
    }
}

void WorldSession::onFreezeTrade()
{
    auto tradeTarget = m_pPlayer->GetTradeTarget();
    if (tradeTarget != nullptr) {
        m_pPlayer->FreezeTrade();

        TS_TRADE tradePct{};
        tradePct.target_player = m_pPlayer->GetHandle();
        tradePct.mode = static_cast<uint8_t>(TM_FREEZE_TRADE);
        tradeTarget->SendPacket(tradePct);
        m_pPlayer->SendPacket(tradePct);
    }
    else
        m_pPlayer->CancelTrade(false);
}

void WorldSession::onConfirmTrade(uint32_t hTradeTarget)
{
    auto tradeTarget = sMemoryPool.GetObjectInWorld<Player>(hTradeTarget);
    if (!isValidTradeTarget(tradeTarget))
        return;

    if (!m_pPlayer->m_bTrading || !tradeTarget->m_bTrading || !m_pPlayer->m_bTradeFreezed || !tradeTarget->m_bTradeFreezed || tradeTarget->GetTradeTarget() != m_pPlayer) {
        m_pPlayer->CancelTrade(true);
        tradeTarget->CancelTrade(true);
        return;
    }

    if (m_pPlayer->m_bTradeAccepted)
        return;

    m_pPlayer->ConfirmTrade();

    TS_TRADE tradePct{};
    tradePct.target_player = m_pPlayer->GetHandle();
    tradePct.mode = static_cast<uint8_t>(TM_CONFIRM_TRADE);
    tradeTarget->SendPacket(tradePct);
    m_pPlayer->SendPacket(tradePct);

    if (!tradeTarget->m_bTradeAccepted)
        return;

    if (!m_pPlayer->CheckTradeWeight() || !tradeTarget->CheckTradeWeight()) {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_HEAVY, 0);
        Messages::SendResult(tradeTarget, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_HEAVY, 0);

        return;
    }

    if (!m_pPlayer->CheckTradeItem() || !tradeTarget->CheckTradeItem()) {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_RESULT_ACCESS_DENIED, 0);
        Messages::SendResult(tradeTarget, NGemity::Packets::TS_TRADE, TS_RESULT_ACCESS_DENIED, 0);

        return;
    }

    int64_t tradeGold = m_pPlayer->GetGold() + tradeTarget->GetTradeGold();
    int64_t tradeTargetGold = tradeTarget->GetGold() + m_pPlayer->GetTradeGold();

    bool bExceedGold = tradeGold > MAX_GOLD_FOR_INVENTORY;
    bool bExceedGoldTradeTarget = tradeTargetGold > MAX_GOLD_FOR_INVENTORY;

    if (bExceedGold || bExceedGoldTradeTarget) {
        m_pPlayer->CancelTrade(false);
        tradeTarget->CancelTrade(false);

        if (bExceedGold) {
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, m_pPlayer->GetHandle());
            Messages::SendResult(tradeTarget, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, m_pPlayer->GetHandle());
        }

        if (bExceedGoldTradeTarget) {
            Messages::SendResult(m_pPlayer, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, tradeTarget->GetHandle());
            Messages::SendResult(tradeTarget, NGemity::Packets::TS_TRADE, TS_RESULT_TOO_MUCH_MONEY, tradeTarget->GetHandle());
        }
    }
    else {
        if (m_pPlayer->m_bTrading && tradeTarget->m_bTrading && m_pPlayer->m_bTradeFreezed && tradeTarget->m_bTradeFreezed && m_pPlayer->GetTradeTarget() == tradeTarget &&
            tradeTarget->GetTradeTarget() == m_pPlayer && m_pPlayer->ProcessTrade()) {
            TS_TRADE tradePct{};
            tradePct.target_player = m_pPlayer->GetHandle();
            tradePct.mode = static_cast<uint8_t>(TM_PROCESS_TRADE);
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

void WorldSession::onDropQuest(const TS_CS_DROP_QUEST *pRecvPct)
{
    if (m_pPlayer == nullptr)
        return;

    if (m_pPlayer->DropQuest(pRecvPct->code)) {
        Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_SUCCESS, 0);
        return;
    }
    Messages::SendResult(m_pPlayer, pRecvPct->getReceivedId(), TS_RESULT_NOT_ACTABLE, 0);
}

void WorldSession::onPing(const TS_CS_PING *)
{
    m_nLastPing = sWorld.GetArTime();
}
