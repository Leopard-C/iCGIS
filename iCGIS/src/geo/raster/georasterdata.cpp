#include "geo/raster/georasterdata.h"

GeoRasterData::GeoRasterData(const GeoRasterData& rhs) {
    int count = rhs.bands.size();
    bands.reserve(count);
    for (int i = 0; i < count; ++i) {
        bands.push_back(new GeoRasterBand(*(rhs.bands[i])));
    }
}

GeoRasterData::~GeoRasterData()
{
    for (auto& band : bands)
        delete band;
}

GeoExtent GeoRasterData::getExtent() const
{
    if (bands.empty())
        return GeoExtent();

    // All bands have the same extent
    return bands[0]->getExtent();
}

void GeoRasterData::addBand(GeoRasterBand* band)
{
    bands.emplace_back(band);
}

void GeoRasterData::Draw() const {
    for (auto& band : bands) {
        band->Draw();
    }
}
