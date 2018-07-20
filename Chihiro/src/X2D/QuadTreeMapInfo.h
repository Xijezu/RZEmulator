#pragma once
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
#include "Common.h"
#include "MapLocationInfo.h"

namespace X2D
{
    class QuadTreeMapInfo
    {
        public:

            class FunctorAdaptor
            {
                public:
                    FunctorAdaptor() = default;
                    ~FunctorAdaptor() = default;
                    std::vector<MapLocationInfo> pResult{ };
            };

            class Node
            {
                public:
                    Node() : init(true) {}

                    ~Node() = default;
                    Node(Pointf p1, Pointf p2, ushort depth);
                    bool Add(MapLocationInfo u);
                    void Enum(X2D::Pointf c, QuadTreeMapInfo::FunctorAdaptor &f);
                    bool Collision(X2D::Pointf c);
                    bool LooseCollision(Linef pLine);
                private:
                    Node getFitNode(MapLocationInfo u);
                    void add(MapLocationInfo u);
                    void divide();
                    bool init{false};
                public:
                    std::vector<MapLocationInfo> m_vList{ };
                    RectangleF                   m_Area{ };
                    std::map<int, Node>          m_pNode{ };
                    ushort                       m_unDepth{ };
            };

            QuadTreeMapInfo(float width, float height);
            void Enum(Pointf c, QuadTreeMapInfo::FunctorAdaptor &f);
            bool Add(MapLocationInfo u);
            bool Collision(X2D::Pointf c);
            Node       m_MasterNode{ };
            RectangleF m_Area{ };
    };
}