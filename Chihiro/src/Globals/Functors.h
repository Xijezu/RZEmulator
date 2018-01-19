#ifndef PROJECT_FUNCTORS_H
#define PROJECT_FUNCTORS_H

#include "Common.h"
class WorldObject;
class Region;
class Player;
class Unit;

#include "XPacket.h"

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
    WorldObject* pObject;
    void Run(WorldObject* obj) override;
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
    Player* player;
    void Run(WorldObject* obj) override;
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

struct SendEnterMessageEachOtherFunctor : public WorldObjectFunctor
{
    WorldObject* obj;
    void Run(WorldObject* obj) override;
};

struct SendEnterMessageFunctor : public WorldObjectFunctor
{
    Player* obj;
    void Run(WorldObject* obj) override;
};

struct AddObjectRegionFunctor : public RegionFunctor
{
    WorldObject* newObj;
    void Run(Region* region) override;
};

struct SendMoveMessageFunctor : public WorldObjectFunctor
{
    Unit* obj;
    void Run(WorldObject* client) override;
};

struct SetMoveFunctor : public RegionFunctor
{
    uint nCnt{0};
    Unit* obj;
    void Run(Region* region) override;
};


#endif // PROJECT_FUNCTORS_H
