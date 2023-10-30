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

#include "GroupManager.h"

#include "DatabaseEnv.h"
#include "Log.h"
#include "Messages.h"
#include "Player.h"

int32_t GroupManager::GetAttackTeamLeadPartyID(int32_t nPartyID)
{
    return 0;
}

int32_t GroupManager::GetAttackTeamGuildID(int32_t nPartyID)
{
    return 0;
}

bool GroupManager::IsAttackTeamParty(int32_t nPartyID)
{
    return false;
}

PARTY_TYPE GroupManager::GetPartyType(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->ePartyType;
    return TYPE_UNKNOWN;
}

bool GroupManager::DestroyParty(int32_t nPartyID)
{
    auto name = GetPartyName(nPartyID);
    DoEachMemberTag(nPartyID, [&name](PartyMemberTag &tag) {
        if (tag.bIsOnline && tag.pPlayer != nullptr) {
            tag.pPlayer->SetInt32Value(PLAYER_FIELD_PARTY_ID, 0);
            Messages::SendChatMessage(100, "@PARTY", tag.pPlayer, NGemity::StringFormat("DESTROY|{}|", name));
        }
    });
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_PARTY);
    stmt->setInt32(0, nPartyID);
    CharacterDatabase.Execute(stmt);

    {
        NG_UNIQUE_GUARD writeGuard(i_lock);
        m_hshPartyID.erase(nPartyID);
    }
    return true;
}

int32_t GroupManager::GetMemberCount(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return (int32_t)info->vMemberNameList.size();
    return 0;
}

std::string GroupManager::GetPartyName(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->strPartyName;
    return "";
}

bool GroupManager::IsLeader(int32_t nPartyID, const std::string &szPlayerName)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->strLeaderName == szPlayerName;
    return false;
}

void GroupManager::OnChangeCharacterLevel(int32_t nPartyID, const std::string &szName, int32_t nLevel) {}

void GroupManager::OnChangeCharacterJob(int32_t nPartyID, const std::string &szName, int32_t nJobID) {}

std::string GroupManager::GetLeaderName(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->strLeaderName;
    return "";
}

bool GroupManager::LeaveParty(int32_t nPartyID, const std::string &szName)
{
    PartyInfo *info{nullptr};
    Messages::SendPartyChatMessage(100, "@PARTY", nPartyID, NGemity::StringFormat("LEAVE|{}|", szName));
    {
        NG_UNIQUE_GUARD writeLock(i_lock);
        info = getPartyInfoNC(nPartyID);
        if (info == nullptr || iequals(info->strLeaderName, szName))
            return false;

        for (auto it = info->vMemberNameList.begin(); it != info->vMemberNameList.end(); ++it) {
            if (iequals(szName, it->strName)) {
                info->vMemberNameList.erase(it);
                break;
            }
        }
    }
    for (auto &member : info->vMemberNameList) {
        if (member.bIsOnline && member.pPlayer != nullptr) {
            Messages::SendPartyInfo(member.pPlayer);
        }
    }
    return true;
}

bool GroupManager::onLogin(int32_t nPartyID, Player *pPlayer)
{
    bool result = false;
    {
        NG_UNIQUE_GUARD writeGuard(i_lock);
        auto info = getPartyInfoNC(nPartyID);
        if (info == nullptr)
            return result;
        for (auto &tag : info->vMemberNameList) {
            if (iequals(tag.strName, pPlayer->GetNameAsString())) {
                tag.bIsOnline = true;
                tag.pPlayer = pPlayer;
                tag.nLevel = pPlayer->GetLevel();
                tag.nJobID = pPlayer->GetCurrentJob();
                result = true;
            }
        }
    }
    if (result) {
        Messages::BroadcastPartyLoginStatus(nPartyID, true, pPlayer->GetNameAsString());
        Messages::BroadcastPartyMemberInfo(pPlayer);
    }
    return result;
}

