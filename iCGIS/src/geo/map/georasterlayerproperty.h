/*******************************************************
** class name:  GeoRasterLayerProperty
**
** description: 栅格图层的属性
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

	bool visable = true;		// 是否可见（显示）
	int id = 0;
	QString name;
	GeoExtent extent;
};
