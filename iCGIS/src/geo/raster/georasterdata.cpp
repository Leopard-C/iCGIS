#include "geo/raster/georasterdata.h"


GeoRasterData::~GeoRasterData()
{
	if (!bands)
		return;

	for (int i = 0; i < bandsCount; ++i) {
		delete bands[i];
	}
	delete[] bands;
}

GeoExtent GeoRasterData::getExtent() const
{
	if (bandsCount < 1)
		return GeoExtent();

	GeoExtent extent = bands[0]->getExtent();
	for (int i = 1; i < bandsCount; ++i) {
		extent.merge(bands[i]->getExtent());
	}
	return extent;
}

void GeoRasterData::setBandsCount(int count)
{
	bandsCount = count;
	if (bands) {
		// Should not be here
	}

	bands = new GeoRasterBand*[count];
}

void GeoRasterData::setBand(int idx, GeoRasterBand* band)
{
	bands[idx] = band;
}

