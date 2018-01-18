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
    return false;
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
    return false;
}

bool GroupManager::onLogin(int nPartyID, Player *pPlayer)
{
    bool result = false;
    MX_UNIQUE_GUARD writeGuard(i_lock);
    auto info = getPartyInfoNC(nPartyID);
    if(info == nullptr)
        return result;
    for(auto& tag : info->vMemberNameList)
    {
        if(tag.strName == pPlayer->GetName())
        {
            tag.bIsOnline = true;
            result = true;
        }
    }
    if(result)
    {
        // @todo do each party login info
    }
    return result;
}

bool GroupManager::onLogout(int nPartyID, Player *pPlayer)
{
    return false;
}

void GroupManager::GetNearMember(Player *pPlayer, float distance, std::vector<Player *> vList)
{

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
        partyInfo.eShareMode           = ITEM_SHARE_MODE::ITEM_SHARE_RANDOM;
        partyInfo.nLastItemAcquirerIdx = 0;
        partyInfo.nLeaderSID           = pPlayer->GetUInt32Value(UNIT_FIELD_UID);
        partyInfo.nLeaderJobID         = pPlayer->GetCurrentJob();
        partyInfo.nPartyPassword       = 0;
        m_hshPartyID[partyInfo.nPartyID] = partyInfo;
    }
    JoinParty(partyInfo.nPartyID, pPlayer);
    AddGroupToDatabase(partyInfo);
    return partyInfo.nPartyID;
}

bool GroupManager::JoinParty(int nPartyID, Player* pPlayer)
{
    MX_UNIQUE_GUARD writeLock(i_lock);
    auto info = getPartyInfoNC(nPartyID);
    if(info == nullptr || info->vMemberNameList.size() >= 8)
        return false;

    PartyMemberTag tag{};
    tag.nLevel = pPlayer->GetLevel();
    tag.sid = pPlayer->GetUInt32Value(UNIT_FIELD_UID);
    tag.bIsOnline = true;
    tag.nJobID = pPlayer->GetCurrentJob();
    tag.strName = pPlayer->GetName();
    info->vMemberNameList.emplace_back(tag);
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
    m_nMaxPartyID = CharacterDatabase.Query("SELECT MAX(sid) FROM Item;").get()->Fetch()->GetUInt64();
    QueryResult result    = CharacterDatabase.Query("SELECT * FROM Party;");
    if (!result)
    {
        MX_LOG_INFO("server.worldserver", ">> Loaded 0 Parties. Table `Party` is empty!");
        return;
    }

    uint32 count = 0, idx = 0;
    do
    {
        Field* field = result->Fetch();
        PartyInfo info{};
        info.nPartyID = field[idx++].GetInt32();
        info.strPartyName = field[idx++].GetString();
        info.nLeaderSID = field[idx++].GetInt32();
        info.eShareMode = (ITEM_SHARE_MODE)field[idx++].GetInt32();
        LoadPartyInfo(info);
        m_hshPartyID[info.nPartyID] = info;
    } while (result->NextRow());

    MX_LOG_INFO("server.worldserver", ">> Loaded %u Monstertemplates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
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
    uint32_t    oldMSTime = getMSTime();
    QueryResult result    = CharacterDatabase.Query(string_format("SELECT sid, name, job, lv FROM `Character` WHERE party_id = %d;", info.nPartyID).c_str());
    if (!result)
    {
        MX_LOG_INFO("server.worldserver", "Invalid party ID %d!", info.nPartyID);
        return;
    }

    uint32 idx = 0;
    do
    {
        Field* field = result->Fetch();
        PartyMemberTag tag{};
        tag.sid = field[idx++].GetInt32();
        tag.strName = field[idx++].GetString();
        tag.nJobID = field[idx++].GetInt32();
        tag.nLevel = field[idx++].GetInt32();
        tag.bIsOnline = 0;
        if(tag.sid == info.nLeaderSID)
        {
            info.strLeaderName = tag.strName;
            info.nLeaderJobID = tag.nJobID;
        }
        info.vMemberNameList.emplace_back(tag);
    } while (result->NextRow());
}
