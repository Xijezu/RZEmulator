/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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

#ifndef NGEMITY_GAMESESSION_H_
#define NGEMITY_GAMESESSION_H_

#include "Common.h"
#include "Log.h"
#include "WorldSocket.h"
#include "Encryption/XRc4Cipher.h"
#include "Entities/Player/Player.h"

struct LobbyCharacterInfo {
	int sex;
	int race;
	int model_id[5] = { 0 };
	int wear_info[24] = { 0 };
	int level;
	int job;
	int job_level;
	int exp;
	int hp;
	int mp;
	int permission;
	bool is_banned;
	std::string name;
	uint32 skin_color;
	std::string szCreateTime;
	std::string szDeleteTime;
	int wear_item_enhance_info[24] = { 0 };
	int wear_item_level_info[24] = { 0 };
};

enum STORAGE_MODE : int
{
	ITEM_INVENTORY_TO_STORAGE = 0x0,
	ITEM_STORAGE_TO_INVENTORY = 0x1,
	GOLD_INVENTORY_TO_STORAGE = 0x2,
	GOLD_STORAGE_TO_INVENTORY = 0x3,
	STORAGE_CLOSE = 0x4,
};

// Handle the player network
class WorldSession
{
	public:
		friend class Player;
		explicit WorldSession(WorldSocket<WorldSession> *socket);
		virtual ~WorldSession();

		// Accept & Close handler
		//void OnAccept() override;
		void OnClose();
		void KickPlayer();
		bool Update(uint diff);

		//void Decrypt(void *, size_t, bool/* =false */) override;
		//void Encrypt(void *, size_t, bool/* =false */) override;
		void ProcessIncoming(XPacket *);

		uint32 GetAccountId() const { return _accountId; }

		std::string GetAccountName() const { return m_pPlayer != nullptr ? m_pPlayer->GetName() : "<null>"; }

		Player *GetPlayer() const { return m_pPlayer != nullptr ? m_pPlayer : nullptr; }

		WorldSocket<WorldSession> *GetSocket() const { return _socket != nullptr ? _socket : nullptr; }

		void HandleNullPacket(XPacket *) {}

		// Client-Auth & Logout
		void onAuthResult(XPacket *);
		void onAccountWithAuth(XPacket *);
		void onCharacterList(XPacket *);
		void onLogin(XPacket *);
		void onReturnToLobby(XPacket *);
		void onLogoutTimerRequest(XPacket *);
		// Game itself
		void onCharacterName(XPacket *);
		void onCreateCharacter(XPacket *);
		void onDeleteCharacter(XPacket *);
		void onChatRequest(XPacket *);

		void onMoveRequest(XPacket *);
		void onRegionUpdate(XPacket *);
		void onChangeLocation(XPacket *);
		void onQuery(XPacket *);
		void onUpdate(XPacket *);
		void onTimeSync(XPacket *);
		void onGameTime(XPacket *);
		void onSetProperty(XPacket *);

		void onJobLevelUp(XPacket *);
		void onLearnSkill(XPacket *);

        /* Trade related */
        void onTrade(XPacket *); // Main packet
        // Those aren't actually packethandlers, but they get used by onTrade
        void onRequestTrade(uint);
        void onAcceptTrade(uint);
        void onCancelTrade();
        void onRejectTrade(uint);
        void onAddItem(uint, XPacket *);
        void onRemoveItem(uint, XPacket *);
        void onAddGold(uint, XPacket *);
        void onFreezeTrade();
        void onConfirmTrade();


		void onPutOnItem(XPacket *);
		void onPutOffItem(XPacket *);
		void onBindSkillCard(XPacket *);
		void onUnBindSkilLCard(XPacket *);
		void onEquipSummon(XPacket *);
		void onSoulStoneCraft(XPacket *);

		void onContact(XPacket *);
		void onDialog(XPacket *);
        void onDropQuest(XPacket*);

		void onBuyItem(XPacket *);
		void onSellItem(XPacket *);
		void onUseItem(XPacket *);

		void onSkill(XPacket *);
		void onDropItem(XPacket *);
		void onMixRequest(XPacket *);
		void onRevive(XPacket *);
		void onAttackRequest(XPacket *);
		void onCancelAction(XPacket *);
		void onTakeItem(XPacket *);

		void onGetSummonSetupInfo(XPacket *);
		void onStorage(XPacket *);

		void _SendResultMsg(uint16, uint16, int);
		std::vector<LobbyCharacterInfo> _PrepareCharacterList(uint32);
	private:
		bool checkCharacterName(const std::string &);
		bool isValidTradeTarget(Player*);
		WorldSocket<WorldSession> *_socket{nullptr};

		uint32      _accountId{ };
		std::string _accountName{ };
		Player      *m_pPlayer{nullptr};
		bool        _isAuthed{false};
		int 		m_nPermission;
};

#endif // NGEMITY_GAMESESSION_H_