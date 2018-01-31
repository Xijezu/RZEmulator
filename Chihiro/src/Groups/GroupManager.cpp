/*
  *  Copyright (C) 2018 Xijezu <http://xijezu.com/>
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
#include "Player.h"
#include "DatabaseEnv.h"
#include "Timer.h"
#include "Messages.h"

int GroupManager::GetAttackTeamLeadPartyID(int nPartyID)
{
    return 0;
}

int GroupManager::GetAttackTeamGuildID(int nPartyID)
{
    return 0;
}

bool GroupManager::IsAttackTeamParty(int nPartyID)
{
    return false;
}

PARTY_TYPE GroupManager::GetPartyType(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->ePartyType;
    return TYPE_UNKNOWN;
}

bool GroupManager::DestroyParty(int nPartyID)
{
    auto name = GetPartyName(nPartyID);
    DoEachMemberTag(nPartyID, [&name](PartyMemberTag& tag) {
        if(tag.bIsOnline && tag.pPlayer != nullptr)
        {
            tag.pPlayer->SetInt32Value(PLAYER_FIELD_PARTY_ID, 0);
            Messages::SendChatMessage(100, "@PARTY", tag.pPlayer, string_format("DESTROY|%s|", name));
        }
    });
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_DEL_PARTY);
    stmt->setInt32(0, nPartyID);
    CharacterDatabase.Execute(stmt);

    {
        MX_UNIQUE_GUARD writeGuard(i_lock);
        m_hshPartyID.erase(nPartyID);
    }
}

int GroupManager::GetMemberCount(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return (int)info->vMemberNameList.size();
    return 0;
}

std::string GroupManager::GetPartyName(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->strPartyName;
    return "";
}

bool GroupManager::IsLeader(int nPartyID, const std::string &szPlayerName)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->strLeaderName == szPlayerName;
    return false;
}

void GroupManager::OnChangeCharacterLevel(int nPartyID, const std::string &szName, int nLevel)
{

}

void GroupManager::OnChangeCharacterJob(int nPartyID, const std::string &szName, int nJobID)
{

}

std::string GroupManager::GetLeaderName(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->strLeaderName;
    return "";
}

bool GroupManager::LeaveParty(int nPartyID, const std::string &szName)
{
    PartyInfo* info{nullptr};
    Messages::SendPartyChatMessage(100, "@PARTY", nPartyID, string_format("LEAVE|%s|", szName.c_str()));
    {
        MX_UNIQUE_GUARD writeLock(i_lock);
        info = getPartyInfoNC(nPartyID);
        if (info == nullptr || iequals(info->strLeaderName, szName))
            return false;

        for (auto it = info->vMemberNameList.begin(); it != info->vMemberNameList.end(); ++it)
        {
            if (iequals(szName, it->strName))
            {
                info->vMemberNameList.erase(it);
                break;
            }
        }
    }
    for(auto& member : info->vMemberNameList)
    {
        if(member.bIsOnline && member.pPlayer != nullptr)
        {
            Messages::SendPartyInfo(member.pPlayer);
        }
    }
    return true;
}

bool GroupManager::onLogin(int nPartyID, Player *pPlayer)
{
    bool result = false;
    {
        MX_UNIQUE_GUARD writeGuard(i_lock);
        auto            info = getPartyInfoNC(nPartyID);
        if (info == nullptr)
            return result;
        for (auto &tag : info->vMemberNameList)
        {
            if (iequals(tag.strName, pPlayer->GetNameAsString()))
            {
                tag.bIsOnline = true;
                tag.pPlayer   = pPlayer;
                tag.nLevel    = pPlayer->GetLevel();
                tag.nJobID    = pPlayer->GetCurrentJob();
                result = true;
            }
        }
    }
    if (result)
    {
        Messages::BroadcastPartyLoginStatus(nPartyID, true, pPlayer->GetNameAsString());
        Messages::BroadcastPartyMemberInfo(pPlayer);
    }
    return result;
}

bool GroupManager::onLogout(int nPartyID, Player *pPlayer)
{
    bool result = false;
    {
        MX_UNIQUE_GUARD writeGuard(i_lock);
        auto            info = getPartyInfoNC(nPartyID);
        if (info == nullptr)
            return result;
        for (auto &tag : info->vMemberNameList)
        {
            if (iequals(tag.strName, pPlayer->GetNameAsString()))
            {
                tag.bIsOnline = false;
                tag.pPlayer   = nullptr;
                result = true;
            }
        }
    }
    if (result)
    {
        Messages::BroadcastPartyLoginStatus(nPartyID, false, pPlayer->GetNameAsString());
    }
    return result;
}

void GroupManager::GetNearMember(Player *pPlayer, float distance, std::vector<Player *>& vList)
{
    auto info = getPartyInfo(pPlayer->GetPartyID());
    if(info == nullptr)
        return;

    for(auto& p : info->vMemberNameList)
    {
        auto player = Player::FindPlayer(p.strName);
        if(player != nullptr && player->GetExactDist2d(pPlayer) <= distance)
        {
            vList.emplace_back(player);
        }
    }
}

PartyInfo* GroupManager::getPartyInfo(int nPartyID)
{
    PartyInfo* info{nullptr};
    {
        MX_SHARED_GUARD readLock(i_lock);
        if(m_hshPartyID.count(nPartyID) != 0)
            info = &m_hshPartyID[nPartyID];
    }
    return info;
}

PartyInfo* GroupManager::getPartyInfoNC(int nPartyID)
{
    PartyInfo* info{nullptr};
    {
        if(m_hshPartyID.count(nPartyID) != 0)
            info = &m_hshPartyID[nPartyID];
    }
    return info;
}


int GroupManager::CreateParty(Player *pPlayer, const std::string &szName, PARTY_TYPE partyType)
{
    PartyInfo partyInfo{ };
    {
        MX_UNIQUE_GUARD writeLock(i_lock);
        for (auto &party : m_hshPartyID)
        {
            if (party.second.strPartyName == szName)
            {
                return -1;
            }
        }

        partyInfo.strPartyName         = szName;
        partyInfo.strLeaderName        = pPlayer->GetName();
        partyInfo.ePartyType           = partyType;
        partyInfo.nPartyID             = (int)++m_nMaxPartyID;
        partyInfo.eShareMode           = ITEM_SHARE_RANDOM;
        partyInfo.nLastItemAcquirerIdx = 0;
        partyInfo.nLeaderSID           = pPlayer->GetUInt32Value(UNIT_FIELD_UID);
        partyInfo.nLeaderJobID         = pPlayer->GetCurrentJob();
        partyInfo.nPartyPassword       = (uint)rand32();
        m_hshPartyID[partyInfo.nPartyID] = partyInfo;
    }
    JoinParty(partyInfo.nPartyID, pPlayer, partyInfo.nPartyPassword);
    AddGroupToDatabase(partyInfo);
    return partyInfo.nPartyID;
}

bool GroupManager::JoinParty(int nPartyID, Player* pPlayer, uint nPass)
{
    MX_UNIQUE_GUARD writeLock(i_lock);
    auto info = getPartyInfoNC(nPartyID);
    if(info == nullptr || info->vMemberNameList.size() >= 8 || nPass != info->nPartyPassword)
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

void GroupManager::DoEachMemberTag(int nPartyID, std::function<void (PartyMemberTag&)> fn)
{
    {
        MX_SHARED_GUARD readGuard(i_lock);
        auto            info = getPartyInfoNC(nPartyID);
        if (info != nullptr)
        {
            for (auto &tag : info->vMemberNameList)
            {
                fn(tag);
            }
        }
    }
}

int GroupManager::GetMinLevel(int nPartyID)
{
    int min = 0xFF;
    DoEachMemberTag(nPartyID, [&min](PartyMemberTag& tag)
    {
        if(tag.nLevel < min)
            min = tag.nLevel;
    });
    return min;
}

uint GroupManager::GetPassword(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->nPartyPassword;
    return 0;
}


int GroupManager::GetMaxLevel(int nPartyID)
{
    int max = 0;
    DoEachMemberTag(nPartyID, [&max](PartyMemberTag& tag)
    {
        if(tag.nLevel > max)
            max = tag.nLevel;
    });
    return max;
}

int GroupManager::GetShareMode(int nPartyID)
{
    auto info = getPartyInfo(nPartyID);
    if(info != nullptr)
        return info->eShareMode;
    return 0;
}

void GroupManager::InitGroupSystem()
{
    uint32_t    oldMSTime = getMSTime();
    m_nMaxPartyID = CharacterDatabase.Query("SELECT MAX(sid) FROM Party;").get()->Fetch()->GetUInt64();
    QueryResult result    = CharacterDatabase.Query("SELECT * FROM Party;");
    if (!result)
    {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 Parties. Table `Party` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        uint idx = 0;
        Field* field = result->Fetch();
        PartyInfo info{};
        info.nPartyID = field[idx++].GetInt32();
        info.strPartyName = field[idx++].GetString();
        info.nLeaderSID = field[idx++].GetInt32();
        info.eShareMode = (ITEM_SHARE_MODE)field[idx].GetInt32();
        LoadPartyInfo(info);
        m_hshPartyID[info.nPartyID] = info;
        count++;
    } while (result->NextRow());

    MX_LOG_INFO("server.worldserver", ">> Loaded %u Parties in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
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
    QueryResult result    = CharacterDatabase.Query(string_format("SELECT sid, name, job, lv FROM `Character` WHERE party_id = %d;", info.nPartyID).c_str());
    if (!result)
    {
        MX_LOG_INFO("server.worldserver", "Invalid party ID %d!", info.nPartyID);
        return;
    }

    do
    {
        uint32 idx = 0;
        Field* field = result->Fetch();
        PartyMemberTag tag{};
        tag.sid = field[idx++].GetInt32();
        tag.strName = field[idx++].GetString();
        tag.nJobID = field[idx++].GetInt32();
        tag.nLevel = field[idx].GetInt32();
        tag.bIsOnline = 0;
        tag.pPlayer = nullptr;
        if(tag.sid == info.nLeaderSID)
        {
            info.strLeaderName = tag.strName;
            info.nLeaderJobID = tag.nJobID;
        }
        info.vMemberNameList.emplace_back(tag);
    } while (result->NextRow());
}