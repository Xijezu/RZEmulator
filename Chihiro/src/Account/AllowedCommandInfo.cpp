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
 *  more details *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AllowedCommandInfo.h"

#include "GroupManager.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "Player.h"
#include "RegionContainer.h"
#include "Scripting/XLua.h"
#include "World.h"

typedef struct AllowedCommands
{
    std::string szCommand;
    bool bNeedsPermission;
    void (AllowedCommandInfo::*handler)(Player *, const std::string &);
} GameHandler;

const AllowedCommands commandHandler[] = {{"/run", true, &AllowedCommandInfo::onRunScript}, {"/sitdown", false, &AllowedCommandInfo::onCheatSitdown},
    {"/standup", false, &AllowedCommandInfo::onCheatStandup}, {"/position", false, &AllowedCommandInfo::onCheatPosition}, {"/battle", false, &AllowedCommandInfo::onBattleMode},
    {"/notice", true, &AllowedCommandInfo::onCheatNotice}, {"/suicide", true, &AllowedCommandInfo::onCheatSuicide}, {"/doit", true, &AllowedCommandInfo::onCheatKillAll},
    {"/pcreate", false, &AllowedCommandInfo::onCheatCreateParty}, {"/regenerate", true, &AllowedCommandInfo::onCheatRespawn}, {"/pinvite", false, &AllowedCommandInfo::onInviteParty},
    {"/pjoin", false, &AllowedCommandInfo::onJoinParty}, {"/plist", false, &AllowedCommandInfo::onPartyInfo}, {"/pdestroy", false, &AllowedCommandInfo::onPartyDestroy},
    {"/pleave", false, &AllowedCommandInfo::onLeaveParty}};

constexpr int32_t tableSize = (sizeof(commandHandler) / sizeof(AllowedCommands));

void AllowedCommandInfo::onCheatPosition(Player *pClient, const std::string & /* tokens*/)
{
    Messages::SendChatMessage(30, "@SYSTEM", pClient, pClient->ToString());
}

void AllowedCommandInfo::onRunScript(Player *pClient, const std::string &pScript)
{
    sScriptingMgr.RunString(pClient, pScript);
}

void AllowedCommandInfo::onCheatSitdown(Player *pClient, const std::string & /* tokens*/)
{
    if (pClient != nullptr && !pClient->bIsMoving && pClient->IsInWorld() && pClient->GetHealth() != 0 && pClient->IsSitdownable())
    {
        if (pClient->GetTargetHandle() != 0)
            pClient->CancelAttack();
        pClient->m_bSitdown = true;
        Messages::BroadcastStatusMessage(pClient);
    }
}

void AllowedCommandInfo::onBattleMode(Player *pClient, const std::string &tokens)
{
    auto target_handle = (uint32_t)std::stoul(tokens);
    auto unit = sMemoryPool.GetObjectInWorld<Unit>(target_handle);
    if (unit != nullptr)
        Messages::BroadcastStatusMessage(unit);
}

void AllowedCommandInfo::onCheatNotice(Player *pClient, const std::string &) {}

void AllowedCommandInfo::onCheatParty(Player *pClient, const std::string &) {}

void AllowedCommandInfo::Run(Player *pClient, const std::string &szMessage)
{
    Tokenizer tokenizer(szMessage, ' ');
    for (const auto &i : commandHandler)
    {
        if (i.szCommand == tokenizer[0] && (!i.bNeedsPermission || (i.bNeedsPermission && pClient->GetPermission() >= 100)))
        {
            auto pos = szMessage.find(' ');
            if (pos != std::string::npos)
                (*this.*i.handler)(pClient, szMessage.substr(pos + 1));
            else
                (*this.*i.handler)(pClient, "");
        }
    }
}

void AllowedCommandInfo::onCheatSuicide(Player * /*pClient*/, const std::string & /*szMessage*/)
{
    World::StopNow(SHUTDOWN_EXIT_CODE);
}

void AllowedCommandInfo::onCheatKillAll(Player *pClient, const std::string &)
{
    auto functor = [&pClient](RegionType &list) -> void {
        for (const auto &obj : list)
        {
            if (obj != nullptr && pClient != nullptr && obj->IsMonster())
                obj->As<Monster>()->TriggerForceKill(pClient);
        }
    };

    sRegion.DoEachVisibleRegion((uint32_t)pClient->GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE), (uint32_t)(pClient->GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)),
        pClient->GetLayer(), functor, (uint8_t)RegionVisitor::MovableVisitor);
}

