#pragma once
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
#include "Common.h"
#include "MapLocationInfo.h"
#include "Boxf.h"
#include "RectangleF.h"

namespace X2D
{
    class QuadTreeMli : public Boxf
    {
        public:
            QuadTreeMli() = default;
            ~QuadTreeMli() = default;

            class FunctorAdaptor
            {
                public:
                    std::vector<MapLocationInfo> pResult{ };
            };

            class Node : public Rectf
            {
                public:
                    Node() = default;
                    bool Add(MapLocationInfo u);
                    void Enum(X2D::Pointf c, X2D::QuadTreeMli::FunctorAdaptor f);
                private:
                    Node getFitNode(MapLocationInfo u);
                    void add(MapLocationInfo u);
                    void divide();
                public:
                    std::vector<MapLocationInfo> m_vList{ };
                    Rectf                        m_rcEffectiveArea{ };
                    std::map<int, Node>          m_pNode;
                    ushort                       m_unDepth;
            };

            Node m_masterNode{ };
    };
}