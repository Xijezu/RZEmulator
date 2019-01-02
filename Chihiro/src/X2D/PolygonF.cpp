/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "PolygonF.h"

bool X2D::Polygonf::Set(X2D::Pointf _begin, X2D::Pointf _end)
{
    this->Clear();

    this->m_vList.emplace_back(Pointf(_begin.x, _begin.y));
    this->m_vList.emplace_back(Pointf(_begin.x, _end.y));
    this->m_vList.emplace_back(Pointf(_end.x, _end.y));
    this->m_vList.emplace_back(Pointf(_end.x, _begin.y));

    this->RemoveDuplicatedPoint();
    this->m_bIsValid = this->isValid(this->m_vList);

    if (this->m_bIsValid)
    {
        this->m_bIsClockWise = this->isClockWise();
        this->calculateArea(this->m_vList, this->m_bxArea);
        this->calculateRect();

    }
    else
    {
        this->Clear();
    }
    return this->m_bIsValid;
}

bool X2D::Polygonf::Set()
{
    this->RemoveDuplicatedPoint();
    this->m_bIsValid = this->isValid(this->m_vList);

    if (this->m_bIsValid)
    {
        this->m_bIsClockWise = this->isClockWise();
        this->calculateArea(this->m_vList, this->m_bxArea);
        this->calculateRect();
    }
    else
    {
        this->Clear();
    }
    return this->m_bIsValid;
}

bool X2D::Polygonf::IsIn(X2D::Rectf t)
{
    for (auto &p : this->m_vList)
    {
        if (!t.IsInclude(p.x, p.y))
            return false;
    }
    return true;
}

void X2D::Polygonf::RemoveDuplicatedPoint()
{

}

bool X2D::Polygonf::IsInclude(X2D::Pointf pt)
{
    X2D::Pointf farAway{ };
    int         nTouchCnt;
    int         nIntersectCnt;

    Pointf tp1{ };
    Pointf tp2{ };

    bool result = this->m_bxArea.IsInclude(pt.x, pt.y);
    if (result)
    {
        nTouchCnt     = 0;
        nIntersectCnt = 0;

        //v5 = this.m_vList;
        farAway.x = m_bxArea.end.x + 1.0f;
        farAway.y = pt.y;

        if (m_vList.empty())
        {
            auto     size = static_cast<int>(m_vList.size());
            for (int i    = 0; i < size; ++i)
            {
                tp1 = m_vList[i];
                if (tp1.x == pt.x && tp1.y == pt.y)
                    return true;

                int i2 = i != size - 1 ? i + 1 : 0;
                tp2 = m_vList[i2];
                Linef::IntersectResult ir = X2D::Linef::IntersectCCW(tp1, tp2, farAway, pt);
                if (ir == Linef::IntersectResult::Intersect)
                    ++nIntersectCnt;
                if (ir == Linef::IntersectResult::Touch)
                {
                    if (std::min(tp1.y, tp2.y) == pt.y)
                    {
                        if (tp2.y != tp1.y)
                            ++nTouchCnt;
                    }
                }
            }
        }
        result    = ((nTouchCnt + nIntersectCnt) & 1) == 1;
    }
    return result;
}

X2D::Linef X2D::Polygonf::GetSegment(uint idx)
{
    uint   idx2 = idx != this->m_vList.size() - 1 ? idx + 1 : 0;
    Pointf p1   = this->m_vList[(int)idx];
    Pointf p2   = this->m_vList[(int)idx2];
    return Linef(p1, p2);
}

void X2D::Polygonf::Clear()
{
    m_vList.clear();
    m_bxArea.Set(Pointf(), Pointf());
    m_bIsClockWise = false;
    m_bIsValid     = false;
}