void AllowedCommandInfo::onCheatRespawn(Player *pClient, const std::string &str)
{
    int32_t cnt = 1;
    Tokenizer tokens(str, ' ');
    if (tokens.size() < 1)
        return;
    auto i = std::stoi(tokens[0]);
    if (tokens.size() == 2)
        cnt = std::stoi(tokens[1]);
    auto pos = pClient->GetCurrentPosition(sWorld.GetArTime());
    auto res = NGemity::StringFormat("add_npc({}, {}, {}, {})", (int32_t)pos.GetPositionX(), (int32_t)pos.GetPositionY(), i, cnt);
    sScriptingMgr.RunString(pClient, res);
}

void AllowedCommandInfo::onCheatCreateParty(Player *pClient, const std::string &partyName)
{
    if (pClient->GetPartyID() != 0)
        return;

    auto partyID = sGroupManager.CreateParty(pClient, partyName, PARTY_TYPE::TYPE_NORMAL_PARTY);
    if (partyID == -1)
    {
        NG_LOG_ERROR("group", "Player wasn't able to create party - %u, %s", pClient->GetPartyID(), pClient->GetName());
        return;
    }
    pClient->SetInt32Value(PLAYER_FIELD_PARTY_ID, partyID);
    Messages::SendChatMessage(100, "@PARTY", pClient, NGemity::StringFormat("CREATE|{}|{}|", partyName, pClient->GetName()));
    Messages::SendPartyInfo(pClient);
}

void AllowedCommandInfo::onInviteParty(Player *pClient, const std::string &szPlayer)
{
    if (pClient->GetPartyID() == 0)
        return;

    auto player = Player::FindPlayer(szPlayer);
    if (player == nullptr)
        return;

    auto partyname = sGroupManager.GetPartyName(pClient->GetPartyID());
    if (partyname.empty())
        return;

    auto nPW = sGroupManager.GetPassword(pClient->GetPartyID());

    Messages::SendChatMessage(100, "@PARTY", player, NGemity::StringFormat("INVITE|{}|{}|{}|{}", pClient->GetName(), partyname, pClient->GetPartyID(), nPW));
}

void AllowedCommandInfo::onJoinParty(Player *pClient, const std::string &args)
{
    if (pClient == nullptr)
        return;
    if (pClient->GetPartyID() != 0)
    {
        Messages::SendChatMessage(100, "@PARTY", pClient, "ERROR_YOU_CAN_JOIN_ONLY_ONE_PARTY");
        return;
    }
    Tokenizer tokenizer(args, ' ');
    if (tokenizer.size() != 2)
        return;

    if (!isMXNumeric(tokenizer[0]) || !isMXNumeric(tokenizer[1]))
        return;

    int32_t partyID = std::stoi(tokenizer[0]);
    uint32_t partyPW = (uint32_t)std::stoi(tokenizer[1]);

    if (!sGroupManager.JoinParty(partyID, pClient, partyPW))
    {
        NG_LOG_ERROR("group", "JoinParty failed!");
        Messages::SendChatMessage(100, "@PARTY", pClient, "HAS_NO_AUTHORITY");
        return;
    }
    Messages::SendPartyChatMessage(100, "@PARTY", partyID, NGemity::StringFormat("JOIN|{}|", sGroupManager.GetPartyName(pClient->GetPartyID())));
    Messages::SendPartyInfo(pClient);
    Messages::BroadcastPartyMemberInfo(pClient);
}

void AllowedCommandInfo::onPartyInfo(Player *pClient, const std::string &)
{
    Messages::SendPartyInfo(pClient);
}

void AllowedCommandInfo::onPartyDestroy(Player *pClient, const std::string &)
{
    if (pClient->GetPartyID() != 0 && !sGroupManager.IsLeader(pClient->GetPartyID(), pClient->GetName()))
        return;

    sGroupManager.DestroyParty(pClient->GetPartyID());
}

void AllowedCommandInfo::onCheatStandup(Player *pClient, const std::string &)
{
    if (pClient == nullptr)
        return;

    pClient->m_bSitdown = false;
    Messages::BroadcastStatusMessage(pClient);
}

void AllowedCommandInfo::onLeaveParty(Player *pClient, const std::string &)
{
    if (pClient == nullptr || pClient->GetPartyID() == 0)
        return;

    if (sGroupManager.LeaveParty(pClient->GetPartyID(), pClient->GetNameAsString()))
        pClient->SetUInt32Value(PLAYER_FIELD_PARTY_ID, 0);
}
