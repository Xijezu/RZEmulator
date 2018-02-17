#ifndef PROJECT_REGIONTESTER_H
#define PROJECT_REGIONTESTER_H

#include "Object.h"

class RegionTester
{
    public:
        virtual void Init(Position OriginalPos, Position TargetPos, float RegionProperty) = 0;
        virtual bool IsInRegion(Position pos) = 0;
};

class DirectionRegionTester : public RegionTester
{
    public:
        DirectionRegionTester() : V1x(), V1y(), ori_x(), ori_y(), dx(), dy(), c(), thickness(), denominator() { };
        ~DirectionRegionTester() = default;
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
    private:
        float V1x;
        float V1y;
        float ori_x;
        float ori_y;
        float dx;
        float dy;
        float c;
        float thickness;
        float denominator;
};

class CrossRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
    private:
};

class ArcCircleRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
};

class CircleRegionTester : public RegionTester
{
    public:
        void Init(Position OriginalPos, Position TargetPos, float RegionProperty) override;
        bool IsInRegion(Position pos) override;
};



#endif // PROJECT_REGIONTESTER_H
