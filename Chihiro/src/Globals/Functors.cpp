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

#include "Functors.h"
#include "Player.h"
#include "RegionContainer.h"
#include "Messages.h"
#include "ClientPackets.h"

void DoEachClientRegionFunctor::Run(Region *region)
{
    if(region != nullptr)
    {
        region->DoEachClient(pFo);
    }
}


void DoEachMovableRegionFunctor::Run(Region *region)
{
    if(region != nullptr)
    {
        region->DoEachMovableObject(pFo);
    }
}

void DoEachStaticRegionFunctor::Run(Region *region)
{
    if(region != nullptr)
    {
        region->DoEachStaticObject(pFo);
    }
}

void BroadcastStatusMessageObjectFunctor::Run(WorldObject *obj)
{
    XPacket statusMsg(TS_SC_STATUS_CHANGE);
    statusMsg << pObject->GetHandle();
    statusMsg << Messages::GetStatusCode(pObject, dynamic_cast<Player*>(obj));
    dynamic_cast<Player*>(obj)->SendPacket(statusMsg);
}

void SendNPCStatusInVisibleRangeFunctor::Run(WorldObject *obj)
{
    if(obj != nullptr && obj->IsNPC())
    {
        XPacket statusMsg(TS_SC_STATUS_CHANGE);
        statusMsg << obj->GetHandle();
        statusMsg << Messages::GetStatusCode(obj, player);
        player->SendPacket(statusMsg);
    }
}

void BroadcastFunctor::Run(WorldObject *obj)
{
    if(obj != nullptr && obj->IsPlayer())
    {
        dynamic_cast<Player*>(obj)->SendPacket(packet);
    }
}

void SendEnterMessageEachOtherFunctor::Run(WorldObject *client)
{
    if(client != nullptr && client->GetHandle() != obj->GetHandle())
    {
        Messages::sendEnterMessage(dynamic_cast<Player*>(client), obj, false);
        if(obj->GetObjType() == 2)
            Messages::sendEnterMessage(dynamic_cast<Player*>(obj), client, false);
    }
}

void AddObjectRegionFunctor::Run(Region *region)
{
    SendEnterMessageEachOtherFunctor fn;
    fn.obj = newObj;
    region->DoEachClient(fn);

    if(newObj->GetObjType() == 2)
    {
        SendEnterMessageFunctor fn2;
        fn2.obj = dynamic_cast<Player*>(newObj);
        region->DoEachStaticObject(fn2);
        region->DoEachMovableObject(fn2);
    }
}

void SendEnterMessageFunctor::Run(WorldObject *client)
{
    Messages::sendEnterMessage(obj, client, false);
}

void SetMoveFunctor::Run(Region *region)
{
    SendMoveMessageFunctor fn;
    fn.obj = obj;
    nCnt += region->DoEachClient(fn);
}

void SendMoveMessageFunctor::Run(WorldObject *client)
{
    Messages::SendMoveMessage(dynamic_cast<Player*>(client), obj);
}

void BroadcastRegionFunctor::Run(Region *region)
{
    region->DoEachClient(fn);
}
