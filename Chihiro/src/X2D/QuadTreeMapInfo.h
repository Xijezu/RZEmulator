#ifndef PROJECT_QUADTREEMAPINFO_H
#define PROJECT_QUADTREEMAPINFO_H

#include "Common.h"
#include "MapLocationInfo.h"

namespace X2D {
    class QuadTreeMapInfo {
    public:

        class FunctorAdaptor {
        public:
            FunctorAdaptor() = default;
            ~FunctorAdaptor() = default;
            std::vector<MapLocationInfo> pResult{ };
        };

        class Node {
        public:
            Node() : init(true) { }
            ~Node() = default;
            Node(Pointf p1, Pointf p2, ushort depth);
            bool Add(MapLocationInfo u);
            void Enum(X2D::Pointf c, QuadTreeMapInfo::FunctorAdaptor& f);
            bool Collision(X2D::Pointf c);
            bool LooseCollision(Linef pLine);
        private:
            Node getFitNode(MapLocationInfo u);
            void add(MapLocationInfo u);
            void divide();
            bool init{false};
        public:
            std::vector<MapLocationInfo> m_vList{ };
            RectangleF            m_Area{ };
            std::map<int,Node>    m_pNode{};
            ushort                m_unDepth{ };
        };

        QuadTreeMapInfo(float width, float height);
        void Enum(Pointf c, QuadTreeMapInfo::FunctorAdaptor& f);
        bool Add(MapLocationInfo u);
        bool Collision(X2D::Pointf c);
        Node m_MasterNode{};
        RectangleF m_Area{};
    };
}

#endif // PROJECT_QUADTREEMAPINFO_H
