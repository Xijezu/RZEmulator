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

#include "AllowedCommandInfo.h"
#include "World.h"
#include "Messages.h"
#include "Scripting/XLua.h"
#include "MemPool.h"
#include "GroupManager.h"
#include "RegionContainer.h"

typedef struct AllowedCommands {
    std::string szCommand;
    bool bNeedsPermission;
    void (AllowedCommandInfo::*handler)(Player*,const std::string&);
} GameHandler;

const AllowedCommands commandHandler[] =
                              {
                                      {"/run",        true,  &AllowedCommandInfo::onRunScript},
                                      {"/sitdown",    false, &AllowedCommandInfo::onCheatSitdown},
                                      {"/position",   false, &AllowedCommandInfo::onCheatPosition},
                                      {"/battle",     false, &AllowedCommandInfo::onBattleMode},
                                      {"/notice",     true,  &AllowedCommandInfo::onCheatNotice},
                                      {"/suicide",    true,  &AllowedCommandInfo::onCheatSuicide},
                                      {"/doit",       true,  &AllowedCommandInfo::onCheatKillAll},
                                      {"/pcreate",    false, &AllowedCommandInfo::onCheatCreateParty},
                                      {"/regenerate", true,  &AllowedCommandInfo::onCheatRespawn},
                                      {"/pinvite",    false, &AllowedCommandInfo::onInviteParty},
                                      {"/pjoin",      false, &AllowedCommandInfo::onJoinParty},
                                      {"/plist",      false, &AllowedCommandInfo::onPartyInfo},
                                      {"/pdestroy",   false, &AllowedCommandInfo::onPartyDestroy}
                              };

const int tableSize = (sizeof(commandHandler) / sizeof(AllowedCommands));

void AllowedCommandInfo::onCheatPosition(Player *pClient, const std::string&/* tokens*/)
{
    auto pos = pClient->GetCurrentPosition(sWorld->GetArTime());
    auto message = string_format("<BR>X: %u, Y: %u, Layer: %u<BR>", (int)pos.GetPositionX(), (int)pos.GetPositionY(), (int)pos.GetLayer());
    Messages::SendChatMessage(30, "@SYSTEM", pClient, message);
}

void AllowedCommandInfo::onRunScript(Player *pClient, const std::string &pScript)
{
    sScriptingMgr->RunString(pClient, pScript);
}

void AllowedCommandInfo::onCheatSitdown(Player *pClient, const std::string&/* tokens*/)
{

}

void AllowedCommandInfo::onBattleMode(Player *pClient, const std::string& tokens) {
    try {
        auto target_handle = (uint)std::stoul(tokens);
        auto unit = sMemoryPool->GetObjectInWorld<Unit>(target_handle);
        if (unit != nullptr)
            Messages::BroadcastStatusMessage(unit);
    } catch (std::exception &ex) {
        MX_LOG_ERROR("world", "AllowedCommandInfo::onBattleMode: Exception thrown!");
    }
}

void AllowedCommandInfo::onCheatNotice(Player *pClient, const std::string &)
{

}

void AllowedCommandInfo::onCheatParty(Player *pClient, const std::string &)
{

}

void AllowedCommandInfo::Run(Player *pClient, const std::string &szMessage)
{
    Tokenizer tokenizer(szMessage, ' ');
    for (const auto &i : commandHandler) {
        if(i.szCommand == tokenizer[0] && (!i.bNeedsPermission || (i.bNeedsPermission && pClient->GetPermission() >= 100))) {
            auto pos = szMessage.find(' ');
            if(pos != std::string::npos)
                (*this.*i.handler)(pClient, szMessage.substr(pos+1));
            else
                (*this.*i.handler)(pClient,"");
        }
    }
}
void AllowedCommandInfo::onCheatSuicide(Player */*pClient*/, const std::string &/*szMessage*/)
{
    World::StopNow(SHUTDOWN_EXIT_CODE);
}

