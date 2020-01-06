/*******************************************************
** description: 有关空间数据处理的工具库
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include "geo/geometry/geogeometry.h"

#include <vector>

#include <gpc/gpc.h>
#include <mapbox/earcut.hpp>

const char* wkbTypeToString(int enumValue);

const char* GeometryTypeToName(GeometryType type);

