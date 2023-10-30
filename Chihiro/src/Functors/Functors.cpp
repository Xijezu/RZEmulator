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

#include "Functors.h"

#include "Messages.h"
#include "Monster.h"
#include "RegionContainer.h"
#include "World.h"

void SendEnterMessageEachOtherFunctor::Run(RegionType &regionType)
{
    for (const auto &client : regionType) {
        if (client != nullptr && client->GetHandle() != obj->GetHandle()) {
            Messages::sendEnterMessage(dynamic_cast<Player *>(client), obj, false);
            if (obj->IsPlayer())
                Messages::sendEnterMessage(dynamic_cast<Player *>(obj), client, false);
            bSent = true;
        }
    }
}

void SendEnterMessageFunctor::Run(RegionType &regionType)
{
    for (const auto &client : regionType) {
        Messages::sendEnterMessage(obj, client, false);
        if (obj->IsMonster()) {
            obj->As<Monster>()->m_bNearClient = true;
        }
    }
}

void AddObjectFunctor::Run()
{
    SendEnterMessageEachOtherFunctor fn;
    fn.obj = newObj;

    sRegion.DoEachVisibleRegion(x, y, layer, NG_REGION_FUNCTOR(fn), (uint8_t)RegionVisitor::ClientVisitor);
    if (fn.bSent)
        bSend = true;

    if (newObj->IsPlayer()) {
        SendEnterMessageFunctor fn2;
        fn2.obj = newObj->As<Player>();
        sRegion.DoEachVisibleRegion(x, y, layer, NG_REGION_FUNCTOR(fn2), (uint8_t)RegionVisitor::MovableVisitor | (uint8_t)RegionVisitor::StaticVisitor);
    }
}

void AddObjectFunctor::Run2()
{
    SendEnterMessageEachOtherFunctor fn;
    fn.obj = newObj;

    sRegion.DoEachNewRegion(x, y, x2, y2, layer, NG_REGION_FUNCTOR(fn), (uint8_t)RegionVisitor::ClientVisitor);
    if (fn.bSent)
        bSend = true;

    if (newObj->IsPlayer()) {
        SendEnterMessageFunctor fn2;
        fn2.obj = newObj->As<Player>();
        sRegion.DoEachNewRegion(x, y, x2, y2, layer, NG_REGION_FUNCTOR(fn2), (uint8_t)RegionVisitor::MovableVisitor | (uint8_t)RegionVisitor::StaticVisitor);
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
    Messages::SendMoveMessage(dynamic_cast<Player *>(client), obj);
}

void EnumMovableObjectRegionFunctor::SubFunctor::Run(WorldObject *obj)
{

    auto c_pos = obj->GetCurrentPosition(pParent->t);
    if (c_pos.GetPositionX() >= pParent->left && c_pos.GetPositionX() <= pParent->right && c_pos.GetPositionY() >= pParent->top && c_pos.GetPositionY() <= pParent->bottom &&
        pParent->range > c_pos.GetExactDist2d(&pParent->pos)) {
        pParent->pvResult.emplace_back(obj->GetHandle());
    }
}

EnumMovableObjectRegionFunctor::EnumMovableObjectRegionFunctor(std::vector<uint32_t> &_pvResult, Position _pos, float _range, bool _bIncludeClient, bool _bIncludeNPC)
    : pvResult(_pvResult)
    , pos(_pos)
    , range(_range)
    , bIncludeClient(_bIncludeClient)
    , bIncludeNPC(_bIncludeNPC)
{
    left = pos.GetPositionX() - range;
    right = pos.GetPositionX() + range;
    top = pos.GetPositionY() - range;
    bottom = pos.GetPositionY() + range;
    t = sWorld.GetArTime();
}

void EnumMovableObjectRegionFunctor::Run(Region *region)
{
    if (region == nullptr)
        return;

    SubFunctor fn{};
    fn.pParent = this;
    if (bIncludeClient)
        region->DoEachClient(fn);
    if (bIncludeNPC)
        region->DoEachMovableObject(fn);
}
