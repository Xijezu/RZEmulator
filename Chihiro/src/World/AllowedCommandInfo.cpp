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

typedef struct AllowedCommands {
    std::string szCommand;
    bool bNeedsPermission;
    void (AllowedCommandInfo::*handler)(Player*,const std::string&);
} GameHandler;

const GameHandler commandHandler[] =
        {
                { "/run", true, &AllowedCommandInfo::onRunScript },
                { "/sitdown", false, &AllowedCommandInfo::onCheatSitdown },
                { "/position", false, &AllowedCommandInfo::onCheatPosition},
                { "/battle", false, &AllowedCommandInfo::onBattleMode },
                { "/notice", true, &AllowedCommandInfo::onCheatNotice },
                { "/plist", false, &AllowedCommandInfo::onCheatParty },
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
        if(i.szCommand == tokenizer[0]) {
            auto pos = szMessage.find(' ');
            if(pos != std::string::npos)
                (*this.*i.handler)(pClient, szMessage.substr(pos+1));
            else
                (*this.*i.handler)(pClient,"");
        }
    }
}
