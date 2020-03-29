/*******************************************************
** class name:  GeoRasterLayerProperty
**
** description: Properties of Raster layer
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QString>

#include "geo/geo_base.hpp"


class GeoRasterLayerProperty {
public:
    GeoRasterLayerProperty() {}
    ~GeoRasterLayerProperty() {}

    bool visible = true;
    int id = 0;
    QString name;
    GeoExtent extent;
};
