#ifndef PROJECT_MAPLOCATIONINFO_H
#define PROJECT_MAPLOCATIONINFO_H

#include "Common.h"
#include "PolygonF.h"

class MapLocationInfo : public X2D::PolygonF {
public:
    MapLocationInfo(X2D::Pointf p1, X2D::Pointf p2, int id, int _pri) : X2D::PolygonF(p1, p2)
    {
        location_id = id;
        priority    = _pri;
    }

    MapLocationInfo(std::vector<X2D::Pointf> points, int id, int _pri) : X2D::PolygonF(std::move(points))
    {
        location_id = id;
        priority    = _pri;
    }

    std::string ToString()
    {
        return std::to_string(location_id);
    }

    int location_id;
    int priority;
};

#endif // PROJECT_MAPLOCATIONINFO_H
