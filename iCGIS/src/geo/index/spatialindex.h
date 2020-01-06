/************************************************************************
** class name:  SpatialIndex
**
** description: 空间索引基类，子类要实现queryFeatures()这一纯虚函数
**
** last change: 2020-01-02
************************************************************************/
#pragma once

#include "memoryleakdetect.h"
#include "geo/geo_base.hpp"
#include "geo/map/geofeature.h"

#include <vector>

class SpatialIndex {
public:
	SpatialIndex() {}
	virtual ~SpatialIndex() {}

	/* 查询空间要素 */
	virtual void queryFeatures(double x, double y, GeoFeature*& featureResult) = 0;
	virtual void queryFeatures(const GeoExtent& extent, std::vector<GeoFeature*>& featuresResult) = 0;

private:
};