bool GroupManager::onLogout(int32_t nPartyID, Player *pPlayer)
{
    bool result = false;
    {
        NG_UNIQUE_GUARD writeGuard(i_lock);
        auto info = getPartyInfoNC(nPartyID);
        if (info == nullptr)
            return result;
        for (auto &tag : info->vMemberNameList) {
            if (iequals(tag.strName, pPlayer->GetNameAsString())) {
                tag.bIsOnline = false;
                tag.pPlayer = nullptr;
                result = true;
            }
        }
    }
    if (result) {
        Messages::BroadcastPartyLoginStatus(nPartyID, false, pPlayer->GetNameAsString());
    }
    return result;
}

void GroupManager::GetNearMember(Player *pPlayer, float distance, std::vector<Player *> &vList)
{
    auto info = getPartyInfo(pPlayer->GetPartyID());
    if (info == nullptr)
        return;

    for (auto &p : info->vMemberNameList) {
        auto player = Player::FindPlayer(p.strName);
        if (player != nullptr && player->GetExactDist2d(pPlayer) <= distance) {
            vList.emplace_back(player);
        }
    }
}

PartyInfo *GroupManager::getPartyInfo(int32_t nPartyID)
{
    PartyInfo *info{nullptr};
    {
        NG_SHARED_GUARD readLock(i_lock);
        if (m_hshPartyID.count(nPartyID) != 0)
            info = &m_hshPartyID[nPartyID];
    }
    return info;
}

PartyInfo *GroupManager::getPartyInfoNC(int32_t nPartyID)
{
    PartyInfo *info{nullptr};
    {
        if (m_hshPartyID.count(nPartyID) != 0)
            info = &m_hshPartyID[nPartyID];
    }
    return info;
}

int32_t GroupManager::CreateParty(Player *pPlayer, const std::string &szName, PARTY_TYPE partyType)
{
    PartyInfo partyInfo{};
    {
        NG_UNIQUE_GUARD writeLock(i_lock);
        for (auto &party : m_hshPartyID) {
            if (party.second.strPartyName == szName) {
                return -1;
            }
        }

        partyInfo.strPartyName = szName;
        partyInfo.strLeaderName = pPlayer->GetName();
        partyInfo.ePartyType = partyType;
        partyInfo.nPartyID = (int32_t)++m_nMaxPartyID;
        partyInfo.eShareMode = ITEM_SHARE_RANDOM;
        partyInfo.nLastItemAcquirerIdx = 0;
        partyInfo.nLeaderSID = pPlayer->GetUInt32Value(UNIT_FIELD_UID);
        partyInfo.nLeaderJobID = pPlayer->GetCurrentJob();
        partyInfo.nPartyPassword = (uint32_t)rand32();
        m_hshPartyID[partyInfo.nPartyID] = partyInfo;
    }
    JoinParty(partyInfo.nPartyID, pPlayer, partyInfo.nPartyPassword);
    AddGroupToDatabase(partyInfo);
    return partyInfo.nPartyID;
}

bool GroupManager::JoinParty(int32_t nPartyID, Player *pPlayer, uint32_t nPass)
{
    NG_UNIQUE_GUARD writeLock(i_lock);
    auto info = getPartyInfoNC(nPartyID);
    if (info == nullptr || info->vMemberNameList.size() >= 8 || nPass != info->nPartyPassword)
        return false;

    PartyMemberTag tag{};
    tag.nLevel = pPlayer->GetLevel();
    tag.sid = pPlayer->GetUInt32Value(UNIT_FIELD_UID);
    tag.bIsOnline = true;
    tag.pPlayer = pPlayer;
    tag.nJobID = pPlayer->GetCurrentJob();
    tag.strName = pPlayer->GetName();
    info->vMemberNameList.emplace_back(tag);
    pPlayer->SetInt32Value(PLAYER_FIELD_PARTY_ID, nPartyID);
    return true;
}

void GroupManager::DoEachMemberTag(int32_t nPartyID, std::function<void(PartyMemberTag &)> fn)
{
    {
        NG_SHARED_GUARD readGuard(i_lock);
        auto info = getPartyInfoNC(nPartyID);
        if (info != nullptr) {
            for (auto &tag : info->vMemberNameList) {
                fn(tag);
            }
        }
    }
}

