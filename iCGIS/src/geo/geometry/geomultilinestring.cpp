#include "geogeometry.h"

GeoMultiLineString::GeoMultiLineString(const GeoMultiLineString& rhs)
    : GeoGeometryCollection(rhs)
{
}

GeoGeometry* GeoMultiLineString::copy() {
    return new GeoMultiLineString(*this);
}

GeometryType GeoMultiLineString::getGeometryType() const
{
    return kMultiLineString;
}

const char* GeoMultiLineString::getGeometryName() const
{
    return "MULTILINESTRING";
}
