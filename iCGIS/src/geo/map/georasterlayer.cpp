#include "geo/map/geolayer.h"


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


