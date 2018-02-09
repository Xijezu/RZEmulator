#ifndef PROJECT_FUNCTORS_H
#define PROJECT_FUNCTORS_H

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
    WorldObject* pObject;
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
    Player* player;
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
    bool bSend{false};
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


#endif // PROJECT_FUNCTORS_H
