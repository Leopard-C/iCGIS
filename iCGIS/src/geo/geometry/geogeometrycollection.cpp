#include "geogeometry.h"

GeoGeometryCollection::GeoGeometryCollection(const GeoGeometryCollection& rhs) {
    int count = rhs.geoms.size();
    this->geoms.reserve(count);
    for (auto& geom : rhs.geoms) {
        this->geoms.push_back(geom->copy());
    }
}

//
//GeoGeometry* GeoGeometryCollection::copy() {
//    return (new GeoGeometryCollection(*this));
//}

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
    delete geoms[idx];
    geoms.erase(geoms.begin() + idx);
}

//GeometryType GeoGeometryCollection::getGeometryType() const
//{
//    return kGeometryCollection;
//}
//
//const char* GeoGeometryCollection::getGeometryName() const
//{
//    return "GEOMETRYCOLLECTION";
//}

GeoExtent GeoGeometryCollection::getExtent() const
{
    if (isEmpty())
        return GeoExtent();

    int count = geoms.size();
    GeoExtent extentOut(geoms[0]->getExtent());
    for (int i = 1; i < count; ++i) {
        extentOut.merge(geoms[i]->getExtent());
    }

    return extentOut;
}

int GeoGeometryCollection::getNumPoints() const
{
    int count = 0;
    for (auto& geom : geoms)
        count += geom->getNumPoints();
    return count;
}

bool GeoGeometryCollection::isEmpty() const
{
    return geoms.empty();
}

void GeoGeometryCollection::swapXY()
{
    for (auto& geom : geoms) {
        geom->swapXY();
    }
}

void GeoGeometryCollection::offset(double xOffset, double yOffset) {
    for (auto& geom : geoms) {
        geom->offset(xOffset, yOffset);
    }
}

void GeoGeometryCollection::rotate(double centerX, double centerY, double sinAngle, double cosAngle) {
    for (auto& geom : geoms) {
        geom->rotate(centerX, centerY, sinAngle, cosAngle);
    }
}
