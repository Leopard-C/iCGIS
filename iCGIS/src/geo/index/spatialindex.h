/************************************************************************
** class name:  SpatialIndex
**
** last change: 2020-01-02
************************************************************************/
#pragma once

#include "util/memoryleakdetect.h"
#include "geo/geo_base.hpp"
#include "geo/map/geofeature.h"

#include <vector>

class SpatialIndex {
public:
    SpatialIndex() {}
    virtual ~SpatialIndex();

    // Point query
    // construct a square
    // x, y:        square's central point
    // halfEdge:    a half of rectangle's length of side
    virtual void queryFeature(double x, double y, double halfEdge, GeoFeature*& featureResult) = 0;

    // box query
    virtual void queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) = 0;
};
