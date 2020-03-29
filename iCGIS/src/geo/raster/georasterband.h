/*******************************************************
** class name:  GeoRasterBand
**
** description: A band of raster data
**
** last change: 2020-01-04
*******************************************************/
#pragma once

#include "util/utility.h"
#include "util/memoryleakdetect.h"
#include "geo/geo_base.hpp"
#include "opengl/openglrasterdescriptor.h"


class GeoRasterBand {
public:
    GeoRasterBand() {}
    GeoRasterBand(int width, int height)
        : width(width), height(height) {}
    GeoRasterBand(const GeoRasterBand& rhs);
    ~GeoRasterBand();

    void setData(void* pDataIn, utils::DataType dataTypeIn);
    GeoExtent getExtent() const;

    GeoRawPoint getGeoXY(int pixelX, int pixelY) const;
    double getGeoX(int pixelX) const
        { return geoTransform[0] + pixelX * geoTransform[1]; }
    double getGeoY(int pixelY) const
        { return geoTransform[3] + pixelY * geoTransform[5]; }

    utils::DataType getDataType() const { return dataType; }

    void setOpenglRasterDescriptor(OpenglRasterDescriptor* desc);

    void Draw() const;

private:
    void destroyData();

public:
    void* pData = nullptr;
    double geoTransform[6];
    int width;
    int height;
    utils::DataType dataType;

private:
    OpenglRasterDescriptor* openglRasterDesc = nullptr;
};
