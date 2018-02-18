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

#ifndef NGEMITY_FUNCTORS_H
#define NGEMITY_FUNCTORS_H

#include "Common.h"
#include "Object.h"
class Region;
class Player;
class Unit;

#include "XPacket.h"

/*
 * Note: I'm not a fan of this, either
 * Previously I was using lambdas instead of this
 * but due to changes to the Region class I decided to simply use this
 * Apparentely lambdas seem to be pretty slow compared to abstract classes
 * depending on the scale of this project we don't really want to use
 * performance hitting stuff just for being easy or "good to read"
 *
 * However, some day this needs a revamp. How? I don't know.
 * But it is annoying.
 */

struct WorldObjectFunctor
{
    virtual void Run(WorldObject* obj) { };
};

struct RegionFunctor
{
    virtual void Run(Region*) { };
};
struct BroadcastStatusMessageObjectFunctor : public WorldObjectFunctor
{
    WorldObject *pObject{nullptr};
    void Run(WorldObject* obj) override;
};

struct BroadcastStatusRegionFunctor : public RegionFunctor
{
    BroadcastStatusMessageObjectFunctor fn;
    void Run(Region *region) override;
};


struct DoEachClientRegionFunctor : public RegionFunctor
{
    WorldObjectFunctor pFo;
    void Run(Region* region) override;
};

struct DoEachMovableRegionFunctor : public RegionFunctor
{
    WorldObjectFunctor pFo;
    void Run(Region* region) override;
};

struct DoEachStaticRegionFunctor : public RegionFunctor
{
    WorldObjectFunctor pFo;
    void Run(Region* region) override;
};

struct SendNPCStatusInVisibleRangeFunctor : public WorldObjectFunctor
{
    Player *player{nullptr};
    void Run(WorldObject* obj) override;
};

struct SendNPCStatusRegionFunctor : public RegionFunctor
{
    SendNPCStatusInVisibleRangeFunctor fn;
    void Run(Region *pRegion) override;
};

struct BroadcastFunctor : public WorldObjectFunctor
{
    XPacket packet;
    void Run(WorldObject* obj) override;
};

struct BroadcastRegionFunctor : public RegionFunctor
{
    BroadcastFunctor fn;
    void Run(Region* region) override;
};

struct KillAllDoableObject : public WorldObjectFunctor
{
    Player *p{nullptr};
    void Run(WorldObject *obj) override;
};

struct KillALlRegionFunctor : public RegionFunctor
{
    KillAllDoableObject fn;
    void Run(Region *region) override;
};

struct SendEnterMessageEachOtherFunctor : public WorldObjectFunctor
{
    WorldObject *obj{nullptr};
    void Run(WorldObject* obj) override;
};

struct SendEnterMessageFunctor : public WorldObjectFunctor
{
    Player *obj{nullptr};
    void Run(WorldObject* obj) override;
};

struct AddObjectRegionFunctor : public RegionFunctor
{
    WorldObject *newObj{nullptr};
    bool        bSend{false};
    void Run(Region* region) override;
};

struct SendMoveMessageFunctor : public WorldObjectFunctor
{
    Unit *obj{nullptr};
    void Run(WorldObject* client) override;
};

struct SetMoveFunctor : public RegionFunctor
{
    uint nCnt{0};
    Unit *obj{nullptr};
    void Run(Region* region) override;
};

struct EnumMovableObjectRegionFunctor : public RegionFunctor
{
    std::vector<uint> &pvResult;
    uint t;
    Position pos;
    float left;
    float right;
    float top;
    float bottom;
    float range;
    bool bIncludeClient;
    bool bIncludeNPC;


    struct SubFunctor : public WorldObjectFunctor
    {
        SubFunctor() : pParent(nullptr) { }
        void Run(WorldObject* obj) override;

        EnumMovableObjectRegionFunctor *pParent;
    };

    EnumMovableObjectRegionFunctor(std::vector<uint>& _pvResult, Position _pos, float _range, bool _bIncludeClient, bool _bIncludeNPC);
    void Run(Region *region) override;
};


#endif // NGEMITY_FUNCTORS_H
