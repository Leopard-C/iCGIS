#include "geogeometry.h"

GeoMultiPoint::GeoMultiPoint(const GeoMultiPoint& rhs)
    : GeoGeometryCollection(rhs)
{
}

GeoGeometry* GeoMultiPoint::copy() {
    return new GeoMultiPoint(*this);
}

GeometryType GeoMultiPoint::getGeometryType() const
{
    return kMultiPoint;
}

const char* GeoMultiPoint::getGeometryName() const
{
    return "MULTIPOINT";
}