void AllowedCommandInfo::onCheatKillAll(Player *pClient, const std::string &)
{
/*    sArRegion->DoEachVisibleRegion((uint)pClient->GetPositionX() / g_nRegionSize, (uint)(pClient->GetPositionY() / g_nRegionSize), pClient->GetLayer()
                                   [=](ArRegion* region) {
                                       region->DoEachMovableObject(
                                               [=](WorldObject* obj) {
                                                   if(obj->GetSubType() == ST_Mob) {
                                                       dynamic_cast<Monster*>(obj)->ForceKill(pClient);
                                                   }
                                               }
                                       );
                                   });*/
    // Causes deadlock

    KillALlRegionFunctor fn;
    KillAllDoableObject fn2;
    fn2.p = pClient;
    fn.fn = fn2;

    sRegion->DoEachVisibleRegion((uint)pClient->GetPositionX() / g_nRegionSize, (uint)(pClient->GetPositionY() / g_nRegionSize), pClient->GetLayer(), fn);

}

void AllowedCommandInfo::onCheatRespawn(Player *pClient, const std::string& str)
{
    int cnt = 1;
    Tokenizer tokens(str, ' ');
    if(tokens.size() < 1)
        return;
    auto i = std::stoi(tokens[0]);
    if(tokens.size() == 2)
        cnt = std::stoi(tokens[1]);
    auto pos = pClient->GetCurrentPosition(sWorld->GetArTime());
    auto res = string_format("add_npc(%d, %d, %d, %d)", (int)pos.GetPositionX(), (int)pos.GetPositionY(),i, cnt);
    sScriptingMgr->RunString(pClient, res);
}

void AllowedCommandInfo::onCheatCreateParty(Player *pClient, const std::string &partyName)
{
    if(pClient->GetPartyID() != 0)
        return;

    auto partyID = sGroupManager->CreateParty(pClient, partyName, PARTY_TYPE::TYPE_NORMAL_PARTY);
    if(partyID == -1)
    {
        MX_LOG_ERROR("group", "Player wasn't able to create party - %d, %s", pClient->GetPartyID(), pClient->GetName());
        return;
    }
    pClient->SetInt32Value(PLAYER_FIELD_PARTY_ID, partyID);
    Messages::SendChatMessage(100, "@PARTY", pClient, string_format("CREATE|%s|%s|", partyName.c_str(), pClient->GetName()));
    Messages::SendPartyInfo(pClient);
}

void AllowedCommandInfo::onInviteParty(Player *pClient, const std::string &szPlayer)
{
    if(pClient->GetPartyID() == 0)
        return;

    auto player = Player::FindPlayer(szPlayer);
    if(player == nullptr)
        return;

    auto partyname = sGroupManager->GetPartyName(pClient->GetPartyID());
    if(partyname.empty())
        return;

    auto nPW = sGroupManager->GetPassword(pClient->GetPartyID());

    Messages::SendChatMessage(100, "@PARTY", player, string_format("INVITE|%s|%s|%d|%d", pClient->GetName(), partyname.c_str(), pClient->GetPartyID(), nPW));
}

void AllowedCommandInfo::onJoinParty(Player *pClient, const std::string &args)
{
    if(pClient == nullptr)
        return;
    if(pClient->GetPartyID() != 0)
    {
        Messages::SendChatMessage(100, "@PARTY", pClient, "ERROR_YOU_CAN_JOIN_ONLY_ONE_PARTY");
        return;
    }
    Tokenizer tokenizer(args, ' ');
    if(tokenizer.size() != 2)
        return;

    if(!isNumeric(tokenizer[0]) || !isNumeric(tokenizer[1]))
        return;

    int partyID = std::stoi(tokenizer[0]);
    uint partyPW = (uint)std::stoi(tokenizer[1]);

    if(!sGroupManager->JoinParty(partyID, pClient, partyPW))
    {
        MX_LOG_ERROR("group", "JoinParty failed!");
        Messages::SendChatMessage(100, "@PARTY", pClient, "HAS_NO_AUTHORITY");
        return;
    }
    Messages::SendPartyInfo(pClient);
}

void AllowedCommandInfo::onPartyInfo(Player *pClient, const std::string &)
{
    Messages::SendPartyInfo(pClient);
}

void AllowedCommandInfo::onPartyDestroy(Player *pClient, const std::string &)
{
    if(pClient->GetPartyID() != 0 && !sGroupManager->IsLeader(pClient->GetPartyID(), pClient->GetName()))
        return;

    sGroupManager->DestroyParty(pClient->GetPartyID());
}
