#include "geo/map/geolayer.h"

GeoRasterLayer::GeoRasterLayer(const GeoRasterLayer& rhs)
    : properties(rhs.properties)
{
    pData = new GeoRasterData(*(rhs.pData));
}

GeoLayer* GeoRasterLayer::copy() {
    return (new GeoRasterLayer(*this));
}

GeoRasterLayer::~GeoRasterLayer() {
    if (pData)
        delete pData;
}

void GeoRasterLayer::setData(GeoRasterData* pDataIn)
{
    if (pData)
        delete pData;

    pData = pDataIn;
    properties.extent = pDataIn->getExtent();
}

void GeoRasterLayer::Draw() const {
    if (pData)
        pData->Draw();
}
