#include "geo/raster/georasterband.h"
#include "util/env.h"


// Copy constructor (deep copy)
GeoRasterBand::GeoRasterBand(const GeoRasterBand& rhs)
{
    // Transform parameters
    for (int i = 0; i < 6; ++i)
        geoTransform[i] = rhs.geoTransform[i];

    // copy data
    int pixCount = width * height;
    switch (dataType) {
    case utils::kByte:
        pData = new char[pixCount];
        memcpy(pData, rhs.pData, pixCount);
        break;
    case utils::kInt:
        pData = new int[pixCount];
        memcpy(pData, rhs.pData, pixCount * sizeof(int));
        break;
    case utils::kUInt:
        pData = new unsigned int[pixCount];
        memcpy(pData, rhs.pData, pixCount * sizeof(unsigned int));
        break;
    case utils::kFloat:
        pData = new float[pixCount];
        memcpy(pData, rhs.pData, pixCount * sizeof(float));
        break;
    case utils::kDouble:
        pData = new double[pixCount];
        memcpy(pData, rhs.pData, pixCount * sizeof(double));
        break;
    case utils::kUnknown:
        break;
    }
}

GeoRasterBand::~GeoRasterBand()
{
    destroyData();
}

void GeoRasterBand::destroyData() {
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
    pData = nullptr;
}

void GeoRasterBand::setData(void* pDataIn, utils::DataType dataTypeIn)
{
    destroyData();
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

// pixel xy -> geo xy
GeoRawPoint GeoRasterBand::getGeoXY(int pixelX, int pixelY) const
{
    double geoX = geoTransform[0] + pixelX * geoTransform[1];
    double geoY = geoTransform[3] + pixelY * geoTransform[5];
    return { geoX, geoY };
}

void GeoRasterBand::setOpenglRasterDescriptor(OpenglRasterDescriptor* desc) {
    if (openglRasterDesc)
        delete openglRasterDesc;
    openglRasterDesc = desc;
}

void GeoRasterBand::Draw() const {
    Env::renderer.DrawTexture(openglRasterDesc->vao, openglRasterDesc->ibo,
                              openglRasterDesc->texs, Env::textureShader);
}
