#pragma once
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
#include "Common.h"
#include "Object.h"
#include "Player.h"

using RegionType = std::vector<WorldObject *>;
class Region;
class Unit;
class Skill;

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
    virtual void Run(WorldObject *obj){};
};

struct RegionFunctor
{
    virtual void Run(Region *){};
};

struct SkillFunctor
{
    virtual void onSkill(const Skill *pSkill) {}
};

template <typename T>
struct BroadcastFunctor
{
    T packet;

    void Run(RegionType &list)
    {
        for (const auto &obj : list)
        {
            if (obj != nullptr && obj->IsPlayer())
            {
                obj->As<Player>()->SendPacket(packet);
            }
        }
    }
};

struct SendEnterMessageEachOtherFunctor
{
    WorldObject *obj{nullptr};
    bool bSent{false};
    void Run(RegionType &client);
};

struct SendEnterMessageFunctor
{
    Player *obj{nullptr};
    void Run(RegionType &regionType);
};

struct AddObjectFunctor
{
    explicit AddObjectFunctor(uint32_t _x, uint32_t _y, uint8_t _layer, WorldObject *obj)
        : newObj(obj), bSend(false), x(_x), y(_y), x2(0), y2(0), layer(_layer) {}

    explicit AddObjectFunctor(uint32_t _x, uint32_t _y, uint32_t _x2, uint32_t _y2, uint8_t _layer, WorldObject *obj)
        : newObj(obj), bSend(false), x(_x), y(_y), x2(_x2), y2(_y2), layer(_layer) {}

    void Run();
    void Run2();

    WorldObject *newObj;

    bool bSend;
    uint32_t x;
    uint32_t y;
    uint32_t x2;
    uint32_t y2;
    uint8_t layer;
};

struct SendMoveMessageFunctor : public WorldObjectFunctor
{
    Unit *obj{nullptr};
    void Run(WorldObject *client) override;
};

struct SetMoveFunctor : public RegionFunctor
{
    uint32_t nCnt{0};
    Unit *obj{nullptr};
    void Run(Region *region) override;
};

struct EnumMovableObjectRegionFunctor : public RegionFunctor
{
    std::vector<uint32_t> &pvResult;
    uint32_t t;
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
        SubFunctor() : pParent(nullptr) {}

        void Run(WorldObject *obj) override;

        EnumMovableObjectRegionFunctor *pParent;
    };

    EnumMovableObjectRegionFunctor(std::vector<uint32_t> &_pvResult, Position _pos, float _range, bool _bIncludeClient, bool _bIncludeNPC);
    void Run(Region *region) override;
};