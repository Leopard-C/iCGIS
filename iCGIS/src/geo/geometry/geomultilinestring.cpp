#include "geogeometry.h"


GeometryType GeoMultiLineString::getGeometryType() const
{
	return kMultiLineString;
}

const char* GeoMultiLineString::getGeometryName() const
{
	return "MULTILINESTRING";
}

