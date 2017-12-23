/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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

#include "RectangleF.h"

bool X2D::Rectf::IsInclude(float x, float y)
{
    float x1 = this->size.x + this->pos.x;
    float y1 = this->size.y + this->pos.y;

    return x >= this->pos.x && x < x1 && y >= this->pos.y && y < y1;
}

bool X2D::Rectf::IsInclude(Pointf p)
{
    float x1 = this->size.x + this->pos.x;
    float y1 = this->size.y + this->pos.y;

    return p.x >= this->pos.x && p.x < x1 && p.y >= this->pos.y && p.y < y1;
}

bool X2D::Rectf::Set(Pointf begin, Pointf end)
{
    this->pos.x  = begin.x;
    this->pos.y  = begin.y;
    this->size.x = end.x - begin.x;
    this->size.y = end.y - begin.y;
    return true;
}

X2D::RectangleF::RectangleF(X2D::Pointf p1, X2D::Pointf p2)
{
    m_TopLeft     = Pointf(p1.x, p1.y);
    m_BottomRight = Pointf(p2.x, p2.y);
    if (p2.x < p1.x) {
        m_TopLeft.x     = p2.x;
        m_BottomRight.x = p1.x;
    }
    if (p2.y < p1.y) {
        m_TopLeft.y     = p2.y;
        m_BottomRight.y = p1.y;
    }
}

X2D::RectangleF::RectangleF(std::vector<X2D::Pointf> points)
{
    Pointf p1 = points.front();
    m_TopLeft = Pointf(p1.x, p1.y);
    Pointf p2 = points.back();
    m_BottomRight = Pointf(p2.x, p2.y);
    for (auto &p : points) {
        if (p.x < m_TopLeft.x)
            m_TopLeft.x     = p.x;
        if (p.y < m_TopLeft.y)
            m_TopLeft.y     = p.y;
        if (p.x > m_BottomRight.x)
            m_BottomRight.x = p.x;
        if (p.y > m_BottomRight.y)
            m_BottomRight.y = p.y;
    }
}

bool X2D::RectangleF::IsInclude(float x, float y)
{
    return x >= this->m_TopLeft.x && x < this->m_BottomRight.x && y >= this->m_TopLeft.y && y < this->m_BottomRight.y;
}

bool X2D::RectangleF::IsInclude(X2D::Pointf p)
{
    return p.x >= this->m_TopLeft.x && p.x < this->m_BottomRight.x && p.y >= this->m_TopLeft.y && p.y < this->m_BottomRight.y;
}

bool X2D::RectangleF::IsCollision(X2D::Linef line)
{
    if (this->m_BottomRight.x - this->m_TopLeft.x == 0 || this->m_BottomRight.y - this->m_TopLeft.y == 0)
        return false;
    if (this->IsInclude((line.end.x + line.begin.x) / 2, (line.end.y + line.begin.y) / 2)
        || this->IsInclude(line.begin.x, line.begin.y)
        || this->IsInclude(line.end.x, line.end.y))
        return true;

    Pointf topBegin    = this->m_TopLeft;
    Pointf topEnd      = Pointf(this->m_BottomRight.x, this->m_TopLeft.y);
    Pointf bottomBegin = Pointf(this->m_TopLeft.x, this->m_BottomRight.y);
    Pointf bottomEnd   = Pointf(this->m_BottomRight.x, this->m_BottomRight.y);
    Pointf leftBegin   = Pointf(this->m_TopLeft.x, this->m_TopLeft.y);
    Pointf leftEnd     = Pointf(this->m_TopLeft.x, this->m_BottomRight.y);
    Pointf rightBegin  = Pointf(this->m_BottomRight.x, this->m_TopLeft.y);
    Pointf rightEnd    = Pointf(this->m_BottomRight.x, this->m_BottomRight.y);

    Linef::IntersectResult topResult    = Linef::IntersectCCW(line.begin, line.end, topBegin, topEnd);
    Linef::IntersectResult bottomResult = Linef::IntersectCCW(line.begin, line.end, bottomBegin, bottomEnd);
    Linef::IntersectResult leftResult   = Linef::IntersectCCW(line.begin, line.end, leftBegin, leftEnd);
    Linef::IntersectResult rightResult  = Linef::IntersectCCW(line.begin, line.end, rightBegin, rightEnd);

//             None = 157,
//             Intersect = 1,
//             Seperate = -1,
//             Touch = 0,

    if (topResult == Linef::IntersectResult::Intersect
        || bottomResult == Linef::IntersectResult::Intersect
        || leftResult == Linef::IntersectResult::Intersect
        || rightResult == Linef::IntersectResult::Intersect) {
        return true;
    }
    if (leftResult != Linef::IntersectResult::Touch) {
        if (topResult == Linef::IntersectResult::Touch) {
            return rightResult != Linef::IntersectResult::Touch;
        }
        return false;
    }
    return bottomResult != Linef::IntersectResult::Touch;
}
