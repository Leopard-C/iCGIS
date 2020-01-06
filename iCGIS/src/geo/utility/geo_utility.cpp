#include "geo/utility/geo_utility.h"

#include "geo/geometry/geogeometry.h"

#include <algorithm>
#include <vector>
#include <array>
#include <iostream>

// 三角剖分库

std::vector<const char*> geoJsonObjectType = {
	"Point", "MultiPoint", "LineString", "MultiLineString", 
	"Polygon", "MultiPolygon", "GeometryCollection",
	"Feature", "FeatureCollection"
};

// geojson
bool isValidType(const char* type) {
	return geoJsonObjectType.end() != 
		std::find(geoJsonObjectType.begin(), geoJsonObjectType.end(), type);
}

const char* wkbTypeToString(int enumValue)
{
	switch (enumValue) {
	default:  return "Unknown";
	case 0:   return "Unknown";
	case 1:   return "Point";
	case 2:   return "LineString";
	case 3:   return "Polygon";
	case 4:   return "MultiPoint";
	case 5:   return "MultiLineString";
	case 6:   return "MultiPolygon";
	case 7:   return "GeometryCollection";
	case 8:   return "CircularString";
	case 9:   return "CompoundCurve";
	case 10:  return "CurvePolygon";
	case 11:  return "MultiCurve";
	case 12:  return "MultiSurface";
	case 13:  return "Curve";
	case 14:  return "Surface";
	case 15:  return "PolyhedralSurface";
	case 16:  return "TIN";
	case 17:  return "Triangle";
	case 100: return "None";
	case 101: return "LinearRing";
	// more
	//case 1008:
	//case 1009:
	// ...
	}
}

const char* GeometryTypeToName(GeometryType type)
{
	switch (type) {
	default:					return "";
	case kPoint:				return "POINT";
	case kPolygon:				return "POLYGON";
	case kLineString:			return "LINESTRING";
	case kLinearRing:			return "LINEARRING";
	case kMultiPoint:			return "MULTIPOINT";
	case kMultiPolygon:			return "MULTIPOLYGON";
	case kMultiLineString:		return "MULTILINESTRING";
	case kGeometryCollection:	return "GEOMETRYCOLLECTION";
	}
}

