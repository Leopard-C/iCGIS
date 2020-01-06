/*******************************************************
** class name:  GeoRasterBand
**
** description: 栅格的一个波段
**
** last change: 2020-01-04
*******************************************************/
#pragma once

#include "utility.h"
#include "memoryleakdetect.h"
#include "geo/geo_base.hpp"


class GeoRasterBand {
public:
	GeoRasterBand() {}
	GeoRasterBand(int width, int height)
		: width(width), height(height) {}
	~GeoRasterBand();

	void setData(void* pDataIn, utils::DataType dataTypeIn);
	GeoExtent getExtent() const;

	GeoRawPoint getGeoXY(int pixelX, int pixelY) const;
	double getGeoX(int pixelX) const
		{ return geoTransform[0] + pixelX * geoTransform[1]; }
	double getGeoY(int pixelY) const
		{ return geoTransform[3] + pixelY * geoTransform[5]; }

	utils::DataType getDataType() const { return dataType; }

public:
	void* pData = nullptr;
	double geoTransform[6];
	int width;
	int height;
	utils::DataType dataType;
};
