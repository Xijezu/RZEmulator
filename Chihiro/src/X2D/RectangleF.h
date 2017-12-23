#ifndef PROJECT_RECTANGLEF_H
#define PROJECT_RECTANGLEF_H

#include "Common.h"
#include "Pointf.h"
#include "Linef.h"

namespace X2D {
    class Rectf {
    public:
        Rectf() = default;
        ~Rectf() = default;
        virtual bool IsInclude(float x, float y);
        virtual bool IsInclude(Pointf p);
        virtual bool Set(Pointf begin, Pointf end);

        X2D::Pointf pos{ }, size{ };
    };

    class RectangleF {
    public:
        RectangleF() = default;
        ~RectangleF() = default;
        RectangleF(Pointf p1, Pointf p2);
        explicit RectangleF(std::vector<Pointf> points);
        virtual bool IsInclude(float x, float y);
        virtual bool IsInclude(Pointf p);
        bool IsCollision(Linef line);

        Pointf m_TopLeft;
        Pointf m_BottomRight;
    };
}

#endif // PROJECT_RECTANGLEF_H
