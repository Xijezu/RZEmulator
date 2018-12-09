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

#include "QuadTreeMapInfo.h"

X2D::QuadTreeMapInfo::Node::Node(X2D::Pointf p1, X2D::Pointf p2, ushort depth)
{
    this->m_Area = RectangleF(p1, p2);
    m_unDepth = depth;
}

bool X2D::QuadTreeMapInfo::Node::Add(MapLocationInfo u)
{
    bool result;
    Node nn{ };
    nn.init = false;

    result = u.IsCollision(this->m_Area);
    if (result)
    {
        if (this->m_pNode.count(0) != 0)
            nn = this->getFitNode(u);
        if (this->m_pNode.count(0) == 0 || !nn.init || !nn.Add(u))
            this->add(u);
        result = true;
    }
    return result;
}

void X2D::QuadTreeMapInfo::Node::Enum(X2D::Pointf c, X2D::QuadTreeMapInfo::FunctorAdaptor &f)
{
    if (this->m_Area.IsInclude(c.x, c.y))
    {
        if (this->m_pNode.count(0) != 0)
        {
            this->m_pNode[0].Enum(c, f);
            this->m_pNode[1].Enum(c, f);
            this->m_pNode[2].Enum(c, f);
            this->m_pNode[3].Enum(c, f);
        }

        for (auto &info : m_vList)
        {
            if (info.IsInclude(c))
            {
                f.pResult.push_back(info);
            }
        }
    }
}

bool X2D::QuadTreeMapInfo::Node::Collision(X2D::Pointf c)
{
    if (this->m_Area.IsInclude(c))
    {
        for (auto &p : this->m_vList)
        {
            if (p.IsInclude(c))
                return true;
        }

        if (this->m_pNode.count(0) != 0
            && (this->m_pNode[0].Collision(c)
                || this->m_pNode[1].Collision(c)
                || this->m_pNode[2].Collision(c)
                || this->m_pNode[3].Collision(c)))
            return true;
    }
    return false;
}

bool X2D::QuadTreeMapInfo::Node::LooseCollision(X2D::Linef pLine)
{
    if (this->m_Area.IsCollision(pLine))
    {
        for (auto &p : this->m_vList)
        {
            if (p.IsLooseCollision(pLine))
                return true;
        }

        if (this->m_pNode.count(0) != 0
            && (this->m_pNode[0].LooseCollision(pLine)
                || this->m_pNode[1].LooseCollision(pLine)
                || this->m_pNode[2].LooseCollision(pLine)
                || this->m_pNode[3].LooseCollision(pLine)))
            return true;
    }
    return false;
}

X2D::QuadTreeMapInfo::Node X2D::QuadTreeMapInfo::Node::getFitNode(MapLocationInfo u)
{
    if (u.IsIn(this->m_pNode[0].m_Area))
        return this->m_pNode[0];
    if (u.IsIn(this->m_pNode[1].m_Area))
        return this->m_pNode[1];
    if (u.IsIn(this->m_pNode[2].m_Area))
        return this->m_pNode[2];
    if (u.IsIn(this->m_pNode[3].m_Area))
        return this->m_pNode[3];
    Node node{ };
    node.init = false;
    return node;
}

void X2D::QuadTreeMapInfo::Node::add(MapLocationInfo u)
{
    this->m_vList.emplace_back(u);
    if (this->m_vList.size() >= 10)
    {
        if (this->m_unDepth < 10)
            this->divide();
    }
}

void X2D::QuadTreeMapInfo::Node::divide()
{
    if (this->m_pNode.count(0) == 0)
    {
        Pointf p1{ };
        Pointf p2{ };

        float easx      = ((this->m_Area.m_BottomRight.x - this->m_Area.m_TopLeft.x) * 0.5f) + this->m_Area.m_TopLeft.x;
        float easy      = ((this->m_Area.m_BottomRight.y - this->m_Area.m_TopLeft.y) * 0.5f) + this->m_Area.m_TopLeft.y;
        auto  new_depth = (ushort)(this->m_unDepth + 1);

        p1 = Pointf(this->m_Area.m_TopLeft.x, this->m_Area.m_TopLeft.y);
        p2 = Pointf(this->m_Area.m_TopLeft.x + easx, this->m_Area.m_TopLeft.y + easy);
        this->m_pNode[0] = Node(p1, p2, new_depth);

        p1 = Pointf(this->m_Area.m_TopLeft.x + easx, this->m_Area.m_TopLeft.y);
        p1 = Pointf(this->m_Area.m_BottomRight.x, this->m_Area.m_TopLeft.y + easy);
        this->m_pNode[1] = Node(p1, p2, new_depth);

        p1 = Pointf(this->m_Area.m_TopLeft.x, this->m_Area.m_TopLeft.y + easx);
        p2 = Pointf(this->m_Area.m_TopLeft.x + easx, this->m_Area.m_BottomRight.y);
        this->m_pNode[2] = Node(p1, p2, new_depth);

        p1 = Pointf(this->m_Area.m_TopLeft.x + easx, this->m_Area.m_TopLeft.y + easx);
        p2 = Pointf(this->m_Area.m_BottomRight.x, this->m_Area.m_BottomRight.y);
        this->m_pNode[3] = Node(p1, p2, new_depth);

        std::vector<MapLocationInfo> nl{ };

        for (auto &info: this->m_vList)
        {
            Node fn = this->getFitNode(info);
            if (fn.init)
            {
                fn.Add(info);
            }
            else
            {
                nl.emplace_back(info);
            }
        }

        this->m_vList.clear();
        this->m_vList = nl;
    }
}

X2D::QuadTreeMapInfo::QuadTreeMapInfo(float width, float height)
{
    Pointf p1 = Pointf(0, 0);
    Pointf p2 = Pointf(width, height);
    m_Area       = RectangleF(p1, p2);
    m_MasterNode = Node(p1, p2, 0);
}

void X2D::QuadTreeMapInfo::Enum(X2D::Pointf c, X2D::QuadTreeMapInfo::FunctorAdaptor &f)
{
    this->m_MasterNode.Enum(c, f);
}

bool X2D::QuadTreeMapInfo::Add(MapLocationInfo u)
{
    return m_MasterNode.Add(u);
}

bool X2D::QuadTreeMapInfo::Collision(X2D::Pointf c)
{
    return m_MasterNode.Collision(c);
}
