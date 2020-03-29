#include "geogeometry.h"

#include <algorithm>

GeoPolygon::GeoPolygon(const GeoPolygon& rhs) {
    this->rings.reserve(rhs.rings.size());
    for (auto& ring : rhs.rings) {
        this->rings.push_back(new GeoLinearRing(*ring));
    }
}

GeoGeometry* GeoPolygon::copy() {
    return (new GeoPolygon(*this));
}

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
    return rings[idx + 1];
}

void GeoPolygon::getRawData(double** rawData) const
{
    if (!rawData)
        return;

    int allPointsCount = getNumPoints();
    *rawData = new double[allPointsCount * 2];

    int offset = 0;

    for (auto& ring : rings) {
        std::copy(ring->begin(), ring->end(), (GeoRawPoint*)(*rawData) + offset);
        offset += ring->getNumPoints();
    }
}

// reserve memory for interior rings
void GeoPolygon::reserveInteriorRingsCount(int size)
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
        // Should not be executed here
        // You should add exterior ring first at any time
        rings.push_back(nullptr);
        rings.push_back(ring);
    }
    else {
        rings.push_back(ring);
    }
}

void GeoPolygon::removeInteriorRing(int idx)
{
    delete rings[idx + 1];
    rings.erase(rings.begin() + idx + 1);
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
    for (auto& ring : rings)
        count += ring->getNumPoints();
    return count;
}

GeoExtent GeoPolygon::getExtent() const
{
    if (isEmpty())
        return GeoExtent();

    GeoExtent extent(rings[0]->getExtent());
    int count = rings.size();
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
    for (auto& ring : rings) {
        ring->swapXY();
    }
}

void GeoPolygon::offset(double xOffset, double yOffset) {
    for (auto& ring : rings) {
        ring->offset(xOffset, yOffset);
    }
}

void GeoPolygon::rotate(double centerX, double centerY, double sinAngle, double cosAngle) {
    for (auto& ring : rings) {
        ring->rotate(centerX, centerY, sinAngle, cosAngle);
    }
}
