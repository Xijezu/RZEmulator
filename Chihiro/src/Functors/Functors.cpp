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
#include "Monster.h"
#include "World.h"

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
        if(obj->IsPlayer())
            Messages::sendEnterMessage(dynamic_cast<Player*>(obj), client, false);
    }
}

void AddObjectRegionFunctor::Run(Region *region)
{
    SendEnterMessageEachOtherFunctor fn;
    fn.obj = newObj;
    if (region->DoEachClient(fn) != 0)
        bSend = true;

    if(newObj->IsPlayer())
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
    if (obj->IsMonster())
    {
        obj->As<Monster>()->m_bNearClient = true;
    }
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

void KillAllDoableObject::Run(WorldObject *obj)
{
    if(obj != nullptr && p != nullptr && obj->IsMonster())
        dynamic_cast<Monster*>(obj)->TriggerForceKill(p);
}

void KillALlRegionFunctor::Run(Region *region)
{
    region->DoEachMovableObject(fn);
}

void BroadcastStatusRegionFunctor::Run(Region *region)
{
    region->DoEachClient(fn);
}

void SendNPCStatusRegionFunctor::Run(Region *pRegion)
{
    pRegion->DoEachMovableObject(fn);
}

void EnumMovableObjectRegionFunctor::SubFunctor::Run(WorldObject *obj)
{

    auto c_pos = obj->GetCurrentPosition(pParent->t);
    if (c_pos.GetPositionX() >= pParent->left && c_pos.GetPositionX() <= pParent->right
        && c_pos.GetPositionY() >= pParent->top && c_pos.GetPositionY() <= pParent->bottom
        && pParent->range > c_pos.GetExactDist2d(&pParent->pos))
    {
        pParent->pvResult.emplace_back(obj->GetHandle());
    }

}

EnumMovableObjectRegionFunctor::EnumMovableObjectRegionFunctor(std::vector<uint> &_pvResult, Position _pos, float _range, bool _bIncludeClient, bool _bIncludeNPC)
    : pvResult(_pvResult), pos(_pos), range(_range), bIncludeClient(_bIncludeClient), bIncludeNPC(_bIncludeNPC)
{
    left = pos.GetPositionX() - range;
    right = pos.GetPositionX() + range;
    top = pos.GetPositionY() - range;
    bottom = pos.GetPositionY() + range;
    t = sWorld->GetArTime();
}

void EnumMovableObjectRegionFunctor::Run(Region *region)
{
    if(region == nullptr)
        return;

    SubFunctor fn{};
    fn.pParent = this;
    if(bIncludeClient)
        region->DoEachClient(fn);
    if(bIncludeNPC)
        region->DoEachMovableObject(fn);
}
