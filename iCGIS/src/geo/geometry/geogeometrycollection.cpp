#include "geogeometry.h"


GeoGeometryCollection::~GeoGeometryCollection()
{
	if (!isEmpty()) {
		for (auto& geom : geoms) {
			delete geom;
		}
	}
}

void GeoGeometryCollection::addGeometry(GeoGeometry* geom)
{
	geoms.push_back(geom);
}

void GeoGeometryCollection::removeGeometry(int idx)
{
	if (checkIndex(idx)) {
		delete geoms[idx];
		geoms.erase(geoms.begin() + idx);
	}
}

GeometryType GeoGeometryCollection::getGeometryType() const
{
	return kGeometryCollection;
}

const char* GeoGeometryCollection::getGeometryName() const
{
	return "GEOMETRYCOLLECTION";
}

GeoExtent GeoGeometryCollection::getExtent() const
{
	if (isEmpty())
		return GeoExtent();
	
	size_t count = geoms.size();
	GeoExtent extentOut(geoms[0]->getExtent());
	for (int i = 1; i < count; ++i) {
		extentOut.merge(geoms[i]->getExtent());
	}

	return extentOut;
}

int GeoGeometryCollection::getNumPoints() const
{
	int count = 0;
	for (const auto& geom : geoms)
		count += geom->getNumPoints();
	return count;
}

bool GeoGeometryCollection::isEmpty() const
{
	return geoms.empty();
}

void GeoGeometryCollection::swapXY()
{
	for (const auto& geom : geoms) {
		geom->swapXY();
	}
}

