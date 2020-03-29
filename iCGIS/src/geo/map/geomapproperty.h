/*******************************************************
** class name:  GeoMapProperty
**
** description: Properties of map
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/geo_base.hpp"

#include <QString>


class GeoMapProperty {
public:
    GeoMapProperty() = default;
    ~GeoMapProperty() = default;

public:
    QString name = "untitled";
    GeoExtent extent;
};
