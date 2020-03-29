#include "geo/map/geofeaturelayerproperty.h"


void GeoFeatureLayerProperty::setGeometryType(GeometryType geometryTypeIn)
{
    if (geometryTypeIn == kMultiLineString)
        geometryType = kLineString;
    else if (geometryTypeIn == kMultiPolygon)
        geometryType = kPolygon;
    else if (geometryTypeIn == kMultiPoint)
        geometryType = kPoint;
    else
        geometryType = geometryTypeIn;
}
