#ifndef PROJECT_LINEF_H
#define PROJECT_LINEF_H

#include "Common.h"
#include "Pointf.h"

namespace X2D {
    class Linef {
    public:
        enum IntersectResult : int {
            None      = 157,
            Intersect = 1,
            Seperate  = -1,
            Touch     = 0,
        };

        Linef() = default;
        ~Linef() = default;
        Linef(Pointf p1, Pointf p2);

        static CcwResult CheckClockWisef(float x1, float y1, float x2, float y2, float x3, float y3);
        static CcwResult CheckClockWisef(Pointf pt1, Pointf pt2, Pointf pt3);
        static IntersectResult IntersectCCW(X2D::Pointf p1, X2D::Pointf p2, X2D::Pointf p3, X2D::Pointf p4);

        Pointf begin{ }, end{ };
    };
}


#endif // PROJECT_LINEF_H
