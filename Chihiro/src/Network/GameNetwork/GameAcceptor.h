/*
  *  Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
  *  Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  *  Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
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

#ifndef __GAMEACCEPTOR_H__
#define __GAMEACCEPTOR_H__

#include "GameHandler.h"
#include "Server/XSocket.h"

#include <ace/Acceptor.h>
#include <ace/SOCK_Acceptor.h>

class GameAcceptor : public ACE_Acceptor<XSocket, ACE_SOCK_Acceptor>
{
public:
    GameAcceptor(void) { }
    virtual ~GameAcceptor(void)
    {
        if (reactor())
            reactor()->cancel_timer(this, 1);
    }

protected:
    virtual int make_svc_handler(XSocket* &sh)
    {
        if (sh == nullptr)
            ACE_NEW_RETURN(sh, XSocket, -1);

        sh->reactor(reactor());
        sh->set_session(new GameSession(sh));
        return 0;
    }

    virtual int handle_timeout(const ACE_Time_Value& /*current_time*/, const void* /*act = 0*/)
    {
        //sLog->outBasic("Resuming acceptor");
        reactor()->cancel_timer(this, 1);
        return reactor()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK);
    }

    virtual int handle_accept_error(void)
    {
#if defined(ENFILE) && defined(EMFILE)
        if (errno == ENFILE || errno == EMFILE)
        {
            //sLog->outError("Out of file descriptors, suspending incoming connections for 10 seconds");
            reactor()->remove_handler(this, ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::DONT_CALL);
            reactor()->schedule_timer(this, nullptr, ACE_Time_Value(10));
        }
#endif
        return 0;
    }
};

#endif
