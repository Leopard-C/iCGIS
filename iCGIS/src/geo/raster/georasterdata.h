/*************************************************************************
** class name:  GeoRasterData
**
** description: 栅格数据（基类），暂时只支持32bit(float)灰度图像
**
** last change: 2020-01-04
*************************************************************************/
#pragma once

#include "geo/raster/georasterband.h"


class GeoRasterData {
public:
	GeoRasterData() {}
	virtual ~GeoRasterData();

	int getBandsCount() const { return bandsCount; }
	GeoRasterBand* getBand(int idx) const { return bands[idx]; }
	GeoExtent getExtent() const;

	void setBandsCount(int count);
	void setBand(int idx, GeoRasterBand* band);

private:
	GeoRasterBand** bands = nullptr;
	int bandsCount = 0;
};


