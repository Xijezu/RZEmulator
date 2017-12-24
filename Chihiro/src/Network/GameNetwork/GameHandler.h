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

#ifndef _GAMESESSION_H_
#define _GAMESESSION_H_

#include "Common.h"
#include "Log.h"
#include "Server/XSocket.h"
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

// Handle the player network
class GameSession: public XSocket::Session {
public:
    explicit GameSession(XSocket &socket);
    virtual ~GameSession();

    // Accept & Close handler
    void OnAccept() override;
    void OnClose() override;

    void Decrypt(void *, size_t, bool/* =false */) override;
    void Encrypt(void *, size_t, bool/* =false */) override;
    void ProcessIncoming(XPacket *) override;

    uint32 GetAccountId() const
    { return _accountId; }

    Player *GetPlayer() const
    { return _player != nullptr ? _player : nullptr; }

    XSocket &GetSocket(void)
    { return _socket; }

    bool HandleNullPacket(XPacket *)
    { return true; }

    // Client-Auth & Logout
    bool onAuthResult(XPacket *);
    bool onAccountWithAuth(XPacket *);
    bool onCharacterList(XPacket *);
    bool onLogin(XPacket *);
    bool onReturnToLobby(XPacket *);
    bool onLogoutTimerRequest(XPacket *);
    // Game itself
    bool onCharacterName(XPacket *);
    bool onCreateCharacter(XPacket *);
	bool onDeleteCharacter(XPacket *);
    bool onChatRequest(XPacket *);

    bool onMoveRequest(XPacket *);
    bool onRegionUpdate(XPacket *);
	bool onChangeLocation(XPacket *);
	bool onQuery(XPacket *);
	bool onUpdate(XPacket *);
    bool onTimeSync(XPacket *);
    bool onGameTime(XPacket *);

	bool onJobLevelUp(XPacket *);

    bool onPutOnItem(XPacket *);
    bool onPutOffItem(XPacket *);

    bool onContact(XPacket *);
	bool onDialog(XPacket *);

	bool onBuyItem(XPacket *);

    bool onGetSummonSetupInfo(XPacket *);

    void _SendMoveMsg(Object &obj, Position nPos, std::vector<Position> vMoveInfo);
    void _SendResultMsg(uint16, uint16, int);
    std::vector<LobbyCharacterInfo> _PrepareCharacterList(uint32);
private:
    bool checkCharacterName(std::string);
    XSocket &_socket;
    XRC4Cipher _rc4encode{ };
    XRC4Cipher _rc4decode{ };

    uint32      _accountId{ };
    std::string _accountName{ };
    Player *_player{nullptr};
    bool _isAuthed{false};
};

#endif // _GAMESESSION_H_