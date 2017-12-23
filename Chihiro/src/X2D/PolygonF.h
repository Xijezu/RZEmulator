#ifndef PROJECT_POLYGONF_H
#define PROJECT_POLYGONF_H

#include "Common.h"
#include "RectangleF.h"
#include "Linef.h"
#include "Boxf.h"

namespace X2D {
    class Polygonf : public Rectf {
    public:
        Polygonf() = default;
        bool Set(Pointf _begin, Pointf _end) override;
        bool Set();
        bool IsIn(X2D::Rectf t);
        void RemoveDuplicatedPoint();
        bool IsInclude(X2D::Pointf pt) override;
        X2D::Linef GetSegment(uint idx);
        void Clear();
        bool IsCollision(X2D::Rectf rc);
    protected:
        bool isValid(std::vector<Pointf> vList);
        bool isClockWise();
        void calculateRect();
        void calculateArea(std::vector<Pointf> vList, Boxf area);
    public:
        bool                m_bIsValid{ };
        bool                m_bIsClockWise{ };
        std::vector<Pointf> m_vList{ };
        Boxf                m_bxArea{ };
    };

    class PolygonF {
    public:
        PolygonF() = default;
        ~PolygonF() = default;

        PolygonF(Pointf p1, Pointf p2);
        PolygonF(std::vector<Pointf> points);
        bool IsCollision(RectangleF rc);
        bool IsLooseCollision(Linef line);
        bool IsLooseInclude(Pointf pt);
        bool IsInclude(X2D::Pointf pt);
        bool IsIn(X2D::RectangleF t);
        Linef GetSegment(uint idx);

        std::vector<Pointf> m_Points{};
        RectangleF m_Area{};
    };
}

#endif // PROJECT_POLYGONF_H
