#ifndef PROJECT_QUADTREEMLI_H
#define PROJECT_QUADTREEMLI_H

#include "Common.h"
#include "MapLocationInfo.h"
#include "Boxf.h"
#include "RectangleF.h"

namespace X2D {
    class QuadTreeMli : public Boxf {
    public:
        QuadTreeMli() = default;
        ~QuadTreeMli() = default;

        class FunctorAdaptor {
        public:
            std::vector<MapLocationInfo> pResult{ };
        };

        class Node : public Rectf {
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
            std::map<int,Node>           m_pNode;
            ushort                       m_unDepth;
        };

        Node m_masterNode{};
    };
}
#endif // PROJECT_QUADTREEMLI_H
