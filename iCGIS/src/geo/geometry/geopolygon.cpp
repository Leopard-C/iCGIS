#include "geogeometry.h"

#include <algorithm>


GeoPolygon::~GeoPolygon()
{
	for (auto& ring : rings)
		delete ring;
}

GeoLinearRing* GeoPolygon::getExteriorRing() const
{
	if (!isEmpty())
		return rings[0];
	else
		return nullptr;
}

GeoLinearRing* GeoPolygon::getInteriorRing(int idx) const
{
	if (checkIndex(idx + 1))
		return rings[idx + 1];
	else
		return nullptr;
}

void GeoPolygon::getRawData(double** rawData) const
{
	if (!rawData)
		return;

	int allPointsCount = getNumPoints();
	*rawData = new double[allPointsCount * 2];

	int offset = 0;

	for (const auto& ring : rings) {
		std::copy(ring->begin(), ring->end(), (GeoRawPoint*)(*rawData) + offset);
		offset += ring->getNumPoints();
	}
}

// 设置内环数目
void GeoPolygon::reserveInteriorRingsCount(size_t size)
{
	rings.reserve(size + 1);
}

void GeoPolygon::setExteriorRing(GeoLinearRing* ring)
{
	if (isEmpty())
		rings.push_back(ring);
	else {
		delete rings[0];
		rings[0] = ring;
	}
}

void GeoPolygon::addInteriorRing(GeoLinearRing* ring)
{
	if (isEmpty()) {
		// 不应该执行到这里
		// 任何时候，都应该先添加外环，再添加内环
		rings.push_back(new GeoLinearRing());
		rings.push_back(ring);
	}
	else {
		rings.push_back(ring);
	}
}

void GeoPolygon::removeInteriorRing(int idx)
{
	if (checkIndex(idx)) {
		delete rings[idx + 1];
		rings.erase(rings.begin() + idx + 1);
	}
}

bool GeoPolygon::checkIndex(int idx) const
{
	return idx > -1 && idx < rings.size();
}


/* Override */

GeometryType GeoPolygon::getGeometryType() const
{
	return kPolygon;
}

const char* GeoPolygon::getGeometryName() const
{
	return "POLYGON";
}

int GeoPolygon::getNumPoints() const
{
	int count = 0;
	for (const auto& ring : rings)
		count += ring->getNumPoints();
	return count;
}

GeoExtent GeoPolygon::getExtent() const
{
	if (isEmpty())
		return GeoExtent();
	
	GeoExtent extent(rings[0]->getExtent());
	int count = getInteriorRingsCount() + 1;
	for (int i = 1; i < count; ++i) {
		extent.merge(rings[i]->getExtent());
	}

	return extent;
}

bool GeoPolygon::isEmpty() const
{
	return rings.empty();
}

void GeoPolygon::swapXY()
{
	for (const auto& ring : rings) {
		ring->swapXY();
	}
}