bool X2D::Polygonf::IsCollision(X2D::Rectf rc)
{
    Linef line_a{ };
    Linef line_b{ };
    Linef line_c{ };
    Linef line_d{ };
    Linef line{ };

    line.end.x = rc.pos.x;
    line.end.y = rc.pos.y;

    if (IsInclude(line.end))
    {
        return true;
    }
    else
    {
        for (auto &p1 :  m_vList)
        {
            if (rc.IsInclude(p1))
                return true;
        }
        float     rsx = rc.size.x + rc.pos.x;
        float     rsy = rc.size.y + rc.pos.y;

        line_a.begin.x = rc.pos.x;
        line_a.begin.y = rc.pos.y;
        line_a.end.x   = rsx;
        line_a.end.y   = rc.pos.y;
        line_b.begin.x = rsx;
        line_b.begin.y = rc.pos.y;
        line_b.end.x   = rsx;
        line_b.end.y   = rsy;
        line_c.begin.y = rsy;
        line_c.begin.x = rsx;
        line_c.end.x   = rc.pos.x;
        line_c.end.y   = rsy;
        line_d.begin.x = rc.pos.x;
        line_d.begin.y = rsy;
        line_d.end.x   = rc.pos.x;
        line_d.end.y   = rc.pos.y;

        for (int i = 0; i < static_cast<int>(m_vList.size()); ++i)
        {
            line = GetSegment((uint)i);
            if (X2D::Linef::IntersectCCW(line.begin, line.end, line_a.begin, line_a.end) == X2D::Linef::IntersectResult::Intersect)
                return true;
            X2D::Linef::IntersectResult r1 = X2D::Linef::IntersectCCW(line.begin, line.end, line_b.begin, line_b.end);
            if (r1 == X2D::Linef::IntersectResult::Intersect)
                return true;
            X2D::Linef::IntersectResult r2 = X2D::Linef::IntersectCCW(line.begin, line.end, line_c.begin, line_c.end);
            if (r2 == X2D::Linef::IntersectResult::Intersect)
                return true;
            if (X2D::Linef::IntersectCCW(line.begin, line.end, line_d.begin, line_d.end) == X2D::Linef::IntersectResult::Intersect
                || (line.end.x != line.begin.x
                    && line.end.y != line.begin.y
                    && r1 == X2D::Linef::IntersectResult::Touch
                    && r2 == X2D::Linef::IntersectResult::Touch))
                return true;
        }
    }
    return false;
}

bool X2D::Polygonf::isValid(std::vector<X2D::Pointf> vList)
{
    return true;
}

bool X2D::Polygonf::isClockWise()
{
    return false;
}

void X2D::Polygonf::calculateRect()
{
    this->pos.x  = this->m_bxArea.begin.x;
    this->pos.y  = this->m_bxArea.begin.y;
    this->size.x = this->m_bxArea.end.x - this->m_bxArea.begin.x;
    this->size.y = this->m_bxArea.end.y - this->m_bxArea.begin.y;
}

void X2D::Polygonf::calculateArea(std::vector<X2D::Pointf> vList, X2D::Boxf area)
{
    area.Set(*vList.begin(), *vList.end());

    for (auto &pt : vList)
    {
        if (area.begin.x > pt.x)
        {
            area.SetLeft(pt.x);
        }
        if (area.begin.y > pt.y)
        {
            area.SetTop(pt.y);
        }
        if (area.end.x < pt.x)
        {
            area.SetRight(pt.x);
        }
        if (area.end.y < pt.y)
        {
            area.SetBottom(pt.y);
        }
    }
}

X2D::PolygonF::PolygonF(X2D::Pointf p1, X2D::Pointf p2)
{
    this->m_Area = RectangleF(p1, p2);
    this->m_Points.emplace_back(Pointf(this->m_Area.m_TopLeft.x, this->m_Area.m_TopLeft.y));
    this->m_Points.emplace_back(Pointf(this->m_Area.m_BottomRight.x, this->m_Area.m_TopLeft.y));
    this->m_Points.emplace_back(Pointf(this->m_Area.m_BottomRight.x, this->m_Area.m_BottomRight.y));
    this->m_Points.emplace_back(Pointf(this->m_Area.m_TopLeft.x, this->m_Area.m_BottomRight.y));
}

X2D::PolygonF::PolygonF(std::vector<X2D::Pointf> points)
{
    this->m_Area = RectangleF(points);
    for (auto &p : points)
    {
        this->m_Points.emplace_back(Pointf(p.x, p.y));
    }
}

bool X2D::PolygonF::IsCollision(X2D::RectangleF rc)
{
    Linef line_a{ };
    Linef line_b{ };
    Linef line_c{ };
    Linef line_d{ };
    Linef line{ };

    line.end.x = rc.m_TopLeft.x;
    line.end.y = rc.m_TopLeft.y;

    if (this->IsInclude(line.end))
    {
        return true;
    }
    else
    {
        for (auto &p1 : this->m_Points)
        {
            if (rc.IsInclude(p1))
                return true;
        }
        float     rsx = rc.m_BottomRight.x;
        float     rsy = rc.m_BottomRight.y;

        line_a.begin.x = rc.m_TopLeft.x;
        line_a.begin.y = rc.m_TopLeft.y;
        line_a.end.x   = rsx;
        line_a.end.y   = rc.m_TopLeft.y;
        line_b.begin.x = rsx;
        line_b.begin.y = rc.m_TopLeft.y;
        line_b.end.x   = rsx;
        line_b.end.y   = rsy;
        line_c.begin.y = rsy;
        line_c.begin.x = rsx;
        line_c.end.x   = rc.m_TopLeft.x;
        line_c.end.y   = rsy;
        line_d.begin.x = rc.m_TopLeft.x;
        line_d.begin.y = rsy;
        line_d.end.x   = rc.m_TopLeft.x;
        line_d.end.y   = rc.m_TopLeft.y;

        for (int i = 0; i < static_cast<int>(m_Points.size()); ++i)
        {
            line = GetSegment((uint)i);
            if (X2D::Linef::IntersectCCW(line.begin, line.end, line_a.begin, line_a.end) == X2D::Linef::IntersectResult::Intersect)
                return true;
            X2D::Linef::IntersectResult r1 = X2D::Linef::IntersectCCW(line.begin, line.end, line_b.begin, line_b.end);
            if (r1 == X2D::Linef::IntersectResult::Intersect)
                return true;
            X2D::Linef::IntersectResult r2 = X2D::Linef::IntersectCCW(line.begin, line.end, line_c.begin, line_c.end);
            if (r2 == X2D::Linef::IntersectResult::Intersect)
                return true;
            if (X2D::Linef::IntersectCCW(line.begin, line.end, line_d.begin, line_d.end) == X2D::Linef::IntersectResult::Intersect
                || (line.end.x != line.begin.x
                    && line.end.y != line.begin.y
                    && r1 == X2D::Linef::IntersectResult::Touch
                    && r2 == X2D::Linef::IntersectResult::Touch))
                return true;
        }
    }
    return false;
}

