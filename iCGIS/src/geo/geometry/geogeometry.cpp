#include "geo/geometry/geogeometry.h"

GeoGeometry::~GeoGeometry() {

}

void GeoGeometry::rotate(double centerX, double centerY, double angle) {
    rotate(centerX, centerY, sin(angle * PI / 180.0), cos(angle * PI / 180.0));
}

void GeoGeometry::rotate(double sinAngle, double cosAngle) {
    GeoExtent extent = getExtent();
    rotate(extent.centerX(), extent.centerY(), sinAngle, cosAngle);
}

void GeoGeometry::rotate(double angle) {
    GeoExtent extent = getExtent();
    rotate(extent.centerX(), extent.centerY(), angle);
}
