#include "geo/raster/georasterband.h"


GeoRasterBand::~GeoRasterBand()
{
	if (!pData)
		return;

	switch (dataType) {
	case utils::kByte:
		delete[] (char*)pData;
		break;
	case utils::kInt:
		delete[] (int*)pData;
		break;
	case utils::kUInt:
		delete[] (unsigned int*)pData;
		break;
	case utils::kFloat:
		delete[] (float*)pData;
		break;
	case utils::kDouble:
		delete[] (double*)pData;
		break;
	case utils::kUnknown:
	default:
		break;
	}
}

void GeoRasterBand::setData(void* pDataIn, utils::DataType dataTypeIn)
{
	pData = pDataIn;
	dataType = dataTypeIn;
}

GeoExtent GeoRasterBand::getExtent() const
{
	double minX = getGeoX(0);
	double maxX = getGeoX(width);
	double minY = getGeoY(height);
	double maxY = getGeoY(0);

	return { minX, maxX, minY, maxY };
}

// 像素点 -> 经纬度
GeoRawPoint GeoRasterBand::getGeoXY(int pixelX, int pixelY) const
{
	double geoX = geoTransform[0] + pixelX * geoTransform[1];
	double geoY = geoTransform[3] + pixelY * geoTransform[5];
	return { geoX, geoY };
}

