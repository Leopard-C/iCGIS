#include "geogeometry.h"


GeometryType GeoMultiPoint::getGeometryType() const
{
	return kMultiPoint;
}

const char* GeoMultiPoint::getGeometryName() const
{
	return "MULTIPOINT";
}

