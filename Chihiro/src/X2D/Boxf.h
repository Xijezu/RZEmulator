#ifndef PROJECT_BOXF_H
#define PROJECT_BOXF_H

#include "Common.h"
#include "Pointf.h"

namespace X2D {
    class Boxf {
    public:
        Boxf() = default;
        ~Boxf() = default;

        bool IsInclude(float x, float y);
        void SetLeft(float x);
        void SetTop(float y);
        void SetRight(float x);
        void SetBottom(float y);
        virtual void Set(Pointf _begin, Pointf _end);

        Pointf begin{ }, end{ };
    protected:
        void normalize();
    };
}


#endif // PROJECT_BOXF_H