int32_t GroupManager::DoEachMemberTagNum(int32_t nPartyID, std::function<bool(PartyMemberTag &)> fn)
{
    {
        NG_SHARED_GUARD readGuard(i_lock);
        auto info = getPartyInfoNC(nPartyID);
        int32_t nCnt{0};
        if (info != nullptr) {
            for (auto &tag : info->vMemberNameList) {
                if (fn(tag))
                    nCnt++;
            }
        }
        return nCnt;
    }
}


int32_t GroupManager::GetMinLevel(int32_t nPartyID)
{
    int32_t min = 0xFF;
    DoEachMemberTag(nPartyID, [&min](PartyMemberTag &tag) {
        if (tag.nLevel < min)
            min = tag.nLevel;
    });
    return min;
}

uint32_t GroupManager::GetPassword(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->nPartyPassword;
    return 0;
}

int32_t GroupManager::GetMaxLevel(int32_t nPartyID)
{
    int32_t max = 0;
    DoEachMemberTag(nPartyID, [&max](PartyMemberTag &tag) {
        if (tag.nLevel > max)
            max = tag.nLevel;
    });
    return max;
}

int32_t GroupManager::GetShareMode(int32_t nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if (info != nullptr)
        return info->eShareMode;
    return 0;
}

void GroupManager::InitGroupSystem()
{
    uint32_t oldMSTime = getMSTime();
    m_nMaxPartyID = CharacterDatabase.Query("SELECT MAX(sid) FROM Party;").get()->Fetch()->GetUInt64();
    QueryResult result = CharacterDatabase.Query("SELECT * FROM Party;");
    if (!result) {
        NG_LOG_INFO("server.worldserver", ">> Loaded 0 Parties. Table `Party` is empty!");
        return;
    }

    uint32_t count = 0;
    do {
        uint32_t idx = 0;
        Field *field = result->Fetch();
        PartyInfo info{};
        info.nPartyID = field[idx++].GetInt32();
        info.strPartyName = field[idx++].GetString();
        info.nLeaderSID = field[idx++].GetInt32();
        info.eShareMode = (ITEM_SHARE_MODE)field[idx].GetInt32();
        LoadPartyInfo(info);
        m_hshPartyID[info.nPartyID] = info;
        count++;
    } while (result->NextRow());

    NG_LOG_INFO("server.worldserver", ">> Loaded %u Parties in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GroupManager::AddGroupToDatabase(const PartyInfo &info)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_PARTY);
    stmt->setInt32(0, info.nPartyID);
    stmt->setString(1, info.strPartyName);
    stmt->setInt32(2, info.nLeaderSID);
    stmt->setInt32(3, info.eShareMode);
    stmt->setInt32(4, 0);
    stmt->setInt32(5, 0);
    CharacterDatabase.Execute(stmt);
}

void GroupManager::LoadPartyInfo(PartyInfo &info)
{
    QueryResult result = CharacterDatabase.Query(NGemity::StringFormat("SELECT sid, name, job, lv FROM `Character` WHERE party_id = {};", info.nPartyID).c_str());
    if (!result) {
        NG_LOG_INFO("server.worldserver", "Invalid party ID %d!", info.nPartyID);
        return;
    }

    do {
        uint32_t idx = 0;
        Field *field = result->Fetch();
        PartyMemberTag tag{};
        tag.sid = field[idx++].GetInt32();
        tag.strName = field[idx++].GetString();
        tag.nJobID = field[idx++].GetInt32();
        tag.nLevel = field[idx].GetInt32();
        tag.bIsOnline = 0;
        tag.pPlayer = nullptr;
        if (tag.sid == info.nLeaderSID) {
            info.strLeaderName = tag.strName;
            info.nLeaderJobID = tag.nJobID;
        }
        info.vMemberNameList.emplace_back(tag);
    } while (result->NextRow());
}