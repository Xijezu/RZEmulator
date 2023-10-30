#pragma once
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
#include <functional>
#include <map>

#include "Common.h"
#include "SharedMutex.h"

enum PARTY_TYPE : int {
    TYPE_UNKNOWN = 0xFF,
    TYPE_NORMAL_PARTY = 0,
    TYPE_RAID_ATTACKTEAM = 1,
    TYPE_SIEGE_ATTACKTEAM = 2,
    /*TYPE_HUNTAHOLIC_PARTY           = 3,
    TYPE_BATTLE_ARENA_TEAM          = 4,
    TYPE_BATTLE_ARENA_EXERCISE_TEAM = 5*/
};

enum ITEM_SHARE_MODE : int { ITEM_SHARE_MONOPOLY = 0, ITEM_SHARE_RANDOM = 1, ITEM_SHARE_LINEAR = 2 };

class Player;
struct PartyMemberTag {
    bool bIsOnline;
    int32_t sid;
    std::string strName;
    Player *pPlayer;
    int32_t nLevel;
    int32_t nJobID;
};

struct PartyInfo {
    uint32_t nPartyPassword;
    int32_t nPartyID;
    int32_t nLeaderSID;
    ITEM_SHARE_MODE eShareMode;
    PARTY_TYPE ePartyType;
    std::string strPartyName;
    std::string strLeaderName;
    int32_t nLeaderJobID;
    std::vector<PartyMemberTag> vMemberNameList;
    std::vector<uint32_t> vOnlineList;
    int32_t nLastItemAcquirerIdx;
    // ATtackTeamInfo
};

class Player;
class GroupManager {
public:
    ~GroupManager() = default;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    GroupManager(const GroupManager &) = delete;
    GroupManager &operator=(const GroupManager &) = delete;

    static GroupManager &Instance()
    {
        static GroupManager instance;
        return instance;
    }

    void InitGroupSystem();

    ///- Getters
    int32_t GetAttackTeamLeadPartyID(int32_t nPartyID);
    int32_t GetAttackTeamGuildID(int32_t nPartyID);
    int32_t GetMemberCount(int32_t nPartyID);
    int32_t GetMinLevel(int32_t nPartyID);
    int32_t GetMaxLevel(int32_t nPartyID);
    int32_t GetShareMode(int32_t nPartyID);
    uint32_t GetPassword(int32_t nPartyID);
    std::string GetPartyName(int32_t nPartyID);
    PARTY_TYPE GetPartyType(int32_t nPartyID);
    std::string GetLeaderName(int32_t nPartyID);
    void GetNearMember(Player *pPlayer, float distance, std::vector<Player *> &vList);

    ///- booleans
    bool IsLeader(int32_t nPartyID, const std::string &szPlayerName);
    bool IsAttackTeamParty(int32_t nPartyID);

    ///- Functionality
    bool JoinParty(int32_t nPartyID, Player *pPlayer, uint32_t nPass);
    bool DestroyParty(int32_t nPartyID);
    bool LeaveParty(int32_t nPartyID, const std::string &szName);
    int32_t CreateParty(Player *pPlayer, const std::string &szName, PARTY_TYPE partyType);
    void DoEachMemberTag(int32_t nPartyID, std::function<void(PartyMemberTag &)> fn);
    int32_t DoEachMemberTagNum(int32_t nPartyID, std::function<bool(PartyMemberTag &)> fn);

    ///- Events
    void OnChangeCharacterLevel(int32_t nPartyID, const std::string &szName, int32_t nLevel);
    void OnChangeCharacterJob(int32_t nPartyID, const std::string &szName, int32_t nJobID);
    bool onLogin(int32_t nPartyID, Player *pPlayer);
    bool onLogout(int32_t nPartyID, Player *pPlayer);

protected:
    void AddGroupToDatabase(const PartyInfo &);
    void LoadPartyInfo(PartyInfo &info);
    PartyInfo *getPartyInfo(int32_t nPartyID);
    PartyInfo *getPartyInfoNC(int32_t nPartyID);
    GroupManager() = default;

private:
    uint64_t m_nMaxPartyID{0};
    std::map<int, PartyInfo> m_hshPartyID{};
    NG_SHARED_MUTEX i_lock;
};

#define sGroupManager GroupManager::Instance()
