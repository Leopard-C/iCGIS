#include "geogeometry.h"


GeoMultiPolygon::GeoMultiPolygon(const GeoMultiPolygon& rhs)
    : GeoGeometryCollection(rhs)
{
}

GeoGeometry* GeoMultiPolygon::copy() {
    return new GeoMultiPolygon(*this);
}


GeometryType GeoMultiPolygon::getGeometryType() const
{
    return kMultiPolygon;
}

const char* GeoMultiPolygon::getGeometryName() const
{
    return "MULTIPOLYGON";
}

int GeoMultiPolygon::getNumLinearRings() const
{
    int polygonsCount = getNumGeometries();
    int linearRingsCount = 0;
    for (int i = 0; i < polygonsCount; ++i) {
        linearRingsCount += (getPolygon(i)->getInteriorRingsCount() + 1);
    }
    return linearRingsCount;
}