bool X2D::PolygonF::IsLooseCollision(X2D::Linef line)
{
    if (!IsLooseInclude(line.begin) && !IsLooseInclude(line.end))
    {
        for (uint i = 0; i < static_cast<uint>(m_Points.size()); ++i)
        {
            Linef                  l   = GetSegment(i);
            Linef::IntersectResult res = Linef::IntersectCCW(l.begin, l.end, line.begin, line.end);
            if (res != Linef::IntersectResult::Seperate && res != Linef::IntersectResult::Touch)
                return true;
        }
    }
    return false;
}

bool X2D::PolygonF::IsLooseInclude(X2D::Pointf pt)
{
    uint   nIntersectCnt = 0;
    Pointf farAway{ };
    uint   nTouchCnt     = 0;

    bool result = this->m_Area.IsInclude(pt.x, pt.y);
    if (!result)
        return false;

    farAway = Pointf(this->m_Area.m_BottomRight.x + 1, pt.y);

    Pointf np{ };

    if (!m_Points.empty())
    {
        auto     size = static_cast<int>(m_Points.size());
        for (int i    = 0; i < size; ++i)
        {
            Pointf p = this->m_Points[i];

            if (p.x == pt.x && p.y == pt.y)
                return false;

            np = m_Points[i >= size - 1 ? 0 : i + 1];

            Linef::IntersectResult res = Linef::IntersectCCW(p, np, farAway, pt);
            if (res == Linef::IntersectResult::Intersect)
                nIntersectCnt++;
            if (res == Linef::IntersectResult::Touch)
            {
                float y = np.y;
                if (np.y >= p.y)
                    y = p.y;
                if (pt.y == y)
                {
                    if (p.y != np.y)
                        nTouchCnt++;
                }
            }
        }
    }
    result  = ((nIntersectCnt + nTouchCnt) & 1) == 1;
    return result;
}

bool X2D::PolygonF::IsInclude(X2D::Pointf pt)
{
    X2D::Pointf farAway{ };
    int         nTouchCnt;
    int         nIntersectCnt;

    Pointf tp1;
    Pointf tp2;

    bool result = this->m_Area.IsInclude(pt.x, pt.y);
//            return result;
    if (result)
    {
        nTouchCnt     = 0;
        nIntersectCnt = 0;

        //v5 = this.m_vList;
        farAway.x = this->m_Area.m_BottomRight.x + 1.0f;
        farAway.y = pt.y;

        if (!m_Points.empty())
        {
            auto     size = static_cast<int>(m_Points.size());
            for (int i    = 0; i < size; ++i)
            {
                tp1 = m_Points[i];
                if (tp1.x == pt.x && tp1.y == pt.y)
                    return true;

                int i2 = i != size - 1 ? i + 1 : 0;
                tp2 = m_Points[i2];
                Linef::IntersectResult ir = X2D::Linef::IntersectCCW(tp1, tp2, farAway, pt);
                if (ir == Linef::IntersectResult::Intersect)
                    ++nIntersectCnt;
                if (ir == Linef::IntersectResult::Touch)
                {
                    if (std::min(tp1.y, tp2.y) == pt.y)
                    {
                        if (tp2.y != tp1.y)
                            ++nTouchCnt;
                    }
                }
            }
        }
        result    = ((nTouchCnt + nIntersectCnt) & 1) == 1;
    }
    return result;
}

bool X2D::PolygonF::IsIn(X2D::RectangleF t)
{
    for (auto &p : this->m_Points)
    {
        if (!t.IsInclude(p.x, p.y))
            return false;
    }
    return true;
}

X2D::Linef X2D::PolygonF::GetSegment(uint idx)
{
    uint   idx2 = idx != this->m_Points.size() - 1 ? idx + 1 : 0;
    Pointf p1   = this->m_Points[(int)idx];
    Pointf p2   = this->m_Points[(int)idx2];
    return Linef(p1, p2);
}
