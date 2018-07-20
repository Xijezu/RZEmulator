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

#pragma once

#include "Common.h"

class Player;
class AllowedCommandInfo
{
    public:
        static AllowedCommandInfo &Instance()
        {
            static AllowedCommandInfo instance;
            return instance;
        }
        ~AllowedCommandInfo() = default;

        void Run(Player *pClient, const std::string &szMessage);
        void onCheatPosition(Player *pClient, const std::string &);
        void onRunScript(Player *pClient, const std::string &pScript);
        void onCheatSitdown(Player *pClient, const std::string &);
        void onCheatStandup(Player *pClient, const std::string &);
        void onBattleMode(Player *pClient, const std::string &);
        void onCheatNotice(Player *pClient, const std::string &);
        void onCheatParty(Player *pClient, const std::string &);
        void onCheatSuicide(Player *pClient, const std::string &);
        void onCheatKillAll(Player *pClient, const std::string &);
        void onCheatRespawn(Player *pClient, const std::string &);
        void onCheatCreateParty(Player *pClient, const std::string &);
        void onInviteParty(Player *pClient, const std::string &);
        void onJoinParty(Player *pClient, const std::string &);
        void onPartyInfo(Player *pClient, const std::string &);
        void onLeaveParty(Player *pClient, const std::string &);
        void onPartyDestroy(Player *pClient, const std::string &);
    protected:
        AllowedCommandInfo() = default;
};

#define sAllowedCommandInfo AllowedCommandInfo::Instance()