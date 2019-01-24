#pragma once
/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "Log.h"
#include "XSocket.h"
#include "Encryption/XRc4Cipher.h"

enum STORAGE_MODE : int
{
  ITEM_INVENTORY_TO_STORAGE = 0x0,
  ITEM_STORAGE_TO_INVENTORY = 0x1,
  GOLD_INVENTORY_TO_STORAGE = 0x2,
  GOLD_STORAGE_TO_INVENTORY = 0x3,
  STORAGE_CLOSE = 0x4,
};

class Player;

// Handle the player network
class WorldSession : public XSocket
{
public:
  friend class Player;
  friend class World;
  explicit WorldSession(boost::asio::ip::tcp::socket &&socket);
  WorldSession(const WorldSession &) = delete;
  virtual ~WorldSession();

  // Accept & Close handler
  //void OnAccept() override;
  void OnClose() override;
  void KickPlayer();
  bool Update(uint diff);

  ReadDataHandlerResult ProcessIncoming(XPacket *) override;

  uint32_t GetAccountId() const { return _accountId; }

  std::string GetAccountName() const;

  Player *GetPlayer() const { return m_pPlayer != nullptr ? m_pPlayer : nullptr; }

  void HandleNullPacket(XPacket *) {}

  // Client-Auth & Logout
  void onAuthResult(const TS_AG_CLIENT_LOGIN *);
  void onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH *);
  void onCharacterList(const TS_CS_CHARACTER_LIST *);
  void onLogin(const TS_CS_LOGIN *);
  void onReturnToLobby(const TS_CS_RETURN_LOBBY *);
  void onRequestReturnToLobby(const TS_CS_REQUEST_RETURN_LOBBY *);
  void onLogoutTimerRequest(const TS_CS_REQUEST_LOGOUT *);
  // Game itself
  void onCharacterName(const TS_CS_CHECK_CHARACTER_NAME *);
  void onCreateCharacter(const TS_CS_CREATE_CHARACTER *);
  void onDeleteCharacter(const TS_CS_DELETE_CHARACTER *);
  void onChatRequest(const TS_CS_CHAT_REQUEST *);

  void onMoveRequest(const TS_CS_MOVE_REQUEST *);
  void onPing(const TS_CS_PING *);
  void onRegionUpdate(const TS_CS_REGION_UPDATE *);
  void onChangeLocation(const TS_CS_CHANGE_LOCATION *);
  void onQuery(const TS_CS_QUERY *);
  void onUpdate(const TS_CS_UPDATE *);
  void onTimeSync(const TS_TIMESYNC *);
  void onGameTime(const TS_CS_GAME_TIME *);
  void onSetProperty(const TS_CS_SET_PROPERTY *);

  void onJobLevelUp(const TS_CS_JOB_LEVEL_UP *);
  void onLearnSkill(const TS_CS_LEARN_SKILL *);

  /* Trade related*/
  void onTrade(const TS_TRADE *); // Main packet
  // Those aren't actually packethandlers, but they get used by onTrade
  void onRequestTrade(uint);
  void onAcceptTrade(uint);
  void onCancelTrade();
  void onRejectTrade(uint);
  void onAddItem(uint, const TS_TRADE *);
  void onRemoveItem(uint, const TS_TRADE *);
  void onAddGold(uint, const TS_TRADE *);
  void onFreezeTrade();
  void onConfirmTrade(uint);

  void onPutOnItem(const TS_CS_PUTON_ITEM *);
  void onPutOffItem(const TS_CS_PUTOFF_ITEM *);
  void onBindSkillCard(const TS_CS_BIND_SKILLCARD *);
  void onUnBindSkilLCard(const TS_CS_UNBIND_SKILLCARD *);
  void onEquipSummon(const TS_EQUIP_SUMMON *);
  void onSoulStoneCraft(const TS_CS_SOULSTONE_CRAFT *);

  void onContact(const TS_CS_CONTACT *);
  void onDialog(const TS_CS_DIALOG *);
  void onDropQuest(const TS_CS_DROP_QUEST *);

  void onBuyItem(const TS_CS_BUY_ITEM *);
  void onSellItem(const TS_CS_SELL_ITEM *);
  void onUseItem(const TS_CS_USE_ITEM *);
  void onDropItem(const TS_CS_DROP_ITEM *);

  void onSkill(const TS_CS_SKILL *);
  void onMixRequest(const TS_CS_MIX *);
  void onRevive(const TS_CS_RESURRECTION *);
  void onAttackRequest(const TS_CS_ATTACK_REQUEST *);
  void onCancelAction(const TS_CS_CANCEL_ACTION *);
  void onTakeItem(const TS_CS_TAKE_ITEM *);

  void onGetSummonSetupInfo(const TS_CS_GET_SUMMON_SETUP_INFO *);
  void onStorage(const TS_CS_STORAGE *);

  void _SendResultMsg(uint16_t, uint16_t, int);
  void _PrepareCharacterList(uint32_t, std::vector<LOBBY_CHARACTER_INFO> *);

private:
  bool checkCharacterName(const std::string &);
  bool isValidTradeTarget(Player *);

  uint32_t m_nLastPing{0};
  uint32_t _accountId{};
  std::string _accountName{};
  Player *m_pPlayer{nullptr};
  bool _isAuthed{false};
  int m_nPermission;
};