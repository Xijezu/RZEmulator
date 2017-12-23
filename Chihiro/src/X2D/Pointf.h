#ifndef PROJECT_POINTF_H
#define PROJECT_POINTF_H

#include "Common.h"

namespace X2D {
    enum CcwResult : int {
        Parallelism      = 0,
        ClockWise        = 1,
        CounterClockWise = -1,
    };

    struct PointBasef {
        float x;
        float y;
    };

    class Pointf {
    public:
        Pointf() = default;
        ~Pointf() = default;

        Pointf(float _x, float _y);

        float GetX()
        { return x; }

        float GetY()
        { return y; }

        void Set(float _x, float _y);
        float GetAlternativeDistance(Pointf rh);
        float GetDistance(Pointf rh);

        float x;
        float y;
    };
}


#endif // PROJECT_POINTF_H
